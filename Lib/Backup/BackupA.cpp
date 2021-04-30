#include <QFile>
#include <QUuid>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QProcess>

#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Db/Db.h>
#include <Lib/Settings/DbSettings.h>
#include <Lib/Settings/FileSettings.h>
#include <Lib/Log/Log.h>
#include <Lib/Db/DbBackup.h>
#include <Lib/Common/Var.h>
#include <Local/ModuleNames.h>
#ifdef Q_OS_UNIX
#include <Lib/Updater/Linux/LinuxUtils.h>
#endif

#include "BackupA.h"


const int kWorkPeriodMs = 500;
const QString kDateFormat("hh:mm");
const QString kFilenameFormat("yyyy-MM-dd hh-mm-ss");
const QString kFilenamePattern("_\?\?\?\?-\?\?-\?\? \?\?-\?\?-\?\?.bak");
const int     kFilenamePatternInd = kFilenamePattern.size() - 1;
const int     kFilenamePatternSize = kFilenamePattern.size() - 5;

bool BackupA::LoadSettings(SettingsA* settings)
{
  mBackupPath   = settings->GetValue("Path", "").toString();
  mUseServer    = settings->GetValue("UseServer", false).toBool();
  mAutoRestore  = settings->GetValue("AutoRestore", false).toBool();
  mBackupPeriod = settings->GetValue("Period", 0).toLongLong();
  QString  time = settings->GetValue("Time", "").toString();
  mDaylyTime = QTime::fromString(time, kDateFormat);

  if (!mVariables->Open(QString::number(GetOverseer()->Id()))) {
    return false;
  }

  mBackupUuid = mVariables->GetValue("Uuid", "").toString();
  if (mBackupUuid.isEmpty()) {
    mBackupUuid = QUuid::createUuid().toString();
    mVariables->SetValue("Uuid", mBackupUuid);
    mVariables->Sync();
  }

  mLastBackup = mVariables->GetValue("LastBackup", "").toDateTime();
  if (!mLastBackup.isValid()) {
    mLastBackup = QDateTime::currentDateTime().addYears(-1);
    mVariables->SetValue("LastBackup", mLastBackup);
    mVariables->Sync();
  }
  CalcNextBackup();
  return true;
}

bool BackupA::DoCircle()
{
  if (!mBackupReady && !PrapareBackup()) {
    return true;
  }

  if (QDateTime::currentMSecsSinceEpoch() > mNextBackupMs) {
    MakeBackup();
  }

  return true;
}

int BackupA::GetVersion()
{
  return 0;
}

bool BackupA::AfterBackup()
{
  return true;
}

bool BackupA::AfterRestore()
{
  return true;
}

QStringList BackupA::DefaultStaticTables()
{
  return QStringList() << "object_type" << "object" << "object_connection"
                       << "object_settings_type" << "object_settings"
                       << "object_state_type" << "object_state_values" << "object_state"
                       << "variables";
}

QStringList BackupA::EventStaticTables()
{
  return QStringList() << "event_type" << "event";
}

QStringList BackupA::DefaultLiveTables()
{
  return QStringList() << "object_state_log";
}

QStringList BackupA::EventLiveTables()
{
  return QStringList() << "event_log_hours" << "event_stat_hours";
}

QStringList BackupA::ReportLiveTables()
{
  return QStringList() << "report" << "files" << "report_files" << "report_send";
}

void BackupA::CalcNextBackup()
{
  mNextBackupMs = QDateTime::currentMSecsSinceEpoch() + 7 * 24 * 60 * 60 * 1000;
  if (mBackupPeriod > 0) {
    mNextBackupPeriodic = mLastBackup.addMSecs(mBackupPeriod);
    mNextBackupMs       = qMin(mNextBackupMs, mNextBackupPeriodic.toMSecsSinceEpoch());
  }
  if (mDaylyTime.isValid()) {
    mNextBackupDayly    = QDateTime(mLastBackup.date().addDays(1), mDaylyTime);
    mNextBackupMs       = qMin(mNextBackupMs, mNextBackupDayly.toMSecsSinceEpoch());
  }
}

bool BackupA::PrapareBackup()
{
  if (mRestoreStarted) {
    return false;
  }

  if (mBackupPath.startsWith("./")) {
    mBackupPath = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(mBackupPath.mid(2));
  }
  mBackupDir.setPath(mBackupPath);
  if (!mBackupDir.exists() && !mBackupDir.mkpath(mBackupPath)) {
    return false;
  }
  if (!PrepareSubdir(mBackupDir, "Static", mBackupStatic) || !PrepareSubdir(mBackupDir, "Live", mBackupLive)
      || !PrepareSubdir(mBackupDir, "Var", mBackupVar)) {
    if (!mBackupPrepareWarning) {
      Log.Warning(QString("Create subdirectories fail"));
      mBackupPrepareWarning = true;
    }
    return false;
  }

  QString backupSettingsFilename = mBackupDir.absoluteFilePath("Info.ini");
  mBackupSettings.reset(new FileSettings());
  if (!mBackupSettings->Open(backupSettingsFilename)) {
    if (!mBackupPrepareWarning) {
      Log.Warning(QString("Create backup settings fail"));
      mBackupPrepareWarning = true;
    }
    return false;
  }
  bool    needRestore = mBackupSettings->GetValue("Restore", false).toBool();
  QString currentUuid = mBackupSettings->GetValue("Uuid", "").toString();
  if ((mAutoRestore && !currentUuid.isEmpty() && currentUuid != mBackupUuid) || needRestore) {
    StartRestore();
    return false;
  }

  int  currentVersion = mBackupSettings->GetValue("Version", 0).toInt();
  if (currentUuid.isEmpty() || currentVersion != GetVersion()) {
    UpdateBackupInfo(mBackupUuid);
  }

  mBackupReady = true;
  Log.Info(QString("Backup to '%1' ready").arg(mBackupPath));
  return true;
}

bool BackupA::PrapareRestore()
{
  if (mBackupPath.startsWith("./")) {
    mBackupPath = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(mBackupPath.mid(2));
  }
  mBackupDir.setPath(mBackupPath);
  if (!mBackupDir.exists()) {
    return false;
  }
  if (!PrepareSubdir(mBackupDir, "Static", mBackupStatic) || !PrepareSubdir(mBackupDir, "Live", mBackupLive)
      || !PrepareSubdir(mBackupDir, "Var", mBackupVar)) {
    if (!mBackupPrepareWarning) {
      Log.Warning(QString("Open backup subdirectories fail"));
      mBackupPrepareWarning = true;
    }
    return false;
  }

  QString backupSettingsFilename = mBackupDir.absoluteFilePath("Info.ini");
  mBackupSettings.reset(new FileSettings());
  if (!mBackupSettings->Open(backupSettingsFilename)) {
    Log.Warning(QString("Open backup settings fail"));
    return false;
  }
  mBackupUuid = mBackupSettings->GetValue("Uuid", "").toString();

  Log.Info(QString("Restore from '%1' ready").arg(mBackupPath));
  return true;
}

bool BackupA::PrepareSubdir(const QDir& root, const QString& subpath, QDir& dir)
{
  dir = root;
  if (dir.cd(subpath)) {
    return true;
  }
  dir.mkdir(subpath);
  if (dir.cd(subpath)) {
    return true;
  }
  return false;
}

bool BackupA::UpdateBackupInfo(const QString& currentUuid)
{
  mBackupSettings->SetValue("Restore", false);
  mBackupSettings->SetValue("Uuid", currentUuid);
  mBackupSettings->SetValue("Version", GetVersion());
  QStringList staticTables;
  GetStaticTables(GetVersion(), staticTables);
  mBackupSettings->SetValue("StaticTables", staticTables);
  QStringList liveTables;
  GetLiveTables(GetVersion(), liveTables);
  mBackupSettings->SetValue("LiveTables", liveTables);
  if (!mBackupSettings->Sync()) {
    if (!mBackupPrepareWarning) {
      Log.Warning(QString("Save backup settings fail"));
      mBackupPrepareWarning = true;
    }
    return false;
  }
  return true;
}

QString BackupA::GetStaticRecentFilename(const QString& table)
{
  QDirIterator dirItr(mBackupStatic.absolutePath(), QStringList() << table + kFilenamePattern, QDir::Files, QDirIterator::Subdirectories);
  QDateTime lastTime = QDateTime::fromMSecsSinceEpoch(0);
  QString filename;
  while (dirItr.hasNext()) {
    QString nextFilename = dirItr.next();
    QString nextTimeText = nextFilename.mid(nextFilename.size() - kFilenamePatternInd, kFilenamePatternSize);
    QDateTime nextTime = QDateTime::fromString(nextTimeText, kFilenameFormat);
    if (nextTime > lastTime) {
      filename = nextFilename;
    }
  }
  return filename;
}

QByteArray BackupA::GetStaticFileHash(const QString& table)
{
  auto itr = mHashMap.find(table);
  if (itr == mHashMap.end()) {
    QString filename = GetStaticRecentFilename(table);
    if (filename.isEmpty()) {
      return QByteArray();
    }

    QByteArray hash;
    QFile file(filename);
    if (file.open(QFile::ReadOnly)) {
      QCryptographicHash md5(QCryptographicHash::Md5);
      md5.addData(&file);
      hash = md5.result();
    }
    itr = mHashMap.insert(table, hash);
  }

  return itr.value();
}

void BackupA::UpdateStaticFileHash(const QString& table, const QByteArray& hash)
{
  mHashMap.insert(table, hash);
}

void BackupA::MakeBackup()
{
  if (MakeStaticBackup() && MakeLiveBackup() && MakeVarBackup() && AfterBackup()) {
    mLastBackup = QDateTime::currentDateTime();
    mVariables->SetValue("LastBackup", mLastBackup);
    mVariables->Sync();

    CalcNextBackup();
  } else {
    mBackupReady = false;
  }
}

bool BackupA::MakeStaticBackup()
{
  QStringList tablesList;
  GetStaticTables(GetVersion(), tablesList);
  foreach (const QString& table, tablesList) {
    if (!MakeStaticTable(table)) {
      return false;
    }
  }
  return true;
}

bool BackupA::MakeStaticTable(const QString& table)
{
  if (!SayWork()) {
    return false;
  }

  QByteArray data;
  QDataStream dataStream(&data, QIODevice::WriteOnly);
  mDbBackup->BackupOne(table, &dataStream);

  QByteArray hashOld = GetStaticFileHash(table);
  QCryptographicHash md5(QCryptographicHash::Md5);
  md5.addData(data);
  QByteArray hashNew = md5.result();

  if (hashNew != hashOld) {
    QDir year;
    PrepareSubdir(mBackupStatic, QDateTime::currentDateTime().toString("yyyy"), year);
    PrepareSubdir(year, QDateTime::currentDateTime().toString("MM"), mBackupStaticSub);

    QFile file(mBackupStaticSub.absoluteFilePath(QString("%1_%2.bak").arg(table).arg(QDateTime::currentDateTime().toString(kFilenameFormat))));
    if (!file.open(QFile::WriteOnly) || (file.write(data) != data.size())) {
      Log.Warning(QString("Write table to disk fail (file: '%1', error: '%2'").arg(file.fileName()).arg(file.errorString()));
      mBackupWriteWarning = true;
      return false;
    }

    Log.Info(QString("Backup table done (table: '%1', md5: '%2')").arg(table).arg(hashNew.toHex().constData()));
    mBackupWriteWarning = false;
    UpdateStaticFileHash(table, hashNew);
  } else {
    Log.Info(QString("Backup table not changed (table: '%1', md5: '%2')").arg(table).arg(hashNew.toHex().constData()));
  }
  return true;
}

bool BackupA::MakeLiveBackup()
{
  QDir year;
  PrepareSubdir(mBackupLive, QDateTime::currentDateTime().toString("yyyy"), year);
  PrepareSubdir(year, QDateTime::currentDateTime().toString("MM"), mBackupLiveSub);

  QStringList tablesList;
  GetLiveTables(GetVersion(), tablesList);
  foreach (const QString& table, tablesList) {
    if (!MakeLiveTable(table)) {
      return false;
    }
  }
  return true;
}

bool BackupA::MakeLiveTable(const QString& table)
{
  if (!SayWork()) {
    return false;
  }

  qint64 topId = mVariables->GetValue(table + "_id", "").toLongLong();

  QByteArray data;
  QDataStream dataStream(&data, QIODevice::WriteOnly);
  qint64 lastId = topId;
  mDbBackup->BackupOne(table, &dataStream, &lastId);

  if (lastId > topId) {
    QFile file(mBackupLiveSub.absoluteFilePath(QString("%1_%2.bak").arg(table).arg(QDateTime::currentDateTime().toString(kFilenameFormat))));
    if (file.exists()) {
      return false;
    }
    if (!file.open(QFile::WriteOnly) || (file.write(data) != data.size())) {
      if (!mBackupWriteWarning) {
        Log.Warning(QString("Write table to disk fail (file: '%1', error: '%2'").arg(file.fileName()).arg(file.errorString()));
        mBackupWriteWarning = true;
      }
      return false;
    }

    mBackupWriteWarning = false;
    mVariables->SetValue(table + "_id", lastId);
    if (!mVariables->Sync()) {
      return false;
    }

    Log.Info(QString("Backup table done (table: '%1', id: %2)").arg(table).arg(lastId));
  }
  return true;
}

bool BackupA::MakeVarBackup()
{
  if (!mUseServer) {
    return true;
  }

  QStringList daemonsNames;
  QStringList daemonsExes;
  GetDaemons(daemonsNames, daemonsExes);

  foreach (const QString& daemon, daemonsNames) {
    if (!QFile::exists(mBackupVar.absoluteFilePath(QString("%1.ini").arg(daemon))) && QFile::exists(GetVarFile(daemon))) {
      if (!QFile::copy(GetVarFile(daemon), mBackupVar.absoluteFilePath(QString("%1.ini").arg(daemon)))) {
        return false;
      }
    }
  }
  return true;
}

bool BackupA::StartRestore()
{
  QString restoreExe = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(kInstallExe);
#ifdef Q_OS_UNIX
  QFile instScript(QCoreApplication::applicationDirPath() + "/install.sh");
  if (!instScript.open(QFile::WriteOnly)) {
    if (!KillFileOwner(instScript.fileName()) || !instScript.open(QFile::WriteOnly)) {
      Log.Warning(QString("Open install script fail"));
      return false;
    }
    Log.Warning(QString("Open install with fuser -k"));
  }
  QByteArray scriptData = QByteArray("#!/bin/bash\n\n") + restoreExe.toUtf8() + QByteArray(" restore '") + mBackupPath.toUtf8() + "'";
  if (instScript.write(scriptData) < scriptData.size() || !instScript.flush()) {
    Log.Warning(QString("Write restore script fail"));
    return false;
  }
  instScript.close();
  QString cmd = "sudo service vica_instd start";
  bool instStarted = QProcess::startDetached(cmd);
#else
  bool instStarted = QProcess::startDetached(restoreExe, QStringList() << "restore" << mBackupPath);
#endif
  if (!instStarted) {
    Log.Warning(QString("Restore start fail"));
    return false;
  }

  mRestoreStarted = true;
  Log.Info(QString("Restore started"));
  return true;
}

bool BackupA::Restore(const QString& path)
{
  mBackupPath = path;
  if (!PrapareRestore()) {
    return false;
  }

  if (!RestoreStaticBackup() || !RestoreLiveBackup()) {
    return false;
  }

  bool ok = true;
  if (!RestoreVarBackup() || !AfterRestore()) {
    ok = false;
  }

  if (!ok) {
    Log.Error(QString("After restore fail"));
    return false;
  }

  mBackupSettings->SetValue("Restore", false);
  if (!mBackupSettings->Sync()) {
    Log.Error(QString("Restore done fail"));
    return false;
  }
  Log.Info(QString("Restore done"));
  GetOverseer()->Restart();
//  if (!UpdateBackupInfo(currentUuid)) {
//    return false;
//  }
//  mBackupUuid = currentUuid;
  return true;
}

bool BackupA::RestoreStaticBackup()
{
  QStringList tables;
  GetStaticTables(GetVersion(), tables);
  tables = mBackupSettings->GetValue("StaticTables", tables).toStringList();

  QStringList tableFilenames;
  foreach (const QString& table, tables) {
    QString filename = GetStaticRecentFilename(table);
    if (filename.isEmpty()) {
      if (!mRestoreWarning) {
        Log.Warning(QString("Table not found on disk (table: '%1'").arg(table));
        mRestoreWarning = true;
      }
      return false;
    }
    tableFilenames.append(filename);
  }

  for (int i = tables.size() - 1; i >= 0; i--) {
    const QString& table = tables.at(i);
    if (!ClearStaticTable(table)) {
      if (!mRestoreWarning) {
        Log.Error(QString("Clear table '%1' fail").arg(table));
        mRestoreWarning = true;
      }
      return false;
    }
  }

  for (int i = 0; i < tables.size(); i++) {
    const QString& table = tables.at(i);
    const QString& filename = tableFilenames.at(i);
    if (!RestoreStaticTable(table, filename)) {
      if (!mRestoreWarning) {
        Log.Error(QString("Restore table '%1' fail").arg(table));
        mRestoreWarning = true;
      }
      return false;
    }
    mRestoreWarning = false;
  }
  return true;
}

bool BackupA::ClearStaticTable(const QString& table)
{
  return mDbBackup->ClearOne(table);
}

bool BackupA::RestoreStaticTable(const QString& table, const QString& filename)
{
  if (!SayWork()) {
    return false;
  }

  QFile file(filename);
  if (!file.open(QFile::ReadOnly)) {
    if (!mRestoreWarning) {
      Log.Warning(QString("Read table from disk fail (file: '%1', error: '%2'").arg(file.fileName()).arg(file.errorString()));
      mRestoreWarning = true;
    }
    return false;
  }
  QDataStream dataStream(&file);
  return mDbBackup->RestoreOne(table, &dataStream, true);
}

bool BackupA::RestoreLiveBackup()
{
  QStringList tables;
  GetStaticTables(GetVersion(), tables);
  tables = mBackupSettings->GetValue("LiveTables", tables).toStringList();
  foreach (const QString& table, tables) {
    if (!RestoreLiveTable(table)) {
      if (!mRestoreWarning) {
        Log.Error(QString("Restore table '%1' fail").arg(table));
        mRestoreWarning = true;
      }
      return false;
    }
    mRestoreWarning = false;
  }
  return true;
}

bool BackupA::RestoreLiveTable(const QString& table)
{
  if (!SayWork()) {
    return false;
  }

  auto q = mDb.MakeQuery();
  q->prepare(QString("DELETE FROM %1").arg(table));
  if (!mDb.ExecuteNonQuery(q)) {
    Log.Error(QString("Clear table '%1' fail").arg(table));
    return false;
  }
  Log.Info(QString("Done clear table '%1'").arg(table));

  QDirIterator dirItr(mBackupLive.absolutePath(), QStringList() << table + kFilenamePattern, QDir::Files, QDirIterator::Subdirectories);
  while (dirItr.hasNext()) {
    QFile file(dirItr.next());
    if (!file.open(QFile::ReadOnly)) {
      Log.Warning(QString("Read table from disk fail (file: '%1', error: '%2'").arg(file.fileName()).arg(file.errorString()));
      mRestoreWarning = true;
      return false;
    }
    QDataStream dataStream(&file);
    if (!mDbBackup->RestoreOne(table, &dataStream, false)) {
      return false;
    }
  }
  return true;
}

bool BackupA::RestoreVarBackup()
{
  QStringList daemonsNames;
  QStringList daemonsExes;
  GetDaemons(daemonsNames, daemonsExes);

  foreach (const QString& daemon, daemonsNames) {
    QFile src(mBackupVar.absoluteFilePath(QString("%1.ini").arg(daemon)));
    if (!src.exists()) {
      continue;
    }
    if (!src.open(QFile::ReadOnly)) {
      Log.Error(QString("Read daemon settings fail (daemon: '%1')").arg(daemon));
      return false;
    }
    QFile dst(GetVarFile(daemon));
    if (dst.setPermissions(dst.permissions() | QFileDevice::WriteOwner) && dst.open(QFile::WriteOnly)) {
      QByteArray data = src.readAll();
      if (dst.write(data) != data.size()) {
        Log.Error(QString("Write daemon settings fail (daemon: '%1')").arg(daemon));
        return false;
      }
    }
    if (!dst.setPermissions(QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadUser | QFileDevice::ReadOther)) {
      Log.Warning(QString("Set daemon settings permissions fail (daemon: '%1')").arg(daemon));
    }
  }
  return true;
}

bool BackupA::StopDaemons()
{
  QStringList daemonsNames;
  QStringList daemonsExes;
  GetDaemons(daemonsNames, daemonsExes);

  foreach (const QString& daemonExe, daemonsExes) {
    QProcess::startDetached(QString("%1 stop").arg(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(daemonExe)));
  }
  return true;
}

bool BackupA::StartDaemons()
{
  QStringList daemonsNames;
  QStringList daemonsExes;
  GetDaemons(daemonsNames, daemonsExes);

  foreach (const QString& daemonExe, daemonsExes) {
    QProcess::startDetached(QString("%1 start").arg(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(daemonExe)));
  }
  return true;
}


BackupA::BackupA(const Db& _Db)
  : ImpD(_Db, kWorkPeriodMs)
  , mDb(_Db), mDbBackup(new DbBackup(mDb)), mVariables(new DbSettings(mDb, "variables", "_object"))
  , mNextBackupMs(0)
  , mBackupReady(false), mRestoreStarted(false), mBackupPrepareWarning(false), mBackupWriteWarning(false), mRestoreWarning(false)
{
}

BackupA::~BackupA()
{
}
