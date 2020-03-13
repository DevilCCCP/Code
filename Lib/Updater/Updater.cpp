#include <qsystemdetection.h>
#include <QUuid>
#include <QFileInfo>
#include <QProcess>
#include <QMutexLocker>
#include <QSharedMemory>

#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectSettings.h>
#include <Lib/Settings/FileSettings.h>
#include <Lib/Log/Log.h>
#include <Local/ModuleNames.h>

#include "Updater.h"
#include "UpChecker.h"
#include "UpSync.h"
#ifdef Q_OS_UNIX
#include "Linux/LinuxUtils.h"
#endif


const int kWorkPeriodMs = 500;
const int kUpdatePointListMs = 5000;
const int kValidateUpdateInfoMs = 5000;
const int kUpdateInfoInitMs = 30 * 1000;

bool Updater::DoInit()
{
  if (!mUpInfo->Create() || !mVersion.LoadFromThis()) {
    GetOverseer()->Restart();
  }
  Log.Info(QString("Run with version %1").arg(mVersion.ToString()));
  return true;
}

bool Updater::DoCircle()
{
  switch (mUpdateState) {
  case eUpWait:
    CheckUpdate();
    break;

  case eUpReady:
    if (CreateSync()) {
      return DoCircle();
    }
    break;

  case eUpSync:
    CheckUpdate();
    break;

  case eUpStarting:
    if (DoUpdate()) {
      return false;
    }
    break;
  }
  return true;
}

void Updater::DoRelease()
{
  mUpInfo.clear();
  mUpSync.clear();
  mCheckers.clear();
}

void Updater::CheckUpdate()
{
  qint64 now = QDateTime::currentMSecsSinceEpoch();
  if (now >= mNextValidateInfo) {
    mUpInfo->Validate();
    mNextValidateInfo = now + kValidateUpdateInfoMs;
  }
  if (now >= mNextReloadPoints) {
    if (mDb->Connect()) {
      if (!mDbConnected) {
        Log.Info(QString("Db connected"));
        mDbConnected = true;
      }
      DoWork();
    } else {
      if (mDbConnected) {
        Log.Warning(QString("Db disconnected"));
        mDbConnected = false;
      }
    }
  }
}

bool Updater::CreateSync()
{
  if (mUpInfo->StartUpdateNow()) {
    QMutexLocker lock(&mUpdateMutex);
    mUpdateState = eUpStarting;
    return false;
  }
  mUpSync.reset(new UpSync(this, mUpInfo));
  GetOverseer()->RegisterWorker(mUpSync);

  QMutexLocker lock(&mUpdateMutex);
  mUpdateState = eUpSync;
  return true;
}

void Updater::GetCurrentVersion(Version& version)
{
  QMutexLocker lock(&mUpdateMutex);
  version = mVersion;
}

void Updater::OnUpdateAvailable(const Version& newVersion, const QString& updatePath, const QString& installerPath)
{
  if (NotifyNewUpdate(newVersion, updatePath, installerPath)) {
    WakeUp();
    Log.Info(QString("Update new ready"));
  }
}

void Updater::OnUpdateSync()
{
  if (NotifyUpdateSync()) {
    WakeUp();
    Log.Info(QString("Update sync done"));
  }
}

void Updater::DoWork()
{
  if (!mPointTypeId) {
    auto typeItem = mObjectTypeTable->GetItemByName("upd");
    if (typeItem) {
      mPointTypeId = typeItem->Id;
    }
  }
  if (!mPointTypeId) {
    return;
  }

  QList<ObjectItemS> points;
  if (!mObjectTable->GetObjectsByType(mPointTypeId, points)) {
    return;
  }

  if (!mDefaultPointId) {
    if (!mObjectTable->ReloadConnections()) {
      return;
    }
    for (auto itr = points.begin(); itr != points.end(); itr++) {
      const ObjectItemS& item = *itr;
      if (mObjectTable->MasterConnection().contains(item->Id)) {
        mDefaultPointId = item->Id;
        Log.Info(QString("Default point set to %1").arg(mDefaultPointId));
        break;
      }
    }
  }

  QSet<int> oldIds = mCheckers.keys().toSet();
  for (auto itr = points.begin(); itr != points.end(); itr++) {
    const ObjectItemS& item = *itr;
    if (item->Id != mDefaultPointId) {
      auto itr = mCheckers.find(item->Id);
      if (itr != mCheckers.end()) {
        const UpCheckerS& checker = *itr;
        checker->UpdatePoint(item);
        oldIds.remove(item->Id);
      } else {
        AddChecker(item);
      }
    }
  }

  for (auto itr = oldIds.begin(); itr != oldIds.end(); itr++) {
    int id = *itr;
    RemoveChecker(id);
  }

  mNextReloadPoints = QDateTime::currentMSecsSinceEpoch() + kUpdatePointListMs;
}

void Updater::AddChecker(const ObjectItemS& item)
{
  Log.Info(QString("Add check point (id: %1)").arg(item->Id));
  UpCheckerS checker(new UpChecker(this, item));
  mCheckers[item->Id] = checker;
  GetOverseer()->RegisterWorker(checker);
}

void Updater::RemoveChecker(int id)
{
  Log.Info(QString("Remove check point (id: %1)").arg(id));
  auto itr = mCheckers.find(id);
  if (itr != mCheckers.end()) {
    const UpCheckerS& checker = *itr;
    checker->Stop();
    mCheckers.erase(itr);
  } else {
    Log.Warning(QString("Remove unexisted point (id: %1)").arg(id));
  }
}

bool Updater::NotifyNewUpdate(const Version& newVersion, const QString& updatePath, const QString& installerPath)
{
  QMutexLocker lock(&mUpdateMutex);
  if (mUpdateState < eUpStarting && mVersion < newVersion) {
    mVersion = newVersion;
    Log.Info(QString("Updated with version %1").arg(mVersion.ToString()));
    mUpdatePath = updatePath;
    mInstallerPath = installerPath;
    if (mUpdateState < eUpReady) {
      mUpdateState = eUpReady;
      return true;
    }
  }
  return false;
}

bool Updater::NotifyUpdateSync()
{
  QMutexLocker lock(&mUpdateMutex);
  if (mUpdateState < eUpStarting) {
    mUpdateState = eUpStarting;
    return true;
  }
  return false;
}

bool Updater::DoUpdate()
{
  QString sourceFile = mUpdatePath + "/" + mInstallerPath;
  QString uniq = QUuid::createUuid().toString();
  uniq = uniq.mid(1, uniq.size() - 2);
  QString destFile = QCoreApplication::applicationDirPath() + "/" + uniq + "_" + mInstallerPath;
  QFileInfo fi(destFile);
  if (!fi.absoluteDir().mkpath(fi.absoluteDir().path())) {
    Log.Warning(QString("mkpath fail '%1'").arg(fi.absoluteDir().path()));
    return false;
  }
  while (fi.exists()) {
    destFile = QCoreApplication::applicationDirPath() + "/" + QUuid::createUuid().toString() + "_" + mInstallerPath;
    fi = QFileInfo(destFile);
  }
  if (!QFile::copy(sourceFile, destFile)) {
    Log.Warning(QString("Copy fail (old: '%1', new: '%2')").arg(sourceFile, destFile));
    return false;
  }

#ifdef Q_OS_UNIX
  QFile instScript(QCoreApplication::applicationDirPath() + "/install.sh");
  if (!instScript.open(QFile::WriteOnly)) {
    if (!KillFileOwner(instScript.fileName()) || !instScript.open(QFile::WriteOnly)) {
      Log.Warning(QString("Open install script fail"));
      return false;
    }
    Log.Warning(QString("Open install with fuser -k"));
  }
  QByteArray scriptData = QByteArray("#!/bin/bash\n\n") + destFile.toUtf8() + " inst " + mUpdatePath.toUtf8();
  if (instScript.write(scriptData) < scriptData.size() || !instScript.flush()) {
    Log.Warning(QString("Write install script fail"));
    return false;
  }
  instScript.close();
  QString cmd = QString("sudo service %1_instd start").arg(MAKE_STRING(PROGRAM_ABBR));
  bool instStarted = QProcess::startDetached(cmd);
  if (instStarted) {
    Log.Info(QString("Installer started (cmd: '%1')").arg(cmd));
    WaitInstaller();
    GetOverseer()->Done();
    return true;
  }
  Log.Warning(QString("Installer start fail (cmd: '%1')").arg(destFile));
#else
  bool instStarted = QProcess::startDetached(destFile, QStringList() << "inst" << mUpdatePath);
  if (instStarted) {
    Log.Info(QString("Installer started (file: '%1', path: '%2')").arg(destFile).arg(mUpdatePath));
    WaitInstaller();
    GetOverseer()->Done();
    return true;
  }
  Log.Warning(QString("Installer start fail (file: '%1')").arg(destFile));
#endif
  return false;
}

void Updater::WaitInstaller()
{
  while (IsAlive()) {
    Rest(20);
  }
}


Updater::Updater(const DbS& _Db)
  : Imp(kWorkPeriodMs)
  , mDb(_Db), mObjectTable(new ObjectTable(*_Db)), mObjectTypeTable(new ObjectTypeTable(*_Db))
  , mObjectSettingsTable(new ObjectSettingsTable(*_Db))
  , mPointTypeId(0), mDefaultPointId(0)
  , mNextValidateInfo(0), mNextReloadPoints(0), mDbConnected(false)
  , mUpdateState(eUpWait)
  , mUpInfo(new UpInfo())
{
  Q_INIT_RESOURCE(Updater);
}
