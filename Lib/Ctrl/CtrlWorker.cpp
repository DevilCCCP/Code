#include <QStringBuilder>
#include <QMutexLocker>

#include <Lib/Common/Format.h>
#include <Lib/Log/Log.h>
#include <Lib/Include/License.h>

#include "CtrlWorker.h"
#include "CtrlManager.h"
#include "WorkerStat.h"


const int kAliveReportMs = 500;
const int kCriticalWarnMs = 5 * 1000;
const int kCriticalFailMs = 30000;

bool CtrlWorker::Watch()
{
  QMutexLocker lock(&mMutex);

  if (mCritical && mCriticalFailMs) {
    qint64 elapsed = mLastWork.elapsed();
    if (elapsed > mCriticalFailMs) {
      return false;
    } else if (elapsed > mCriticalWarnMs + mCriticalWarnExtraMs) {
      Log.Warning(QString("Critical worker not respond (%1)").arg(mCriticalWarnMs + mCriticalWarnExtraMs));
      mCriticalWarnExtraMs += kCriticalWarnMs;
    } else if (elapsed < kCriticalWarnMs) {
      mCriticalWarnExtraMs = 0;
    }
  }

  return true;
}

void CtrlWorker::AddWorkInfo(const QString& name, int index)
{
  QMutexLocker lock(&mMutex);
  mWorkerStat->AddWorkInfo(name, index);
}

void CtrlWorker::ChangeWork(int index)
{
  QMutexLocker lock(&mMutex);
  mWorkerStat->ChangeWork(index);
}

void CtrlWorker::ReturnWork(int index)
{
  QMutexLocker lock(&mMutex);
  mWorkerStat->ReturnWork(index);
}

void CtrlWorker::CountWork(int index, int count)
{
  QMutexLocker lock(&mMutex);
  mWorkerStat->CountWork(index, count);
}

bool CtrlWorker::SayWork()
{
  QMutexLocker lock(&mMutex);
  mFpsCalc.AddFrame();
  mLastWork.start();
  mWorkerStat->UpdateWork(true);
  if (mFailState && mFailTimer.elapsed() > mFailStateDeadMs) {
    Log.Fatal(QString("Too long in fail state, stopping (fail: %1)").arg(FormatTime(mFailStateDeadMs)));
    lock.unlock();
    mManager->Stop();
  }
  return !mStop;
}

void CtrlWorker::SayOk()
{
  if (mFailState) {
    Log.Info(QString("Fail state finished"));
    mFailState = false;
  }
}

void CtrlWorker::SayFail(const QString& reason)
{
  if (!mFailState) {
    if (mFailStateDeadMs <= 0) {
      LOG_WARNING_ONCE(QString("Can't enter fail state, fail time not specified"));
      return;
    }
    Log.Warning(QString("Fail state entered (reason: '%1')").arg(reason));
    mFailState = true;
    mFailTimer.start();
  }
}

void CtrlWorker::Rest()
{
  QMutexLocker lock(&mMutex);
  if (!mStop) {
    mWorkerStat->ChangeWork(-1);
    int workPeriodMs = (int)mLastWork.restart();
    if (!mWakeUp && workPeriodMs < mWorkPeriodMs) {
      mSleepCondition.wait(&mMutex, mWorkPeriodMs - workPeriodMs);
      mLastWork.start();
    }
    mWakeUp = false;
    mWorkerStat->ChangeWork(0);
  }
}

void CtrlWorker::Rest(int period)
{
  QMutexLocker lock(&mMutex);
  if (!mStop && !mWakeUp) {
    mWorkerStat->ChangeWork(-1);
    mSleepCondition.wait(&mMutex, period);
    mWorkerStat->ChangeWork(0);
  }
}

void CtrlWorker::Sleep(int period)
{
  qint64 wakeTime = QDateTime::currentMSecsSinceEpoch() + period;
  QMutexLocker lock(&mMutex);
  if (mStop) {
    return;
  }
  mWorkerStat->ChangeWork(-1);
  while (mSleepCondition.wait(&mMutex, period)) {
    if (mStop) {
      return;
    }
    period = wakeTime - QDateTime::currentMSecsSinceEpoch();
    if (period <= 0) {
      break;
    }
  }
  mWorkerStat->ChangeWork(0);
}

void CtrlWorker::WakeUp()
{
  mSleepCondition.wakeAll();
}

void CtrlWorker::WakeUpConfirmed()
{
  QMutexLocker lock(&mMutex);
  mWakeUp = true;
  mSleepCondition.wakeAll();
}

void CtrlWorker::WorkStart()
{
  QMutexLocker lock(&mMutex);
  mLastWork.start();
}

void CtrlWorker::NotifyFinished()
{
  QMutexLocker lock(&mFinishMutex);
  mFinished = true;
  mFinishCondition.wakeAll();
}

void CtrlWorker::NotifyFinished2()
{
  if (mManager) {
    mManager->OnWorkerFinished();
  }
  if (!mFastThread) {
    Log.Info(QString("%1 exited").arg(Name()));
  }
}

void CtrlWorker::run()
{
  if (!mFastThread) {
    Log.Info(QString("%1 (%2) run").arg(Name()).arg(ShortName()));
  }
  mTid = (qintptr)QThread::currentThreadId();

  try {
    mLastWork.start();
    if (DoInit()) {
      WorkStart();
      mWorkerStat->InitStats(ShortName());
      while (DoCircle()) {
        LICENSE_CIRCLE(0x3E35880);
        if (mAutoWorkCalc) {
          SayWork();
        }
        if (mWorkPeriodMs) {
          Rest();
        }
        if (IsStop()) {
          break;
        }
      }

      DoRelease();
    }
  } catch (FatalException&) {
    Log.Info(QString("%1 ask process to safely stop").arg(Name()));
    mManager->Stop();
  } catch (...) {
    Log.Info(QString("Unhandled exception in %1").arg(Name()));
    mManager->Stop();
  }
  NotifyFinished();
  NotifyFinished2();
}

bool CtrlWorker::DoInit()
{
  return true;
}

void CtrlWorker::DoRelease()
{
}

void CtrlWorker::Stop()
{
  mStop = true;
  WakeUp();
}

bool CtrlWorker::WaitFinish(unsigned long time)
{
  QMutexLocker lock(&mFinishMutex);
  if (!mFinished) {
    return mFinishCondition.wait(&mFinishMutex, time);
  }
  return true;
}

void CtrlWorker::SetAggregateWorkerStat(const WorkerStatS& _AggregateWorkerStat)
{
  mAggregateWorkerStat = _AggregateWorkerStat;
}

void CtrlWorker::CollectStats(QList<WorkerStatS>* statList, QMap<WorkerStat*, WorkerStat*>* aggregateStatMap)
{
  if (mAggregateWorkerStat) {
    auto itr = aggregateStatMap->find(mAggregateWorkerStat.data());
    if (itr == aggregateStatMap->end()) {
      statList->append(WorkerStatS(new WorkerStat()));
      itr = aggregateStatMap->insert(mAggregateWorkerStat.data(), statList->last().data());
    }
    WorkerStat* aggregateStatTarget = itr.value();

    QMutexLocker lock(&mMutex);
    aggregateStatTarget->AggregateOne(*mWorkerStat);
  } else {
    QMutexLocker lock(&mMutex);
    WorkerStatS workerStat = mWorkerStat->Clone();
    statList->append(workerStat);
  }
}

void CtrlWorker::ClearWorkerStat()
{
  QMutexLocker lock(&mMutex);
  mWorkerStat->Clear();
}

WorkerStatS CtrlWorker::CloseStat()
{
  QMutexLocker lock(&mMutex);
  if (mAggregateWorkerStat) {
    mAggregateWorkerStat->AggregateOne(*mWorkerStat);
    mAggregateWorkerStat->AddCloseOne();
    return mAggregateWorkerStat;
  }
  return WorkerStatS();
}


CtrlWorker::CtrlWorker(int _WorkPeriodMs, bool _AutoWorkCalc, bool _Critical, int _CriticalFailMs)
  : mWorkPeriodMs(_WorkPeriodMs), mAutoWorkCalc(_AutoWorkCalc), mCritical(_Critical), mCriticalFailMs(_CriticalFailMs > 0? _CriticalFailMs: kCriticalFailMs)
  , mFastThread(false)
  , mManager(nullptr), mWakeUp(false), mStop(false)
  , mFinished(false), mCriticalWarnMs(kCriticalWarnMs), mCriticalWarnExtraMs(0)
  , mWorkerStat(new WorkerStat())
  , mFailState(false), mFailStateDeadMs(0)
{
  mLastWork.start();
  mFailTimer.start();
}

CtrlWorker::~CtrlWorker()
{
}
