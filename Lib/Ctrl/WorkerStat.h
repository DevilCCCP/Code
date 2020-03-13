#pragma once

#include <QVector>
#include <QString>
#include <QElapsedTimer>
#include <QMutex>

#include <Lib/Include/Common.h>


struct WorkStat {
  QString Name;
  qint64  Circles;
  qint64  WorkNsecs;
  qint64  LongestWorkNsecs;

  WorkStat(const QString& _Name): Name(_Name), Circles(0), WorkNsecs(0), LongestWorkNsecs(0) { }
  WorkStat(): Circles(0), WorkNsecs(0), LongestWorkNsecs(0) { }
};

DefineClassS(WorkerStat);

class WorkerStat
{
  QString           mName;

  mutable QMutex    mMutex;
  QElapsedTimer     mWorkTimer;
  qint64            mLastNsecs;
  QVector<WorkStat> mWorkStatList;
  qint64            mIdleNsecs;
  int               mCurrentWork;
  int               mAggregated;
  int               mClosed;

public:
  const QString& Name() { return mName; }
  qint64 TotalNsecs() { return mLastNsecs; }
  const QVector<WorkStat>& WorkStatList() { return mWorkStatList; }
  int TotalTimeMs() { return (int)(mLastNsecs / 1000000); }
  int ElapsedTimeMs() { return  mWorkTimer.elapsed(); }

public:
  void UpdateWork(bool circle);
  void ChangeWork(int index);
  void ReturnWork(int index);
  void CountWork(int index, int count = 1);

public:
  void InitStats(const QString& _Name);
  void Clear();
  void AddWorkInfo(const QString& name, int index = -1);
  QString DumpStats();
  WorkerStatS Clone() const;
  void AggregateOne(const WorkerStat& workerStat);
  void AddCloseOne();

private:
  void CopyTo(WorkerStat& workerStat) const;

public:
  WorkerStat();
};
