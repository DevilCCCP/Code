#include <QMutexLocker>

#include <Lib/Log/Log.h>
#include <Lib/Dispatcher/Overseer.h>

#include "Conveyor.h"


const int kStuckTimeout = 2000;
const int kWorkPeriodMs = 2000;
const int kFramesPerCircleLimit = 20;
const int kFrameOverflowCount = 200;

bool Conveyor::DoCircle()
{
  QMutexLocker lock(&Mutex());
  for (int i = 0; IsAlive() && !mConveyorFrames.isEmpty() && i < kFramesPerCircleLimit; i++) {
    mCurrentFrame = mConveyorFrames.takeFirst();
    lock.unlock();
    if (ProcessFrame()) {
      SayWork();
    }
    lock.relock();
    mFrameDone++;
    mCurrentFrame.clear();
  }
  return IsAlive();
}

void Conveyor::DoRelease()
{
  if (mFrameLost > 0) {
    mLossPercent = 100.0 * mFrameLost / (mFrameDone + mFrameLost);
    Log.Error(QString("Conveyour lost frames (done: %1, loss: %2, percent: %3)").arg(mFrameDone).arg(mFrameLost).arg(mLossPercent, 0, 'f', 2));
  }
  QList<Conveyor*> prevImps;
  Mutex().lock();
  prevImps.swap(mPrevImps);
  Mutex().unlock();
  foreach (Conveyor* imp, prevImps) {
    imp->DisconnectModule(this);
  }
}

void Conveyor::ConnectModule(CtrlWorker* worker)
{
  Conveyor* imp = dynamic_cast<Conveyor*>(worker);
  if (imp) {
    Log.Info(QString("Conveyor: connect %1 -> %2").arg(Name()).arg(worker->Name()));
    QMutexLocker lock(&Mutex());
    mNextImps.append(imp);
    imp->ConnectParent(this);
  }
}

void Conveyor::DisconnectModule(CtrlWorker* worker)
{
  QMutexLocker lock(&Mutex());
  Conveyor* imp = dynamic_cast<Conveyor*>(worker);
  if (imp) {
    for (auto itr = mNextImps.begin(); itr != mNextImps.end(); itr++) {
      if (*itr == imp) {
        Log.Info(QString("Conveyor: disconnect %1 ->X %2").arg(Name()).arg(worker->Name()));
        mNextImps.erase(itr);
        return;
      }
    }
  }
}

void Conveyor::OnOverflowWarn()
{
}

bool Conveyor::EmergeFrame(const FrameAS &frame)
{
  bool ok = true;
  SayWork();
  for (auto itr = mNextImps.begin(); itr < mNextImps.end(); itr++) {
    if (!(*itr)->PushFrame(frame)) {
      ok = false;
    }
  }
  return ok;
}

void Conveyor::ConnectParent(Conveyor* conveyor)
{
  QMutexLocker lock(&Mutex());
  mPrevImps.append(conveyor);
}

void Conveyor::DisconnectParent(Conveyor* conveyor)
{
  QMutexLocker lock(&Mutex());
  mPrevImps.removeAll(conveyor);
}

void Conveyor::ClearConveyor()
{
  QMutexLocker lock(&Mutex());
  mConveyorFrames.clear();
}

bool Conveyor::PushFrame(const FrameAS &frame)
{
  if (IsAlive()) {
    QMutexLocker lock(&Mutex());
    bool hasOverflowWarn = HasOverflowWarn();
    if (hasOverflowWarn) {
      bool hasRealOverflow = mConveyorFrames.size() >= mFrameOverflowCount;
      lock.unlock();
      if (hasRealOverflow) {
        ProcessOverflow();
      } else {
        OnOverflowWarn();
      }
    }

    mConveyorFrames.append(frame);
    WakeUp();
    return !hasOverflowWarn;
  }
  return true;
}

void Conveyor::ProcessOverflow()
{
  if (mFrameDone <= mFrameLast) {
    if (mStuckTimer.elapsed() > kStuckTimeout) {
      Log.Fatal(QString("Conveyor is stucked"));
      GetManager()->Stop();
      Stop();
      return;
    }
  } else {
    mStuckTimer.start();
  }

  QMutexLocker lock(&Mutex());
  int sizeBefore = mConveyorFrames.size();
  OnOverflow(mConveyorFrames);

  int sizeAfter = mConveyorFrames.size();
  int done = mFrameDone - mFrameLast;
  mFrameLast = mFrameDone;
  lock.unlock();
  int lost = sizeBefore - sizeAfter;
  mFrameLost += lost;
  qreal lossPercent = 100.0 * lost / (done + lost);
  if (lossPercent > 1.25 * mLossPercent) {
    mLossPercent = lossPercent;
    Log.Error(QString("Conveyour overflow, frames lost (done: %1, loss %2, perc: %3)")
              .arg(done).arg(lost).arg(mLossPercent, 0, 'f', 2));
  }
}

Conveyor::Conveyor(int _FrameOverflowCount, int _WorkPeriodMs)
  : Imp((_WorkPeriodMs >= 0 ? _WorkPeriodMs: kWorkPeriodMs), false)
  , mFrameOverflowCount((_FrameOverflowCount > 0)? _FrameOverflowCount: kFrameOverflowCount)
  , mFrameDone(0), mFrameLost(0), mFrameLast(0), mLossPercent(0)
{
  static int gId = 0;
  mId = ++gId;
}

Conveyor::~Conveyor()
{
}


