#pragma once

#include <QDir>
#include <QList>
#include <QByteArray>

#include <Lib/UpdaterCore/PackageCore.h>


DefineClassS(Db);

class Package: public PackageCore
{
  DbS                 mDb;

protected:
  /*override */virtual bool ExecSql(const QString& scriptPath) override;
  /*override */virtual bool ConnectDb() override;

public:
  Package();
  ~Package();
};
