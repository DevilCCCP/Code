#include <QMutexLocker>

#include <Lib/Common/Format.h>

#include "WorkerStat.h"


void WorkerStat::UpdateWork(bool circle)
{
  qint64 nowMsecs = mWorkTimer.nsecsElapsed();
  qint64 lastNSecs = nowMsecs - mLastNsecs;
  if (mCurrentWork >= 0 && mCurrentWork < mWorkStatList.size()) {
    if (circle) {
      mWorkStatList[mCurrentWork].Circles++;
    }
    mWorkStatList[mCurrentWork].WorkNsecs += lastNSecs;
    mWorkStatList[mCurrentWork].LongestWorkNsecs = qMax(mWorkStatList[mCurrentWork].LongestWorkNsecs, lastNSecs);
  } else {
    mIdleNsecs += lastNSecs;
  }
  mLastNsecs = nowMsecs;
  mAggregated = 1;
}

void WorkerStat::ChangeWork(int index)
{
  UpdateWork(true);
  mCurrentWork = index;
}

void WorkerStat::ReturnWork(int index)
{
  UpdateWork(false);
  mCurrentWork = index;
}

void WorkerStat::CountWork(int index, int count)
{
  if (index >= 0 && index < mWorkStatList.size()) {
    mWorkStatList[index].Circles += count;
  }
}

void WorkerStat::InitStats(const QString& _Name)
{
  mName = _Name;
  mCurrentWork = 0;
  if (mWorkStatList.isEmpty()) {
    mWorkStatList.append(WorkStat());
  }

  for (auto itr = mWorkStatList.begin(); itr != mWorkStatList.end(); itr++) {
    WorkStat* workStat = &*itr;
    workStat->Circles = 0;
    workStat->WorkNsecs = 0;
    workStat->LongestWorkNsecs = 0;
  }
  mIdleNsecs = 0;
  mWorkTimer.start();
  mLastNsecs = 0;
  mAggregated = mClosed = 0;
}

void WorkerStat::Clear()
{
  for (auto itr = mWorkStatList.begin(); itr != mWorkStatList.end(); itr++) {
    WorkStat* workStat = &*itr;
    workStat->Circles = 0;
    workStat->WorkNsecs = 0;
    workStat->LongestWorkNsecs = 0;
  }
  mIdleNsecs = 0;
  mWorkTimer.start();
  mLastNsecs = 0;
  mAggregated = mClosed = 0;
}

void WorkerStat::AddWorkInfo(const QString& name, int index)
{
  Q_ASSERT(index < 100);

  if (index > 0) {
    mWorkStatList.resize(qMax(mWorkStatList.size(), index + 1));
    mWorkStatList[index] = WorkStat(name);
  } else {
    mWorkStatList.append(WorkStat(name));
  }
}

QString WorkerStat::DumpStats()
{
  QString stats;
  if (mAggregated > 1) {
    if (mClosed > 0) {
      stats = QString("%1(+%2-%3=%4): ").arg(mName).arg(mAggregated).arg(mClosed).arg(mAggregated - mClosed);
    } else {
      stats = QString("%1(%2): ").arg(mName).arg(mAggregated);
    }
  } else {
    stats = QString("%1: ").arg(mName);
  }

  qint64 totalNsecs = TotalNsecs();
  if (totalNsecs < 1000000000) {
    return stats;
  }
  if (mWorkStatList.size() == 1) {
    const WorkStat& workStat = mWorkStatList.first();
    qreal workPerc = 100.0 * workStat.WorkNsecs / totalNsecs;
    qreal workFreq = 1000000000.0 * workStat.Circles / totalNsecs;
    stats.append(QString("%1(%2%)").arg(workFreq, 1, 'f', 1).arg(workPerc, 2, 'f', 2));
    if (workStat.Circles > 0) {
      qreal workPeriod = workStat.WorkNsecs / workStat.Circles;
      if (workStat.LongestWorkNsecs > 4*workPeriod) {
        int longestMs = (int)(workStat.LongestWorkNsecs/1000000);
        if (longestMs > 500) {
          stats.append(QString("<%1>").arg(FormatTime(longestMs)));
        }
      }
    }
  } else {
    qint64 workNsecs = totalNsecs - mIdleNsecs;
    qreal workPerc = 100.0 * workNsecs / totalNsecs;
    stats.append(QString("%1% [").arg(workPerc, 2, 'f', 2));
    for (int i = 1; i < mWorkStatList.size(); i++) {
      const WorkStat& workStat = mWorkStatList.at(i);
      qreal workPerc = 100.0 * workStat.WorkNsecs / totalNsecs;
      qreal workFreq = 1000000000.0 * workStat.Circles / totalNsecs;
      stats.append(QString("%1: %2(%3%)").arg(workStat.Name).arg(workFreq, 1, 'f', 1).arg(workPerc, 2, 'f', 2));
      if (workStat.Circles > 0) {
        qreal workPeriod = workStat.WorkNsecs / workStat.Circles;
        if (workStat.LongestWorkNsecs > 4*workPeriod) {
          int longestMs = (int)(workStat.LongestWorkNsecs/1000000);
          if (longestMs > 500) {
            stats.append(QString("<%1>").arg(FormatTime(longestMs)));
          }
        }
      }
      stats.append(i < mWorkStatList.size()-1? "|": "]");
    }
  }

  return stats;
}

WorkerStatS WorkerStat::Clone() const
{
  WorkerStatS workerStat(new WorkerStat());
  CopyTo(*workerStat);
  return workerStat;
}

void WorkerStat::AggregateOne(const WorkerStat& workerStat)
{
  WorkerStat addWorkerStat;
  workerStat.CopyTo(addWorkerStat);

  QMutexLocker lock(&mMutex);
  if (mName.isEmpty()) {
    mName = addWorkerStat.mName;
  }
  mLastNsecs += addWorkerStat.mLastNsecs;
  mIdleNsecs += addWorkerStat.mIdleNsecs;
  mWorkStatList.resize(qMax(mWorkStatList.size(), addWorkerStat.mWorkStatList.size()));
  for (int i = 0; i < addWorkerStat.mWorkStatList.size(); i++) {
    WorkStat& workStat = mWorkStatList[i];
    const WorkStat& workStatAdd = workerStat.mWorkStatList.at(i);
    if (workStat.Name.isEmpty()) {
      workStat.Name = workStatAdd.Name;
    }
    workStat.Circles += workStatAdd.Circles;
    workStat.WorkNsecs += workStatAdd.WorkNsecs;
    workStat.LongestWorkNsecs = qMax(workStat.LongestWorkNsecs, workStatAdd.LongestWorkNsecs);
  }
  mAggregated += addWorkerStat.mAggregated;
  mClosed += addWorkerStat.mClosed;
}

void WorkerStat::AddCloseOne()
{
  mClosed++;
}

void WorkerStat::CopyTo(WorkerStat& workerStat) const
{
  QMutexLocker lock(&mMutex);
  workerStat.mName = mName;
  workerStat.mLastNsecs = mLastNsecs;
  workerStat.mIdleNsecs = mIdleNsecs;
  workerStat.mWorkStatList.resize(mWorkStatList.size());
  for (int i = 0; i < mWorkStatList.size(); i++) {
    const WorkStat& workStat = mWorkStatList.at(i);
    workerStat.mWorkStatList[i] = workStat;
  }
  workerStat.mAggregated = mAggregated;
  workerStat.mClosed = mClosed;
}


WorkerStat::WorkerStat()
  : mLastNsecs(0), mIdleNsecs(0), mCurrentWork(-2), mAggregated(0), mClosed(0)
{
  mWorkTimer.start();
}
