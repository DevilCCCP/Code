#pragma once

#include <QProcess>
#include <QSharedMemory>
#include <QSharedPointer>
#include <QElapsedTimer>
#include <QMap>
#include <QList>
#include <QMutex>
#include <Lib/Ctrl/CtrlWorker.h>
#include "MainInfo.h"

DefineClassS(QSharedMemory);
DefineStructS(ModuleLoadInfo);
DefineStructS(ModuleInfo);
DefineStructS(WinSession);

struct MainShmemPage
{
  QSharedMemoryS  Shmem;
  MainInfo*       Info;

  MainShmemPage(const QString& shmemKey): Shmem(new QSharedMemory(shmemKey)), Info(nullptr) { }
};

// Модуль управляет запуском/остановкой процессов
class ProcessManager : public CtrlWorker
{
  const QString mDaemonName;

  QList<MainShmemPage>   mMainShmemPages;
  QMutex                 mMainShmemMutex;

  QMap<int, ModuleInfoS> mModules;
  QList<int>             mFreeIndexes;
  int                    mTopIndex;
  int                    mActiveProcessCount;
  int                    mStartInCircleCount;

  QElapsedTimer          mLiveTimer;
  qint64                 mManagedTime;
  ProcessInfo*           mManageProcess;
  ModuleInfo*            mManageModule;
  int                    mLongWorkWarning;

  QList<ModuleLoadInfoS> mRegisterList;
  QElapsedTimer          mExitTimer;
  bool                   mUpdateDetected;

public:
  /*override */virtual const char* Name() override { return "ProcessManager"; }
  /*override */virtual const char* ShortName() override { return "PM"; }
protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
  /*override */virtual void DoRelease() override;

  /*override */virtual void Stop() override;

private:
  void TestUpdate();
  void RegisterAllPending();
  void RegisterNewModule(ModuleLoadInfoS& moduleInfo);
  void ManageAllProcesses();
  bool ManageProcess();
  bool StartProcess();
  bool WaitProcessStart();
  bool WatchProcessStart();
  bool WatchProcessLive();
  bool WatchProcessStop();
  bool WatchProcessTimeout(int timeoutRestartMs, int timeoutWarningMs);
  void PostProcessRestart(bool crushed = false);

  ModuleInfo* GetModule(int moduleId, const char* warnMessage);

public:
  bool Init();
private:
  bool ValidateShmem();
  bool AddShmemPage();
  bool CreateShmemPage();
  bool InitShmemPage();

  void ClearProcessInfo(int index);
  void UnregisterAllProcesses();
  void KillAllProcesses();
  void ClearShmemProcesses(MainInfo* info, int pageNumber);

  ProcessInfo& GetProcess(int index);

public:
  QMutex& GetMainShmemMutex() { return mMainShmemMutex; }
  void RegisterModule(ModuleLoadInfoS& moduleInfo);
  void RestartModule(ModuleLoadInfoS& moduleInfo);
  void UnregisterModule(int moduleId);
private:
  void ChangeStatus(int moduleId, EProcStatus demandStatus, ModuleInfo** module = nullptr);

public:
  ProcessManager(const QString& _DaemonName);
  /*override */virtual ~ProcessManager();
};

