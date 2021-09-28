#include <QCoreApplication>
#include <QProcess>
#include <QSharedMemory>
#include <QElapsedTimer>
#include <QThread>
#include <QFile>

#include <Lib/Log/Log.h>
#include <Lib/Dispatcher/MainInfo.h>
#include <Lib/Dispatcher/Tools.h>
#include <Local/ModuleNames.h>

#include "Installer.h"
#include "Package.h"
#include "UpInfo.h"


const int kWaitDaemonsLiveMs = 30000;
const int kWaitDaemonsDeadMs = 6000;
QStringList kDaemonNamesList = DAEMONS_NAMES_LIST;
QStringList kDaemonExeList = DAEMONS_EXE_LIST;

bool Installer::Install(const QString& sourceBasePath)
{
#ifdef Q_OS_WIN32
  QDir::setCurrent(QCoreApplication::applicationDirPath());
#else
  QDir::setCurrent(QString("/opt/%1").arg(MAKE_STRING(PROGRAM_ABBR)));
#endif

  StoppingDaemons();
  DeployNew(sourceBasePath);

  CleanShmem();
  RestartDaemons();
  Clean();

  Log.Info("Install done");
  return true;
}

bool Installer::RestoreBackup(const QString& backupPath)
{
#ifdef Q_OS_WIN32
  QDir::setCurrent(QCoreApplication::applicationDirPath());
#else
  QDir::setCurrent(QString("/opt/%1").arg(MAKE_STRING(PROGRAM_ABBR)));
#endif

  StoppingDaemons();

  QProcess restoreProc;
  QString restoreExe = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(kBackupExe);
  restoreProc.start(restoreExe, QStringList() << "--id=-1" << "-t" << QString("-p%1").arg(backupPath));
  restoreProc.waitForFinished(-1);

  CleanShmem();
  RestartDaemons();

  Log.Info("Restore done");
  return true;
}

void Installer::StoppingDaemons()
{
  mNormalMode = true;
  Log.Info(QString("Stopping daemons"));
  forever {
    StoppingDaemonsByCmd();
    if (!mNormalMode) {
      StoppingDaemonsByShmem();
    }

    QElapsedTimer waitTimer;
    waitTimer.start();
    forever {
      bool ok = mNormalMode? CheckDaemonsByShmem(): CheckDaemonsByPid();
      if (ok) {
        return;
      }
      if (waitTimer.elapsed() > kWaitDaemonsDeadMs) {
        Log.Warning(QString("Wait daemons for too long, retry"));
        break;
      }
      QThread::msleep(100);
    }
    mNormalMode = false;
  }
}

void Installer::StoppingDaemonsByCmd()
{
  DaemonsCommand("stop");
}

void Installer::StoppingDaemonsByShmem()
{
  for (auto itr = kDaemonNamesList.begin(); itr != kDaemonNamesList.end(); itr++) {
    const QString& daemon = *itr;
    QSharedMemoryS shmem(new QSharedMemory(MainInfo::GetShmemName(daemon, 0)));
    if (shmem->attach(QSharedMemory::ReadWrite)) {
      MainInfo* info = reinterpret_cast<MainInfo*>(shmem->data());
      if (info->Pid) {
        Log.Info(QString("Stopping daemon by shmem (name: '%1', pid: %2)").arg(daemon).arg(info->Pid));
        mPidList.append(info->Pid);
        info->Pid = 0;
        mShmemLockedList.append(shmem);
      } else {
        Log.Info(QString("Stopped daemon by shmem (name: '%1')").arg(daemon));
        shmem->detach();
      }
    } else if (shmem->error() == QSharedMemory::PermissionDenied) {
      LOG_ERROR_ONCE("Shmem attach 'PermissionDenied'");
    } else if (shmem->error() != QSharedMemory::NotFound) {
      Log.Error(QString("Shmem attach error (name: '%1', code: %2, err: '%3')").arg(daemon).arg(shmem->error()).arg(shmem->errorString()));
    }
  }
}

bool Installer::CheckDaemonsByShmem()
{
  for (auto itr = kDaemonNamesList.begin(); itr != kDaemonNamesList.end(); itr++) {
    const QString& daemon = *itr;
    QSharedMemoryS shmem(new QSharedMemory(MainInfo::GetShmemName(daemon, 0)));
    if (shmem->attach(QSharedMemory::ReadWrite)) {
      shmem->detach();
      return false;
    } else if (shmem->error() != QSharedMemory::NotFound) {
      return false;
    }
  }
  return true;
}

bool Installer::CheckDaemonsByPid()
{
  for (auto itr = mPidList.begin(); itr != mPidList.end(); ) {
    qint64 pid = *itr;

    bool alive;
    int errorCode;
    if (IsProcessAliveByPid(pid, alive, &errorCode)) {
      if (!alive) {
        Log.Info(QString("Process is dead (Pid: %1)").arg(pid));
        itr = mPidList.erase(itr);
        continue;
      }
    }

    itr++;
  }
  return mPidList.isEmpty();
}

void Installer::DeployNew(const QString& sourceBasePath)
{
  Log.Info("Deploy new version");
  Package pack;
  for (int i = 0; !pack.Install(sourceBasePath, mAppDir.path(), i >= 15); i++) {
    QThread::msleep(100);
  }
}

void Installer::CleanShmem()
{
  if (!mShmemLockedList.isEmpty()) {
    Log.Info("Clean locked shmem");
    for (auto itr = mShmemLockedList.begin(); itr != mShmemLockedList.end(); ) {
      const QSharedMemoryS& shmem = *itr;
      shmem->detach();
      itr = mShmemLockedList.erase(itr);
    }
  }
}

void Installer::RestartDaemons()
{
  Log.Info("Start daemons");
  forever {
    DaemonsCommand("start");

    mPidList.clear();
    QElapsedTimer waitTimer;
    waitTimer.start();
    QThread::msleep(2000);
    forever {
      if (!WaitDaemonsByShmem()) {
        break;
      }
      if (waitTimer.elapsed() > kWaitDaemonsLiveMs) {
        Log.Info(QString("Wait daemons live"));
        return;
      }
      QThread::msleep(100);
    }
    Log.Warning(QString("Not all daemons live, retry"));
  }
}

bool Installer::WaitDaemonsByShmem()
{
  for (int i = 0; i < kDaemonNamesList.size(); i++) {
    const QString& daemon = kDaemonNamesList.at(i);
    QSharedMemoryS shmem(new QSharedMemory(MainInfo::GetShmemName(daemon, 0)));
    if (shmem->attach(QSharedMemory::ReadWrite)) {
      MainInfo* info = reinterpret_cast<MainInfo*>(shmem->data());
      if (info->Pid) {
        if (i < mPidList.size()) {
          if (mPidList.at(i) != info->Pid) {
            return false;
          }
        } else {
          mPidList.append(info->Pid);
        }
        shmem->detach();
      } else {
        Log.Info(QString("Daemon is stopped by shmem (name: '%1')").arg(daemon));
        shmem->detach();
        return false;
      }
    } else if (shmem->error() == QSharedMemory::PermissionDenied) {
      Log.Warning("Shmem attach 'PermissionDenied'");
    } else if (shmem->error() == QSharedMemory::NotFound) {
      Log.Warning(QString("Shmem not exists (name: '%1')").arg(daemon));
      return false;
    } else {
      Log.Warning(QString("Shmem attach error (name: '%1', code: %2, err: '%3')").arg(daemon).arg(shmem->error()).arg(shmem->errorString()));
    }
  }
  return true;
}

void Installer::Clean()
{
  Log.Info("Start install clean");
  QProcess::startDetached(mAppDir.absoluteFilePath(kInstallExe), QStringList() << "clean" << QCoreApplication::applicationFilePath());
}

void Installer::DaemonsCommand(const QString& cmd)
{
  for (auto itr = kDaemonExeList.begin(); itr != kDaemonExeList.end(); itr++) {
    const QString& daemonExe = *itr;
    QString prog = mAppDir.absoluteFilePath(daemonExe);
    int ret = QProcess::execute(prog, QStringList() << cmd);
    Log.Info(QString("%1 %2 (ret: %3)").arg(daemonExe, cmd).arg(ret));
  }
}


Installer::Installer()
  : mAppDir(QCoreApplication::applicationDirPath())
  , mNormalMode(true)
{
}

