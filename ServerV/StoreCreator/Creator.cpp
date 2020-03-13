#include <Lib/Db/Db.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Settings/DbSettings.h>
#include <Lib/Log/Log.h>
#include <Lib/Include/License.h>
#include <Lib/Common/Format.h>
#include <LibV/Storage/Storage.h>

#include "Creator.h"


const int kRetryPeriodStepMs = 1000;
const int kMaxRetryPeriodMs = 2*60*1000;

bool Creator::DoInit()
{
  LICENSE_INIT(0x2492D4);
  mDb.reset(new Db());
  return mDb->OpenDefault();
}

bool Creator::DoCircle()
{
  if (mDb->Connect()) {
    if (mLastPeriodMs && mWorkTimer.elapsed() < mLastPeriodMs) {
      return true;
    }
    if (CreateStorageTable() && CreateStorageFile() && UpdateStorageStatus()) {
      Log.Info(QString("Create storage done"));
      GetOverseer()->Done();
      return false;
    } else {
      mLastPeriodMs = qMin(mLastPeriodMs + kRetryPeriodStepMs, kMaxRetryPeriodMs);
      Log.Info(QString("Create storage fail, next retry after %1").arg(FormatTimeDelta(mLastPeriodMs)));
      mWorkTimer.start();
    }
  }
  return true;
}

bool Creator::CreateStorageTable()
{
  if (mDbOk) {
    return true;
  }

  int id = GetOverseer()->Id();
  Log.Info(QString("Creating storage (id: %1)").arg(id));
  QString name = Storage::ConnectionFile(id);
  QString pass = QUuid::createUuid().toString() + "=" + QUuid::createUuid().toString();

  if (!mDb->SaveConnectionFile(name, QString(), QString(), QString(), name, pass)) {
    return false;
  }

  mDb->Exec(QString("DROP OWNED BY %1 CASCADE;").arg(name), false);
  QString createScript = QString("DROP SCHEMA IF EXISTS %1 CASCADE;"
                                 " DROP USER IF EXISTS %1;"
                                 " CREATE USER %1 WITH PASSWORD '%2';"
                                 " GRANT %1 TO su;"
                                 " GRANT usr TO %1;"
                                 " CREATE SCHEMA %1 AUTHORIZATION %1;"
                                 " GRANT USAGE ON SCHEMA %1 TO GROUP su;").arg(name, pass);

  if (!ExecStringScript(createScript)) {
    Log.Error(QString("Create storage scheme fail (name: %1)").arg(name));
    return false;
  }

  return mDbOk = ExecFileScript("storage_cell.sql", name)
      && ExecFileScript("storage_current_cell.sql", name)
      && ExecFileScript("storage_resize.sql", name)
      && ExecFileScript("get_current_cell.sql", name)
      && ExecFileScript("get_next_cell.sql", name);
}

bool Creator::CreateStorageFile()
{
  if (mFileOk) {
    return true;
  }

  Log.Info("Create storage file");
  DbSettingsS settings = DbSettingsS(new DbSettings(*mDb));
  if (settings->Open(QString::number(GetOverseer()->Id()))) {
    mStorage = StorageS(new Storage(*settings, Storage::ConnectionFile(GetOverseer()->Id())));
    return mFileOk = mStorage->Create();
  }
  Log.Error(QString("Open storage settings fail"));
  return false;
}

bool Creator::ExecStringScript(const QString& script)
{
  return mDb->Exec(script);
}

bool Creator::ExecFileScript(const QString& filename, const QString& username)
{
  QFile file(QCoreApplication::applicationDirPath() + "/Scripts/" + filename);
  if (!file.open(QIODevice::ReadOnly)) {
    Log.Error(QString("No script file '%1'").arg(filename));
    return false;
  }

  QTextStream in(&file);
  QString text = in.readAll().replace("%username%", username);
  file.close();

  if (!mDb->Exec(text)) {
    Log.Error(QString("Exec storage create script fail (filename: %1)").arg(filename));
    return false;
  } else {
    Log.Info(QString("Exec storage create script success (filename: %1)").arg(filename));
  }
  return true;
}

bool Creator::UpdateStorageStatus()
{
  auto q = mDb->MakeQuery();
  q->prepare(QString("UPDATE object SET status = 0, revision = revision + 1 WHERE _id = %1; "
                     " UPDATE object SET revision = revision + 1 FROM object_connection c"
                     " WHERE c._oslave = %1 AND c._omaster = object._id;").arg(GetOverseer()->Id()));
  return mDb->ExecuteNonQuery(q);
}


Creator::Creator()
  : Imp(100)
  , mLastPeriodMs(0), mDbOk(false), mFileOk(false)
{
}

Creator::~Creator()
{
}


