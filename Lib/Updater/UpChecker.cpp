#include <QCoreApplication>

#include <Lib/Db/ObjectType.h>
#include <Lib/Settings/DbSettings.h>
#include <Lib/Log/Log.h>
#include <Lib/Updater/Package.h>
#include <Lib/Updater/PackLoaderFile.h>
#include <Lib/Updater/PackLoaderHttp.h>

#include "UpChecker.h"
#include "Updater.h"

#ifdef TRACE_MD5
#define LogMd5(X) Log.Trace(X)
#else
#define LogMd5(X)
#endif


const int kWorkPeriodMs = 100;
const int kUnreachRetryMs = 30000;

bool UpChecker::DoInit()
{
  return true;
}

bool UpChecker::DoCircle()
{
  ObjectItemS item = mPointItem;
  if (!mPointId || item->Revision != mRevision) {
    if (ReloadPointInfo(item)) {
      CheckUpdate();
    }
  } else if (mCheckTimer.elapsed() > (mUnreach? kUnreachRetryMs: mPeriodMs)) {
    CheckUpdate();
  }

  return true;
}

void UpChecker::Stop()
{
  if (mLoader) {
    mLoader->Abort();
  }

  Imp::Stop();
}

void UpChecker::UpdatePoint(const ObjectItemS& item)
{
  if (mPointItem->Revision != item->Revision) {
    mPointItem = item;
  }
}

bool UpChecker::ReloadPointInfo(const ObjectItemS& item)
{
  mUnreach = false;
  Log.Info(QString("Reload update point info (id: %1)").arg(item->Id));
  Db db;
  if (!db.OpenDefault()) {
    Log.Warning("Db init fail");
    return false;
  }
  while (!db.Connect()) {
    if (!SayWork()) {
      return false;
    }
  }
  DbSettings settings(db);
  if (!settings.Open(QString::number(item->Id))) {
    Log.Warning("Open settings fail");
  }

  QString _Uri = settings.GetMandatoryValue("UpUri").toString();
  QString _Login = settings.GetValue("UpLogin", "").toString();
  QString _Pass = settings.GetValue("UpPass", "").toString();
  mPeriodMs = settings.GetValue("UpPeriod", 5000).toInt();
  static const QString kInfoSuffix(".info");
  if (_Uri.endsWith(kInfoSuffix)) {
    _Uri.remove(_Uri.size() - kInfoSuffix.size(), kInfoSuffix.size());
  }
  if (!_Uri.endsWith('/')) {
    _Uri.append('/');
  }

  static const QString kFilePrefix("file://");
  static const QString kHttpPrefix("http");
  if (_Uri.startsWith(kFilePrefix)) {
    mLoader.reset(new PackLoaderFile());
    mLoader->setUri(_Uri.mid(kFilePrefix.size()));
    mLoader->setLogin(_Login);
    mLoader->setPass(_Pass);
  } else if (_Uri.startsWith(kHttpPrefix)) {
    mLoader.reset(new PackLoaderHttp());
    mLoader->setUri(_Uri);
    mLoader->setLogin(_Login);
    mLoader->setPass(_Pass);
  } else {
    Log.Error(QString("Unimplemented update point type (Uri: '%1')").arg(_Uri));
    mLoader.clear();
  }

  mPointId = item->Id;
  mRevision = item->Revision;
  return true;
}

bool UpChecker::CheckUpdate()
{
  if (!mLoader) {
    return false;
  }

  mUpdater->GetCurrentVersion(mCurrentVersion);
  QByteArray verText;
  if (mLoader->FindPack() && mLoader->LoadVer(verText)) {
    if (mUnreach) {
      Log.Info(QString("Update is reachable (point: '%1')").arg(mLoader->getUri()));
      mUnreach = false;
      mError = false;
    }
    Version upVersion;
    if (upVersion.LoadFromString(verText)) {
      Log.Trace(QString("%1").arg(verText.constData()));
      if (mUpVersion < upVersion) {
        mUpVersion = upVersion;
        Log.Info(QString("Found update version (point: '%1', ver: '%2')").arg(mLoader->getUri()).arg(mUpVersion.ToString()));
      }
    } else {
      if (!mError) {
        Log.Error(QString("Version not read from info file"));
        mError = true;
      }
    }
  } else if (!mUnreach) {
    Log.Warning(QString("Update is unreachable (point: '%1')").arg(mLoader->getUri()));
    mUnreach = true;
  }

  if (SayWork() && mCurrentVersion < mUpVersion) {
    LoadUpdate();
  }

  mCheckTimer.start();
  return false;
}

bool UpChecker::LoadUpdate()
{
  QString downloadDir = QCoreApplication::applicationDirPath() + QString("/Updates/%1_p%4")
      .arg(mUpVersion.ToFilename()).arg(mPointId);

  Package pack;
  pack.SetCtrl(this);
  if (pack.Deploy(mLoader, downloadDir)) {
    mUpdater->OnUpdateAvailable(mUpVersion, downloadDir, pack.getInstallerPath());
  }
  return true;
}


UpChecker::UpChecker(Updater* _Updater, const ObjectItemS& _PointItem)
  : Imp(kWorkPeriodMs)
  , mUpdater(_Updater), mPointItem(_PointItem)
  , mPointId(0), mRevision(0), mUnreach(false)
{
}
