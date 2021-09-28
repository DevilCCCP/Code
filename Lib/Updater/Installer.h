#pragma once

#include <QDir>
#include <QSharedMemory>
#include <QList>

#include <Lib/Include/Common.h>
#include <Lib/UpdaterCore/InstallerCore.h>


DefineClassS(QSharedMemory);

class Installer: public InstallerCore
{
  QDir                  mAppDir;
  QList<QSharedMemoryS> mShmemLockedList;
  QList<qint64>         mPidList;
  bool                  mNormalMode;

protected:
  bool Install(const QString& sourceBasePath);
  bool RestoreBackup(const QString& backupPath);

private:
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

  void DaemonsCommand(const QString& cmd);

public:
  Installer();
};

