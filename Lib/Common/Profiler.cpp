#include <Lib/Log/Log.h>

#include "Profiler.h"


void Profiler::Start(int count)
{
  mWorkTimer.start();
  mCount += count;
}

void Profiler::Pause()
{
  mNsecs += mWorkTimer.nsecsElapsed();
}

void Profiler::AutoDump(int periodMs)
{
  if (IsTimeToDump(periodMs)) {
    Log.Info(QString("%1 auto dump (%2)").arg(mName).arg(Dump()));
  }
}

QString Profiler::Dump()
{
  qint64 total = mLiveTimer.nsecsElapsed();
  qreal percent = 100.0 * mNsecs / total;
  qreal countPs = 1000000000.0 * (mCount - mLastCount) / (total - mLastNsecs);
  QString text = QString("percent: %1%, count: %2 p/s")
      .arg(percent, 0, 'f', 2).arg(countPs, 0, 'f', 2);

  mLastNsecs = total;
  mLastDump = mLiveTimer.elapsed();
  mLastCount = mCount;
  return text;
}


Profiler::Profiler(const char* _Name)
  : mName(_Name)
  , mCount(0), mNsecs(0)
  , mLastNsecs(0), mLastDump(0), mLastCount(0)
{
  mLiveTimer.start();
}
