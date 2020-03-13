#pragma once

#include <QList>
#include <QElapsedTimer>
#include <QString>
#include <QPair>

#include <Lib/Include/Common.h>


namespace FpsCalcPrivate {
const int kMinHistoryLenMs =  300;
const int kMaxHistoryLenMs = 3000;
}
// reentrant
class FpsCalc
{
  const int        mMaxHistoryLenMs;

  QElapsedTimer mTimer;
  qint64        mLastTime;
  QList<int>    mHistory;
  int           mHistoryLenMs;
  int           mHistoryNow;

public:
  void AddFrame(int periodMs);
  void AddFrame();
  void UpdateFrame();

  float GetFps();

private:
  void ClearLateHistory();

  float GetFpsFormula() { return 1000.0f * mHistory.size() / qMax(mHistoryLenMs, FpsCalcPrivate::kMinHistoryLenMs); }

public:
  QString Format();
  QString FormatShort();

  void Reset();

  FpsCalc(int _MaxHistoryLenMs = FpsCalcPrivate::kMaxHistoryLenMs);

private:
  FpsCalc(FpsCalc&other);
};

