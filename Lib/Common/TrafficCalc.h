#pragma once

#include <QList>
#include <QElapsedTimer>
#include <QString>
#include <QPair>

#include <Lib/Include/Common.h>
#include <Lib/Common/Format.h>


// reentrant
class TrafficCalc
{
  static const int kMaxHistoryLenMs = 3000;
  const int        mMaxHistoryLenMs;

  QElapsedTimer           mTimer;
  qint64                  mLastTime;
  QList<QPair<int, int> > mHistory;
  int                     mHistoryValue;
  int                     mHistoryLenMs;
  int                     mHistoryNow;

public:
  void AddFrame(int periodMs, int size);
  void AddFrame(int size);
  void UpdateFrame();

  float GetTraffic();

private:
  void ClearLateHistory();

  float GetTrafficFormula() { return mHistoryValue * 1000.0f / (mHistoryLenMs + mHistoryNow); }

public:
  QString Format();
  QString FormatRu();
  void Reset();

  TrafficCalc(int _MaxHistoryLenMs = kMaxHistoryLenMs);

private:
  TrafficCalc(TrafficCalc&);
};
