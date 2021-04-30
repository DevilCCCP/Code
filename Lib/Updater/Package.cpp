#include <Lib/Db/Db.h>
#include <Lib/Log/Log.h>

#include "Package.h"


bool Package::ExecSql(const QString& scriptPath)
{
  if (!ConnectDb()) {
    Log.Error(QString("Connect Db fail"));
    return false;
  }

  QFile sqlFile(scriptPath);
  if (!sqlFile.open(QFile::ReadOnly)) {
    Log.Error(QString("Open file fail (file: '%1', err: '%2')").arg(scriptPath).arg(sqlFile.errorString()));
    return false;
  }
  QByteArray sqlData = sqlFile.readAll();
  if (sqlData.isEmpty() && sqlFile.size() != 0) {
    Log.Error(QString("Open file fail (file: '%1')").arg(scriptPath));
    return false;
  }
  if (!mDb->Exec(QString::fromUtf8(sqlData))) {
    return false;
  }
  Log.Info(QString("Sql done ('%1')").arg(scriptPath));

  sqlFile.close();
  QFile::remove(scriptPath);
  return true;
}

bool Package::ConnectDb()
{
  if (!mDb) {
    mDb.reset(new Db());
    if (!mDb->OpenDefault()) {
      mDb.reset();
      return false;
    }
  }
  return mDb->Connect();
}


Package::Package()
{
}

Package::~Package()
{
}

