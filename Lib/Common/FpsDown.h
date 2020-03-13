#pragma once

#include <QList>
#include <QElapsedTimer>

#include <Lib/Include/Common.h>


class FpsDown
{
  static const int kMaxHistoryLenMs = 1000;
  const int        mMaxHistoryLenMs;

  PROPERTY_GET_SET(int, DestFps)

  QElapsedTimer mTimer;
  QList<int>    mHistory;
  int           mHistoryLenMs;
  int           mPeriodEnd;

public:
  bool TakeFrame();
  bool TakeFrame(int periodMs);

private:
  void ClearLateHistory();

public:
  FpsDown(int _DestFps = 0, int _MaxHistoryLenMs = kMaxHistoryLenMs);

private:
  FpsDown(FpsDown& other);
};
