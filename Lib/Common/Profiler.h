#pragma once

#include <QElapsedTimer>
#include <QString>


class Profiler
{
  const char*   mName;

  int           mCount;
  qint64        mNsecs;
  QElapsedTimer mWorkTimer;
  QElapsedTimer mLiveTimer;

  qint64        mLastNsecs;
  qint64        mLastDump;
  int           mLastCount;

public:
  void Start(int count = 1);
  void Pause();

  void AutoDump(int periodMs = 2000);
  QString Dump();

  bool IsTimeToDump(int periodMs) { return mLiveTimer.elapsed() - mLastDump > periodMs; }

public:
  explicit Profiler(const char* _Name = "Unonimous profiler");
};
