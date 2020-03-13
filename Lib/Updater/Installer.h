#pragma once

#include <QDir>
#include <QSharedMemory>
#include <QList>

#include <Lib/Include/Common.h>


DefineClassS(QSharedMemory);

class Installer
{
  QDir                  mAppDir;
  QList<QSharedMemoryS> mShmemLockedList;
  QList<qint64>         mPidList;
  bool                  mNormalMode;

public:
  static int Exec(int argc, char* argv[]);

  static int DoPack(const QString& sourceBasePath, const QString& destBasePath);
  static int DoInstall(const QString& sourceBasePath);
  static int DoClean(const QString& execPath);
  static int DoRestore(const QString& backupPath);

private:
  bool Install(const QString& sourceBasePath);
  bool RestoreBackup(const QString& backupPath);
  void StoppingDaemons();
  void StoppingDaemonsByCmd();
  void StoppingDaemonsByShmem();
  bool CheckDaemonsByShmem();
  bool CheckDaemonsByPid();
  void DeployNew(const QString& sourceBasePath);
  void CleanShmem();
  void RestartDaemons();
  bool WaitDaemonsByShmem();
  void Clean();

public:
  Installer();
};

