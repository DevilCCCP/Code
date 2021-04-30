#include <QThread>
#include <QFile>

#include <Lib/Log/Log.h>

#include "InstallerCore.h"
#include "PackageCore.h"


int InstallerCore::Exec(int argc, char* argv[])
{
  QByteArray cmd = (argc > 1)? QByteArray(argv[1]): QByteArray();
  QString param1 = (argc > 2)? QString::fromUtf8(argv[2]): QString();
  QString param2 = (argc > 3)? QString::fromUtf8(argv[3]): QString();

  if (cmd == "inst") {
#ifdef QT_NO_DEBUG
    Log.SetFileLogging();
#endif
    return DoInstall(param1);
  } else if (cmd == "clean") {
#ifdef QT_NO_DEBUG
    Log.SetFileLogging();
#endif
    return DoClean(param1);
  } else if (cmd == "pack") {
    return DoPack(param1, param2);
  } else if (cmd == "restore") {
    return DoRestore(param1);
  } else if (cmd == "update") {
    return DoUpdate(param1, param2);
  } else {
    Log.Info(QString("Usage: <exe> <pack|inst|clean|restore|update> <params>"));
  }
  return 0;
}

int InstallerCore::DoPack(const QString& sourceBasePath, const QString& destBasePath)
{
  PackageCore pack;
  if (!sourceBasePath.isEmpty() && !destBasePath.isEmpty()) {
    if (pack.Prepare(sourceBasePath, destBasePath)) {
      return 0;
    }
  }
  Log.Info(QString("Usage: <exe> pack <source path> <dest path>"));
  return 1;
}

int InstallerCore::DoInstall(const QString& sourceBasePath)
{
  InstallerCore inst;
  if (!sourceBasePath.isEmpty()) {
    if (inst.Install(sourceBasePath)) {
      return 0;
    }
  }
  Log.Info(QString("Usage: <exe> inst <source path>"));
  return 1;
}

int InstallerCore::DoClean(const QString& execPath)
{
  QThread::msleep(2000);
  if (!execPath.isEmpty()) {
    while (QFile::exists(execPath) && !QFile::remove(execPath)) {
      Log.Warning(QString("clean fail (file: '%1')").arg(execPath));
      QThread::msleep(500);
    }
    Log.Info("Clean done");
    return 0;
  }
  Log.Info(QString("Usage: <exe> inst <exec path>"));
  return 1;
}

int InstallerCore::DoRestore(const QString& backupPath)
{
  InstallerCore inst;
  if (!inst.RestoreBackup(backupPath)) {
    return 2;
  }

  return 0;
}

int InstallerCore::DoUpdate(const QString& sourceBasePath, const QString& destBasePath)
{
  PackageCore pack;
  if (!sourceBasePath.isEmpty() && !destBasePath.isEmpty()) {
    if (pack.Update(sourceBasePath, destBasePath)) {
      return 0;
    }
  }
  Log.Info(QString("Usage: <exe> update <source path> <dest path>"));
  return 1;
}

bool InstallerCore::Install(const QString& sourceBasePath)
{
  Q_UNUSED(sourceBasePath);

  Log.Error(QString("Install not implemented in light version"));
  return false;
}

bool InstallerCore::RestoreBackup(const QString& backupPath)
{
  Q_UNUSED(backupPath);

  Log.Error(QString("Restore not implemented in light version"));
  return false;
}


InstallerCore::InstallerCore()
{
}

InstallerCore::~InstallerCore()
{
}

