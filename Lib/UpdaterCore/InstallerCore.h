#pragma once

#include <QDir>
#include <QSharedMemory>
#include <QList>

#include <Lib/Include/Common.h>


DefineClassS(QSharedMemory);

class InstallerCore
{
public:
  static int Exec(int argc, char* argv[]);

  static int DoPack(const QString& sourceBasePath, const QString& destBasePath);
  static int DoInstall(const QString& sourceBasePath);
  static int DoClean(const QString& execPath);
  static int DoRestore(const QString& backupPath);
  static int DoUpdate(const QString& sourceBasePath, const QString& destBasePath);

protected:
  /*new */virtual bool Install(const QString& sourceBasePath);
  /*new */virtual bool RestoreBackup(const QString& backupPath);

public:
  InstallerCore();
  virtual ~InstallerCore();
};

