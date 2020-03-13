#include "TrafficCalc.h"


void TrafficCalc::AddFrame(int periodMs, int size)
{
  mHistory.append(qMakePair(periodMs, size));
  mHistoryLenMs += periodMs;
  mHistoryValue += size;
  mHistoryNow = 0;

  ClearLateHistory();
}

void TrafficCalc::AddFrame(int size)
{
  if (mHistory.isEmpty()) {
    mTimer.start();
    mLastTime = 0;
    mHistory.append(qMakePair(0, size));
  } else {
    qint64 now = mTimer.elapsed();
    AddFrame((int)(now - mLastTime), size);
    mLastTime = now;
  }
}

void TrafficCalc::UpdateFrame()
{
  if (mHistory.isEmpty()) {
    return;
  }

  mHistoryNow = (int)(mTimer.elapsed() - mLastTime);

  ClearLateHistory();
}

float TrafficCalc::GetTraffic()
{
  if (mHistoryLenMs > 0) {
    return GetTrafficFormula();
  } else {
    return 0;
  }
}

void TrafficCalc::ClearLateHistory()
{
  while (mHistoryLenMs + mHistoryNow > mMaxHistoryLenMs && mHistory.size() > 0) {
    const QPair<int, int>& p = mHistory.first();
    mHistoryLenMs -= p.first;
    mHistoryValue -= p.second;
    mHistory.removeFirst();
  }
}

QString TrafficCalc::Format()
{
  if (mHistoryLenMs > 0) {
    return FormatBytes(GetTrafficFormula()) + "/s";
  } else {
    return "n/a B/s";
  }
}

QString TrafficCalc::FormatRu()
{
  if (mHistoryLenMs > 0) {
    return FormatBytesRu(GetTrafficFormula()) + "/с";
  } else {
    return "н/д Б/с";
  }
}

void TrafficCalc::Reset()
{
  mHistory.clear();
  mHistoryLenMs = 0;
  mHistoryValue = 0;
}


TrafficCalc::TrafficCalc(int _MaxHistoryLenMs)
  : mMaxHistoryLenMs(_MaxHistoryLenMs)
  , mHistoryValue(0), mHistoryLenMs(0)
{
}

TrafficCalc::TrafficCalc(TrafficCalc&)
  : mMaxHistoryLenMs(0)
{ }
