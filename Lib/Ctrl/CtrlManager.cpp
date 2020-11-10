#include <qsystemdetection.h>
#include <QSysInfo>
#include <QStringBuilder>
#include <QMap>
#ifdef Q_OS_WIN32
#include <Windows.h>
#elif defined(Q_OS_UNIX)
#include <signal.h>
#endif

#include <Lib/Log/Log.h>
#include <Lib/Include/License.h>

#include "CtrlManager.h"
#include "CtrlWorker.h"
#include "WorkerStat.h"


static volatile int gStopSignal = 0;
const int kWatchTimeoutMs = 100;
const int kDefaultPublishPeriodMs = 5 * 60 * 1000;

#ifdef Q_OS_WIN32
static BOOL WINAPI ConsoleCtrlCheck(DWORD)
{
  gStopSignal = 1;
  return TRUE;
}
#elif defined(Q_OS_UNIX)
static void StopSignalHandler(int)
{
  gStopSignal = 1;
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = SIG_DFL;
  ::sigaction(SIGINT, &action, nullptr);
}
#endif

void CtrlManager::RegisterWorker(CtrlWorkerS& _Worker)
{
  QMutexLocker lock(&mMutex);
  if (!mStop) {
    mWorkers.append(_Worker);
    _Worker->SetManager(this);
    _Worker->ConnectManager(this);
    if (mStarted) {
      mAliveCount++;
      _Worker->start();
    }
  }
}

void CtrlManager::SetConsoleBreak()
{
  gStopSignal = 0;
  Log.Info("Using console");
#ifdef Q_OS_WIN32
  if (QSysInfo::windowsVersion() > QSysInfo::WV_XP) {
    Log.Info("Setting console CP to UTF-8");
    SetConsoleOutputCP(65001);
  }
  SetConsoleCtrlHandler(ConsoleCtrlCheck, TRUE);
#elif defined(Q_OS_UNIX)
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = StopSignalHandler;
  ::sigaction(SIGINT, &action, nullptr);
  ::sigaction(SIGTERM, &action, nullptr);
#endif
}

int CtrlManager::Run()
{
  Start();
  mLastWatch.start();
  mPublishTimer.start();
  mPublishStart = QDateTime::currentDateTime();

  int retCode = 0;
  if (InitReport()) {
    QMutexLocker lock(&mMutex);
    while (!mStop) {
      if (gStopSignal) {
        Log.Info(QString("Stop signal received"));
        break;
      }
      int waitMs = kWatchTimeoutMs - mLastWatch.elapsed();
      if (waitMs > 0) {
        LICENSE_CIRCLE(0xAEEB6044);
        if (mStopCondition.wait(&mMutex, waitMs) && mStop) {
          break;
        }
      }

      Watch();
      mLastWatch.restart();
      if (!DoReport()) {
        break;
      }
    }
    LogWorkerStats(true);
    mStop = true;
    lock.unlock();
    FinalReport();
  } else {
    retCode = -3002;
    QMutexLocker lock(&mMutex);
    mStop = true;
  }

  EndChilds();
  return (retCode)? retCode: mAliveCount;
}

void CtrlManager::Stop()
{
  Log.Info("Ask manager to stop");
  QMutexLocker lock(&mMutex);
  mStop = true;
  mStopCondition.wakeAll();
}

void CtrlManager::SetLogWorker(CtrlWorker* _LogWorker)
{
  mLogWorker = _LogWorker;
}

void CtrlManager::Start()
{
  Log.Info("Start workers");
  QMutexLocker lock(&mMutex);
  for (auto itr = mWorkers.begin(); itr != mWorkers.end(); itr++) {
    CtrlWorker* worker = itr->data();
    mAliveCount++;
    worker->start();
  }
  mStarted = true;
}

void CtrlManager::Watch()
{
  for (auto itr = mWorkers.begin(); itr != mWorkers.end(); itr++) {
    CtrlWorker* worker = itr->data();
    if (!worker->Watch()) {
      mMutex.unlock();
      CriticalFail();
      mMutex.lock();
    }
  }
  for (auto itr = mGarbageWorkers.begin(); itr != mGarbageWorkers.end(); ) {
    CtrlWorker* worker = itr->data();
    if (worker->isFinished()) {
      CollectWorkerStats(worker);
      itr = mGarbageWorkers.erase(itr);
    } else {
      itr++;
    }
  }

  LogWorkerStats();
}

void CtrlManager::CollectWorkerStats(CtrlWorker* worker)
{
  const WorkerStatS& aggregateStat = worker->CloseStat();
  if (aggregateStat) {
    mDeadWorkerStatList.insert(aggregateStat);
  }
}

void CtrlManager::LogWorkerStats(bool force)
{
  if (!mConsole && !force && mPublishTimer.elapsed() < mPublishPeriodMs) {
    return;
  }

  QMap<WorkerStat*, WorkerStat*> aggregateStatMap;
  QList<WorkerStatS> statList;

  for (auto itr = mWorkers.begin(); itr != mWorkers.end(); itr++) {
    CtrlWorker* worker = itr->data();
    if (worker != mLogWorker) {
      worker->CollectStats(&statList, &aggregateStatMap);
    }
  }

  foreach (const WorkerStatS& workerStat, mDeadWorkerStatList) {
    auto itr = aggregateStatMap.find(workerStat.data());
    if (itr == aggregateStatMap.end()) {
      statList.append(WorkerStatS(new WorkerStat()));
      itr = aggregateStatMap.insert(workerStat.data(), statList.last().data());
    }
    WorkerStat* aggregateStatTarget = itr.value();
    aggregateStatTarget->AggregateOne(*workerStat);
  }

  if (!mConsole && IsPublicStats()) {
    QDateTime endTime = QDateTime::currentDateTime();
    PublishStats(mPublishStart, endTime, statList);
    mPublishStart = endTime;
  }
  DumpWorkerStats(statList);

  ClearWorkerStats();
}

void CtrlManager::DumpWorkerStats(const QList<WorkerStatS>& statList)
{
  QString allState;
  for (auto itr = statList.begin(); itr != statList.end(); itr++) {
    const WorkerStatS& stat = *itr;
    allState.append(stat->DumpStats() + "; ");
  }
  Log.Status(allState);
}

void CtrlManager::ClearWorkerStats()
{
  if (mConsole && mPublishTimer.elapsed() < mPublishPeriodMs) {
    return;
  }

  for (auto itr = mWorkers.begin(); itr != mWorkers.end(); itr++) {
    CtrlWorker* worker = itr->data();
    worker->ClearWorkerStat();
  }
  mPublishTimer.start();
  mDeadWorkerStatList.clear();
}

bool CtrlManager::InitReport()
{
  return true;
}

bool CtrlManager::DoReport()
{
  return true;
}

void CtrlManager::FinalReport()
{
}

void CtrlManager::CriticalFail()
{
  static bool warned = false;
  if (!warned) {
    Log.Fatal("Critical worker fail");
    warned = true;
  }
}

bool CtrlManager::IsPublicStats()
{
  return false;
}

void CtrlManager::PublishStats(const QDateTime& startTime, const QDateTime& endTime, const QList<WorkerStatS>& statList)
{
  Q_UNUSED(startTime);
  Q_UNUSED(endTime);
  Q_UNUSED(statList);
}

void CtrlManager::EndChilds()
{
  EndChildsAsk() && EndChildsWaitAll() && EndChildsTerminateAlive();
  EndChildsClear();
}

bool CtrlManager::EndChildsAsk()
{
  Log.Info(QString("Ask all workers to stop (cnt: %1)").arg(mWorkers.size()));
  //QMutexLocker lock(&mMutex); no need lock workers at stopping
  for (auto itr = mWorkers.begin(); itr != mWorkers.end(); itr++) {
    CtrlWorker* worker = itr->data();
    worker->Stop();
  }
  return mAliveCount != 0;
}

bool CtrlManager::EndChildsWaitAll()
{
  QElapsedTimer timer;
  timer.start();

  {
    QMutexLocker lock(&mMutex);
    while (mAliveCount) {
      if (!mStopCondition.wait(&mMutex, mEndWorkersTimoutMs)) {
        break;
      }
    }
  }

  if (mAliveCount) {
    Log.Warning(QString("Wait working threads timedout (%1 still alive)").arg(mAliveCount));
  } else {
    Log.Info(QString("All working threads ended safe (in %1ms)").arg(timer.elapsed()));
  }

  return mAliveCount != 0;
}

bool CtrlManager::EndChildsTerminateAlive()
{
  //QMutexLocker lock(&mMutex); no need lock workers at stopping
  for (auto itr = mWorkers.begin(); itr != mWorkers.end(); itr++) {
    CtrlWorker* worker = itr->data();
    if (worker->isRunning()) {
      Log.Warning(QString("Alive worker %1 (%2)").arg(worker->Name()).arg(worker->GetId()));
      //worker->terminate();
    }
  }

  Log.Fatal("Not all threads exited, terminating hole application", true);
  return true;
}

bool CtrlManager::EndChildsClear()
{
  // no need lock, all concurents must rip to this point
  // delete in order of registered
  for (auto itr = mWorkers.begin(); itr != mWorkers.end(); itr++) {
    const CtrlWorkerS& w = *itr;
    w->wait();
    itr->clear();
  }
  for (auto itr = mGarbageWorkers.begin(); itr != mGarbageWorkers.end(); itr++) {
    itr->clear();
  }
  return true;
}

void CtrlManager::OnWorkerFinished()
{
  QThread* exitThread = QThread::currentThread();
  QMutexLocker lock(&mMutex);
  for (auto itr = mWorkers.begin(); itr != mWorkers.end(); itr++) {
    CtrlWorker* worker = itr->data();
    QThread* workerThread = static_cast<QThread*>(worker);
    if (workerThread == exitThread) {
      if (!mStop) {
        mGarbageWorkers.append(*itr);
        mWorkers.erase(itr);
      }
      mAliveCount--;
      break;
    }
  }

  if (!mAliveCount) {
    mStopCondition.wakeAll();
  }
}

CtrlManager::CtrlManager(bool _Console, int _EndWorkersTimoutMs)
  : mConsole(_Console), mEndWorkersTimoutMs(_EndWorkersTimoutMs)
  , mLogWorker(nullptr), mStarted(false), mStop(false), mAliveCount(0)
  , mPublishPeriodMs(kDefaultPublishPeriodMs)
{
#ifdef Q_OS_WIN32
  if (mConsole) {
    SetConsoleBreak();
  }
#else
  SetConsoleBreak();
#endif
}

CtrlManager::~CtrlManager()
{
}

