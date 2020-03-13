#include <QCoreApplication>
#include <qsystemdetection.h>

#include <Lib/Log/Log.h>
#include <Lib/Ctrl/CtrlManager.h>

#include "ProcessManager.h"
#include "ModuleInfo.h"
#ifdef Q_OS_WIN32
#include "Win/WinTools.h"
#else
#include "Linux/LinuxTools.h"
#endif


const int kWorkPeriodMs = 100;
const int kProcessPerPage = 20;
const int kStartInCircleCountMax = 5;
const int kStartWaitDispatcherAliveMs = 13000;

const int kProcessWaitEndsMs = 5000;
const int kProcessWaitEndsWarningMs = 2000;

const int kProcessCrushRestartCountMax = 10;
const int kProcessCrushRestartDelayMaxMs = 2000;
const int kProcessCrushCounterClearMs = 15*60000;

const int kProcessStartMs = 60000;
const int kProcessInitMs = 120000;
const int kProcessLiveMs = 10000;
const int kProcessMiningMs = 60000;
const int kProcessStopMs = 5000;
const int kProcessWarnLiveMs = 3000;
const int kProcessWarnMiningMs = 30000;

bool ProcessManager::DoInit()
{
#ifdef Q_OS_WIN32
  SetDebugPrivilege(true);
#endif
  return true;
}

bool ProcessManager::DoCircle()
{
  RegisterAllPending();
  ManageAllProcesses();
  TestUpdate();
  return true;
}

void ProcessManager::DoRelease()
{
  bool warn = false;
  while (mExitTimer.elapsed() < kProcessWaitEndsMs) {
    if (!warn && mExitTimer.elapsed() > kProcessWaitEndsWarningMs) {
      Log.Warning(QString("Wait ending processes too long"));
      warn = true;
    }
    ManageAllProcesses();
    if (mActiveProcessCount == 0) {
      Log.Info(QString("All processes exited in %1 ms").arg(mExitTimer.elapsed()));
      return;
    }
    msleep(kWorkPeriodMs / 8 + 1);
  }
  Log.Warning(QString("Not all processes exited (live: %1)").arg(mActiveProcessCount));
  KillAllProcesses();
}

void ProcessManager::Stop()
{
  mExitTimer.start();
  UnregisterAllProcesses();

  CtrlWorker::Stop();
}

void ProcessManager::TestUpdate()
{
  if (mUpdateDetected) {
    return;
  }

  QMutexLocker lock(&mMainShmemMutex);
  if (!mMainShmemPages.isEmpty()) {
    MainShmemPage* page = &mMainShmemPages.first();
    if (page->Info->Pid == 0) {
      Log.Info(QString("Update detected"));
      GetManager()->Stop();
      mUpdateDetected = true;
    }
  }
}

void ProcessManager::RegisterAllPending()
{
  QMutexLocker lock(&mMainShmemMutex);
  while (IsAlive() && !mRegisterList.empty()) {
    ModuleLoadInfoS moduleLoadInfo = mRegisterList.takeFirst();
    RegisterNewModule(moduleLoadInfo);
  }
}

void ProcessManager::RegisterNewModule(ModuleLoadInfoS &moduleInfo)
{
  if (mFreeIndexes.empty()) {
    if (!AddShmemPage()) {
      Log.Fatal("Can't add new shmem page", true);
    }
  }
  if (ModuleInfo* info = GetModule(moduleInfo->Id, nullptr)) {
    int index = info->Index;
    ProcessInfo& pi = GetProcess(index);
    Log.Info(QString("Register module at %1 rewrite (id: %2, path: %3, params: \"%4\")")
             .arg(index).arg(moduleInfo->Id).arg(moduleInfo->Path).arg(moduleInfo->Params.join("\" \"")));
    mModules[moduleInfo->Id]->Rewrite(*moduleInfo);
    pi.DemandStatus = eRestart;
  } else {
    int index = mFreeIndexes.takeFirst();
    mTopIndex = qMax(mTopIndex, index);
    ProcessInfo& pi = GetProcess(index);
    Log.Info(QString("Register module at %1 (id: %2, path: %3, params: \"%4\")")
             .arg(index).arg(moduleInfo->Id).arg(moduleInfo->Path).arg(moduleInfo->Params.join("\" \"")));
    ModuleInfoS module(new ModuleInfo(*moduleInfo, index));
    mModules[moduleInfo->Id] = module;
    pi.Init(moduleInfo->Id, mLiveTimer.elapsed());
  }
}

void ProcessManager::ManageAllProcesses()
{
  mActiveProcessCount = 0;
  mStartInCircleCount = 0;

  int firstIndex = 0;
  mManagedTime = mLiveTimer.elapsed();
  QMutexLocker lock(&mMainShmemMutex);
  for (auto itr = mMainShmemPages.begin(); itr != mMainShmemPages.end(); itr++, firstIndex += kProcessPerPage) {
    MainShmemPage* page = &*itr;
    page->Info->LastAlive = mManagedTime;
    int topIndex = qMin(mTopIndex - firstIndex, firstIndex + kProcessPerPage - 1);
    for (int index = 0; index <= topIndex; index++) {
      mManageProcess = &page->Info->Process(index);
      if (mManageProcess->IsInit()) {
        mManageModule = GetModule(mManageProcess->Id, "Manage");
        if (!mManageModule || !ManageProcess()) {
          ClearProcessInfo(index);
        }
      }
    }
  }
  lock.unlock();

  qint64 workTime = mLiveTimer.elapsed() - mManagedTime;
  if (workTime > mLongWorkWarning + mStartInCircleCount * 20) {
    Log.Warning(QString("Manager works for too long (time: %1 ms, starts: %2, pages: %3)").arg(workTime).arg(mStartInCircleCount).arg(mMainShmemPages.size()));
    mLongWorkWarning = (workTime - mStartInCircleCount * 20) * 3/2;
  }
}

/* Status graph:
 * eWaitStart | eWaitStart <- init
 * eWaitStart | eStart <- start
 * timeout -> error
 * eInitialize | eStart -> eInitialize | eInitialize
 * eLive | eInitialize -> eLive | eLive
 * eLive | eLive
 * extra work -> eMining | eLive
 * fatal -> eZombie | eLive
 * need restart -> eRestart | eLive
 * all except eWaitStart | X: timeout -> restart -> init
*/
bool ProcessManager::ManageProcess()
{
  switch (mManageProcess->DemandStatus)
  {
  case eWaitStart:
    return StartProcess();

  case eStart:
    return WatchProcessStart();

  case eInitialize:
  case eLive:
  case eMining:
    return WatchProcessLive();

  case eStop:
  case eRestart:
  case eClear:
    return WatchProcessStop();

  case eProcNone: // empty slot
  case eZombie:   // live as is, no watch (not used)
  case eRip:      // ended normal, restart will not help or not needed
  case eCrushed:  // ended ubnormal or as in this case not started at all
    return true;
  default:        // undefined behavor
    Log.Warning(QString("Undefined DemandStatus %1(%2)").arg(mManageProcess->DemandStatus).arg(EProcStatus_ToString(mManageProcess->DemandStatus)));
    return false;
  }
}

bool ProcessManager::StartProcess()
{
  if (WaitProcessStart()) {
    return true;
  }

#ifdef Q_OS_WIN32
  mStartInCircleCount++;
  if (StartProcessByPid(mManageModule->Path, mManageModule->Params, mManageModule->Pid)) {
    mManageModule->StartWarned = false;
    mManageProcess->Pid          = mManageModule->Pid;
    mManageProcess->DemandStatus = eStart;
    mManageProcess->LastAliveMs  = mManagedTime;
    Log.Info(QString("Started module (id: %1, Pid: %2)").arg(mManageProcess->Id).arg(mManageProcess->Pid));
  } else {
    if (!mManageModule->StartWarned) {
      Log.Error(QString("Starting module fail (id: %1)").arg(mManageProcess->Id));
      mManageModule->StartWarned = true;
    }
    PostProcessRestart(true);
  }
#else
  QStringList params = mManageModule->Params;
  const QProcessS& proc = mManageModule->Process;
  proc->waitForFinished(0);
  proc->setProgram(mManageModule->Path);
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LD_LIBRARY_PATH", qApp->applicationDirPath());
  int displayInd = params.indexOf(QRegExp("DISPLAY=*", Qt::CaseSensitive, QRegExp::Wildcard));
  QString envText;
  if (displayInd >= 0) {
    const QString& displayText = params.at(displayInd);
    int indEq = displayText.indexOf('=');
    QString key = displayText.mid(0, indEq).trimmed();
    QString value = displayText.mid(indEq + 1).trimmed();
    envText.append(QString("'%1'='%2'").arg(key).arg(value));
    env.insert(key, value);
    params.removeAt(displayInd);
  }
  proc->setArguments(params);
  proc->setProcessEnvironment(env);
  proc->start();
  mStartInCircleCount++;
  if (proc->state() != QProcess::NotRunning) {
    mManageProcess->Pid = (qint64)proc->pid();
    Log.Info(QString("Started module (id: %1, Pid: %2, env: %3)").arg(mManageProcess->Id).arg(mManageProcess->Pid).arg(envText));
    mManageModule->StartWarned = false;
    mManageProcess->DemandStatus = eStart;
    mManageProcess->LastAliveMs = mManagedTime;
  } else {
    if (!mManageModule->StartWarned) {
      Log.Error(QString("Starting module fail '%2' (id: %1)").arg(mManageProcess->Id).arg(proc->errorString()));
      mManageModule->StartWarned = true;
    }
    PostProcessRestart(true);
  }
#endif
  return true;
}

bool ProcessManager::WaitProcessStart()
{
  if (mStartInCircleCount >= kStartInCircleCountMax) {
    return true;
  }
  if (mManageModule->CrushCount > 0) {
    int crushes = qMin(mManageModule->CrushCount, kProcessCrushRestartCountMax);
    qint64 waitMs = kProcessCrushRestartDelayMaxMs * crushes / kProcessCrushRestartCountMax;
    return mManagedTime - mManageProcess->LastAliveMs < waitMs;
  }
  return false;
}

bool ProcessManager::WatchProcessStart()
{
#ifdef Q_OS_WIN32
  bool ok;
  if (IsProcessAliveByPid(mManageModule->Pid, ok)) {
    if (ok) {
      Log.Trace(QString("Running module (id: %1)").arg(mManageProcess->Id));
      mManageProcess->DemandStatus = eInitialize;
      mManageProcess->LastAliveMs  = mManagedTime;
    } else if (mManagedTime - mManageProcess->LastAliveMs > kProcessStartMs) {
      Log.Error(QString("Process is not started (id: %1)").arg(mManageProcess->Id));
      mManageProcess->DemandStatus = eCrushed;
    }
  } else {
    LOG_WARNING_ONCE("ProcessAliveByPid fail");
  }
  return true;
#else
  const QProcessS& proc = mManageModule->Process;
  if (proc->waitForStarted(0)) {
    Log.Trace(QString("Running module (id: %1)").arg(mManageProcess->Id));
    mManageProcess->DemandStatus = eInitialize;
    mManageProcess->LastAliveMs = mManagedTime;
  } else if (mManageProcess->IsLive(mManagedTime)) {
    Log.Trace(QString("Respond module (id: %1)").arg(mManageProcess->Id));
    if (proc->state() != QProcess::Running) {
      LOG_WARNING_ONCE("Qt QProcess::state not running, but process response");
    }
    mManageProcess->DemandStatus = eInitialize;
  } else {
    if (mManagedTime - mManageProcess->LastAliveMs > kProcessStartMs) {
      Log.Error(QString("Process is not started (id: %1)").arg(mManageProcess->Id));
      mManageProcess->DemandStatus = eCrushed;
    }
  }
#endif
  return true;
}

bool ProcessManager::WatchProcessLive()
{
  EProcStatus curStatus = mManageProcess->CurrentStatus;
  switch (curStatus) {
  case eWaitStart:  // process not said anything
  case eInitialize: // process in initialize state
    if (mManageProcess->DemandStatus != eInitialize) {
      LOG_WARNING_ONCE(QString("Process responds '%1'(%2) in %3, this is wrong")
                       .arg(curStatus).arg(EProcStatus_ToString(curStatus)).arg(EProcStatus_ToString(mManageProcess->DemandStatus)));
    }
    return WatchProcessTimeout(kProcessInitMs, kProcessInitMs);

  case eLive:       // process in live state
    if (mManageProcess->DemandStatus < eLive) {
      Log.Info(QString("Process is live (id: %1)").arg(mManageProcess->Id));
      mManageProcess->DemandStatus = eLive;
    }
    return WatchProcessTimeout(kProcessLiveMs, kProcessWarnLiveMs);

  case eMining:     // process in maining state (doing hard blocking work)
    if (mManageProcess->DemandStatus < eLive) {
      Log.Info(QString("Process is live (id: %1)").arg(mManageProcess->Id));
      mManageProcess->DemandStatus = eLive;
    }
    return WatchProcessTimeout(kProcessMiningMs, kProcessWarnMiningMs);

  case eStop:       // process in stop state
  case eRestart:    // process in stop state and ask to restart
    if (mManageProcess->DemandStatus != curStatus) {
      Log.Info(QString("Process want to %2 (id: %1)").arg(mManageProcess->Id).arg(EProcStatus_ToString(curStatus)));
      mManageProcess->DemandStatus = curStatus;
    }
    return true;

  case eZombie:     // process in stop state and not want alive
    if (mManageProcess->DemandStatus != curStatus) {
      Log.Info(QString("Process done it's work and now %2 (id: %1)").arg(mManageProcess->Id).arg(EProcStatus_ToString(curStatus)));
      mManageProcess->DemandStatus = curStatus;
    }
    return true;

  case eProcNone:   // process not respond this, this is error
  case eStart:      // process not respond this, this is error
  case eClear:      // process not respond this, this is error
  case eRip:        // process not respond this, this is error
  case eCrushed:    // process not respond this, this is error
  default:          // undefined behavor, this is error too
    LOG_WARNING_ONCE(QString("Process responds '%1'(%2), this is wrong").arg(curStatus).arg(EProcStatus_ToString(curStatus)));
    return true;
  }
}

bool ProcessManager::WatchProcessStop()
{
#ifdef Q_OS_WIN32
  bool running;
  if (!IsProcessAliveByPid(mManageModule->Pid, running)) {
    LOG_WARNING_ONCE("ProcessAliveByPid fail");
    running = true;
  }
#else
  const QProcessS& proc = mManageModule->Process;
  bool running = proc->state() == QProcess::Running
      && !proc->waitForFinished(0);
#endif

  if (mManagedTime - mManageProcess->LastAliveMs > kProcessStopMs) {
    Log.Warning(QString("Process is not ending for too long, killing (id: %1)").arg(mManageProcess->Id));
#ifdef Q_OS_WIN32
    KillProcessByPid(mManageModule->Pid);
#else
    mManageModule->Process->kill();
#endif
    mManageProcess->LastAliveMs = mManagedTime;
  }

  if (!running) {
    Log.Info(QString("Process is finished (id: %1)").arg(mManageProcess->Id));
    mManageProcess->CurrentStatus = eRip;
    if (mManageProcess->DemandStatus == eRestart) {
      PostProcessRestart();
    } else if (mManageProcess->DemandStatus == eClear) {
      ClearProcessInfo(mManageModule->Index);
    } else {
      mManageProcess->DemandStatus = eRip;
    }
  }

  if (running) {
    mActiveProcessCount++;
  }
  return true;
}

bool ProcessManager::WatchProcessTimeout(int timeoutRestartMs, int timeoutWarningMs)
{
#ifdef Q_OS_WIN32
  bool running;
  int retCode;
  if (!IsProcessAliveByPid(mManageModule->Pid, running, &retCode)) {
    running = true;
    LOG_WARNING_ONCE("ProcessAliveByPid fail");
  }
#else
  const QProcessS& proc = mManageModule->Process;
  bool running = !proc->waitForFinished(0);
  int retCode = proc->exitCode();
#endif
  if (!running) {
    if (mManageProcess->CurrentStatus & eFlagLive) {
      Log.Info(QString("Process crushed (id: %1)").arg(mManageProcess->Id));
      PostProcessRestart(true);
    } else if (mManageProcess->CurrentStatus == eRestart) {
      Log.Info(QString("Process want restart (id: %1, exit: %2)")
               .arg(mManageProcess->Id).arg(retCode));
      PostProcessRestart(false);
    } else if ((mManageProcess->CurrentStatus & eFlagEnded) || mManageProcess->CurrentStatus == eStop) {
      Log.Info(QString("Process want %2 (id: %1, exit: %3)")
               .arg(mManageProcess->Id).arg(EProcStatus_ToString(mManageProcess->CurrentStatus)).arg(retCode));
      mManageProcess->DemandStatus = mManageProcess->CurrentStatus;
    } else {
      Log.Warning(QString("Process finished from %2 (id: %1, exit: %3)")
                  .arg(mManageProcess->Id).arg(EProcStatus_ToString(mManageProcess->CurrentStatus)).arg(retCode));
      mManageProcess->DemandStatus = mManageProcess->CurrentStatus;

      PostProcessRestart(true);
    }
    return true;
  }

  mActiveProcessCount++;
  if (mManageProcess->IsLive(mManagedTime)) {
    if (mManageModule->TimeoutWarned) {
      Log.Info(QString("Process is active (id: %1)").arg(mManageProcess->Id));
      mManageModule->TimeoutWarned = false;
    }
  } else if (mManagedTime - mManageProcess->LastAliveMs > timeoutRestartMs) {
    Log.Info(QString("Process is not respond for too long, killing (id: %1)").arg(mManageProcess->Id));
#ifdef Q_OS_WIN32
    KillProcessByPid(mManageModule->Pid);
#else
    mManageModule->Process->kill();
#endif
    PostProcessRestart(true);
  } else if (mManagedTime - mManageProcess->LastAliveMs > timeoutWarningMs) {
    Log.Info(QString("Process is silent (id: %1)").arg(mManageProcess->Id));
    mManageModule->TimeoutWarned = true;
  }
  return true;
}

void ProcessManager::PostProcessRestart(bool crushed)
{
  mManageModule->TimeoutWarned = false;
  if (mManageModule->CrushCount) {
    if (mManagedTime - mManageModule->LastCrush > kProcessCrushCounterClearMs) {
      mManageModule->CrushCount = 0;
    }
  }
  if (crushed) {
    mManageModule->CrushCount++;
    mManageModule->LastCrush = mManagedTime;
  }

  mManageProcess->CurrentStatus = eWaitStart;
  mManageProcess->DemandStatus = eWaitStart;
  mManageProcess->LastAliveMs = mManagedTime;
}

ModuleInfo* ProcessManager::GetModule(int moduleId, const char *warnMessage)
{
  auto itr = mModules.find(moduleId);
  if (itr == mModules.end()) {
    if (warnMessage) {
      Log.Error(QString("%2 unregistered module (id: %1)").arg(moduleId).arg(warnMessage));
    }
    return nullptr;
  }
  return itr.value().data();
}

bool ProcessManager::Init()
{
  if (!ValidateShmem()) {
    Log.Fatal("Can't use main shmem");
    return false;
  }

  if (!AddShmemPage()) {
    Log.Fatal("Can't add page to main shmem");
    return false;
  }

  return true;
}

bool ProcessManager::ValidateShmem()
{
  QElapsedTimer retry;
  retry.start();
  QSharedMemory shmem(MainInfo::GetShmemName(mDaemonName, mMainShmemPages.count()));
  qint64 lastAlive = 0;
  bool clear = false;
  while (shmem.attach(QSharedMemory::ReadOnly)) {
    MainInfo* info = reinterpret_cast<MainInfo*>(shmem.data());
    if (info->Pid == 0) {
      Log.Info("Shmem exists and inaccessible (Pid: 0)");
      return false;
    } else if (!info->Validate(shmem.size())) {
      bool alive;
      if (IsProcessAliveByPid(info->Pid, alive)) {
        if (alive) {
          Log.Error(QString("Find alive incompatible old dispatcher (Pid: %1)").arg(info->Pid));
          if (!KillProcessByPid(info->Pid)) {
            Log.Fatal("Kill process fail, very bad");
            return false;
          }
        } else {
          Log.Error(QString("Find dead incompatible old dispatcher (Pid: %1)").arg(info->Pid));
        }
      } else {
        Log.Error(QString("ProcessAliveByPid fail (Pid: %1)").arg(info->Pid));
      }
    } else if (lastAlive) {
      if (info->LastAlive != lastAlive) {
        Log.Info("Dispatcher exists and work");
        return false;
      } else if (retry.elapsed() > kStartWaitDispatcherAliveMs) {
        if (!clear) {
          Log.Info("Dispatcher exists but not respond, clear all");
          ClearShmemProcesses(info, mMainShmemPages.count());
          clear = true;
          retry.restart();
        } else {
          Log.Fatal("Shmem still unavailable");
          return false;
        }
      }
    } else {
      lastAlive = info->LastAlive;
    }
    shmem.detach();
  }
  return true;
}

bool ProcessManager::AddShmemPage()
{
  return CreateShmemPage() && InitShmemPage();
}

bool ProcessManager::CreateShmemPage()
{
  if (!mMainShmemPages.empty()) {
    mMainShmemPages.last().Info->Next = true;
  }
  int pageNumber = mMainShmemPages.count();
  mMainShmemPages.append(MainShmemPage(MainInfo::GetShmemName(mDaemonName, pageNumber)));
  MainShmemPage& newPage = mMainShmemPages.last();
  if (!newPage.Shmem->create(MainInfo::Size(kProcessPerPage))) {
    Log.Warning("Can't create new shmem page, try recover");
    QSharedMemory shmemNext(MainInfo::GetShmemName(mDaemonName, pageNumber));
    if (shmemNext.attach(QSharedMemory::ReadOnly)) {
      MainInfo* info = reinterpret_cast<MainInfo*>(shmemNext.data());
      ClearShmemProcesses(info, mMainShmemPages.count());
    }
    if (!newPage.Shmem->create(MainInfo::Size(kProcessPerPage))) {
      return false;
    }
  }
  Log.Info(QString("Created new shmem '%1'").arg(newPage.Shmem->key()));
  return true;
}

bool ProcessManager::InitShmemPage()
{
  MainShmemPage& newPage = mMainShmemPages.last();
  newPage.Info = new(newPage.Shmem->data()) MainInfo(kProcessPerPage, QCoreApplication::instance()->applicationPid());
  int firstIndex = kProcessPerPage * (mMainShmemPages.count() - 1);
  int lastIndex = firstIndex + kProcessPerPage;
  for (int index = firstIndex; index < lastIndex; index++) {
    newPage.Info->Process(index - firstIndex) = ProcessInfo();
    mFreeIndexes.append(index);
  }
  return true;
}

void ProcessManager::ClearProcessInfo(int index)
{
  mModules.remove(mManageProcess->Id);
  mFreeIndexes.push_front(index);
  mManageProcess->Clear();
  if (mTopIndex == index) {
    do {
      mTopIndex--;
    } while (mFreeIndexes.contains(mTopIndex));
  }
}

void ProcessManager::UnregisterAllProcesses()
{
  Log.Info(QString("Unregister all processes"));
  int firstIndex = 0;
  QMutexLocker lock(&mMainShmemMutex);
  for (auto itr = mMainShmemPages.begin(); itr != mMainShmemPages.end(); itr++, firstIndex += kProcessPerPage) {
    MainShmemPage& page = *itr;
    int topIndex = qMin(mTopIndex - firstIndex, firstIndex + kProcessPerPage - 1);
    for (int index = 0; index <= topIndex; index++) {
      mManageProcess = &page.Info->Process(index);
      if (mManageProcess->DemandStatus != eProcNone) {
        if (mManageProcess->CurrentStatus == eRip && !(mManageProcess->DemandStatus & eFlagStarting)) {
          ClearProcessInfo(index);
        } else {
          mManageProcess->DemandStatus = eClear;
        }
      }
    }
  }
}

void ProcessManager::KillAllProcesses()
{
#ifndef Q_OS_WIN32
  QList<QProcessS> aliveProcess;
#endif
  for (auto itr = mModules.begin(); itr != mModules.end(); itr++) {
    ModuleInfo& module = *itr.value();
#ifdef Q_OS_WIN32
    bool alive;
    if (!IsProcessAliveByPid(module.Pid, alive)) {
      LOG_WARNING_ONCE("ProcessAliveByPid fail");
      continue;
    }
    if (alive) {
      Log.Warning(QString("Process killing (id: %1)").arg(module.Id));
      KillProcessByPid(module.Pid);
    }
#else
    const QProcessS& proc = module.Process;
    if (proc->state() != QProcess::NotRunning && !proc->waitForFinished(0)) {
      Log.Warning(QString("Process killing (id: %1)").arg(module.Id));
      proc->kill();
      aliveProcess.append(proc);
    }
#endif
  }
  mModules.clear();

#ifndef Q_OS_WIN32
  QElapsedTimer endTimer;
  endTimer.start();
  while (endTimer.elapsed() < 500) {
    for (auto itr = aliveProcess.begin(); itr != aliveProcess.end(); ) {
      const QProcessS& proc = *itr;
      if (proc->state() == QProcess::NotRunning || proc->waitForFinished(1)) {
        itr = aliveProcess.erase(itr);
      } else {
        itr++;
      }
    }
  }
#endif
}

void ProcessManager::ClearShmemProcesses(MainInfo* info, int pageNumber)
{
  for (int index = 0; index <= info->ProcessCount; index++) {
    ProcessInfo& pi = info->Process(index);
    if (pi.IsInit()) {
      if (KillProcessByPid(pi.Pid)) {
        Log.Info(QString("Kill process (Id: %1, Pid: %2)").arg(pi.Id).arg(pi.Pid));
      } else {
        Log.Error(QString("Kill process fail (Id: %1, Pid: %2)").arg(pi.Id).arg(pi.Pid));
      }
    }
  }
  if (info->Next) {
    for (int nextPageNumber = pageNumber + 1; nextPageNumber <= pageNumber + 5; nextPageNumber++) {
      QSharedMemory shmem(MainInfo::GetShmemName(mDaemonName, nextPageNumber));
      if (shmem.attach(QSharedMemory::ReadOnly)) {
        MainInfo* nextInfo = reinterpret_cast<MainInfo*>(shmem.data());
        ClearShmemProcesses(nextInfo, nextPageNumber);
        shmem.detach();
        return;
      } else {
        Log.Warning(QString("Can't attach next shmem page (page: %1)").arg(nextPageNumber));
      }
    }
  }
}

ProcessInfo &ProcessManager::GetProcess(int index)
{
  int page = index / kProcessPerPage;
  int ind = index % kProcessPerPage;
  MainShmemPage& currentPage = mMainShmemPages[page];
  return currentPage.Info->Process(ind);
}

void ProcessManager::RegisterModule(ModuleLoadInfoS &moduleInfo)
{
  mRegisterList.append(moduleInfo);
}

void ProcessManager::RestartModule(ModuleLoadInfoS& moduleInfo)
{
  if (ModuleInfo* module = GetModule(moduleInfo->Id, nullptr)) {
    ChangeStatus(moduleInfo->Id, eRestart, nullptr);
    Log.Info(QString("Update module %1 rewrite (path: '%2', params: \"%3\")")
          .arg(moduleInfo->Id).arg(moduleInfo->Path).arg(moduleInfo->Params.join("\" \"")));
    module->Update(*moduleInfo);
  } else {
    for (auto itr = mRegisterList.begin(); itr != mRegisterList.end(); itr++) {
      ModuleLoadInfoS& info = *itr;
      if (info->Id == moduleInfo->Id) {
        info->Path = moduleInfo->Path;
        info->Params = moduleInfo->Params;
        return;
      }
    }
    Log.Warning(QString("Ask restart unregistered module (id: %1)").arg(moduleInfo->Id));
  }
}

void ProcessManager::UnregisterModule(int moduleId)
{
  Log.Info(QString("Unregister module (id: %1)").arg(moduleId));
  ChangeStatus(moduleId, eClear);
}

void ProcessManager::ChangeStatus(int moduleId, EProcStatus demandStatus, ModuleInfo** moduleInfo)
{
  if (ModuleInfo* module = GetModule(moduleId, EProcStatus_ToString(demandStatus))) {
    if (moduleInfo) {
      *moduleInfo = module;
    }
    ProcessInfo& pi = GetProcess(module->Index);
    Log.Info(QString("%1 module ask (id: %3, idx: %4)").arg(EProcStatus_ToString(demandStatus)).arg(moduleId).arg(module->Index));
    pi.DemandStatus = demandStatus;
  }
}

ProcessManager::ProcessManager(const QString &_DaemonName)
  : CtrlWorker(kWorkPeriodMs)
  , mDaemonName(_DaemonName), mTopIndex(-1)
  , mManagedTime(0), mManageProcess(nullptr), mManageModule(nullptr), mLongWorkWarning(kWorkPeriodMs/2)
  , mUpdateDetected(false)
{
  mLiveTimer.start();
}

ProcessManager::~ProcessManager()
{
}


