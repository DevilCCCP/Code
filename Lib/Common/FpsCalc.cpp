#include "FpsCalc.h"


void FpsCalc::AddFrame(int periodMs)
{
  mHistory.append(periodMs);
  mHistoryLenMs += periodMs;
  mHistoryNow = 0;

  ClearLateHistory();
}

void FpsCalc::AddFrame()
{
  if (mHistory.isEmpty()) {
    mTimer.start();
    mLastTime = 0;
    mHistory.append(0);
  } else {
    qint64 now = mTimer.elapsed();
    AddFrame((int)(now - mLastTime));
    mLastTime = now;
  }
}

void FpsCalc::UpdateFrame()
{
  if (mHistory.isEmpty()) {
    return;
  }

  mHistoryNow = (int)(mTimer.elapsed() - mLastTime);

  ClearLateHistory();
}

float FpsCalc::GetFps()
{
  if (mHistoryLenMs > 0) {
    return GetFpsFormula();
  } else {
    return 0;
  }
}

void FpsCalc::ClearLateHistory()
{
  while (mHistoryLenMs + mHistoryNow > mMaxHistoryLenMs && mHistory.size() > 0) {
    mHistoryLenMs -= mHistory.takeFirst();
  }
}

QString FpsCalc::Format()
{
  if (mHistoryLenMs > 0) {
    return QString::number(GetFpsFormula(), 'f', 2);
  } else {
    return "--.--";
  }
}

QString FpsCalc::FormatShort()
{
  float fps = GetFps();
  if (fps >= 100.0f) {
    if (fps >= 1000.0f) {
      return QString::number((int)fps);
    } else {
      return QString::number((int)fps) + '.';
    }
  } else if (fps >= 10.0f) {
    return QString::number(fps, 'f', 1);
  } else if (fps > 0.0f) {
    return QString::number(fps, 'f', 2);
  } else {
    return "-.--";
  }
}

void FpsCalc::Reset()
{
  mHistory.clear();
  mHistoryLenMs = 0;
  mLastTime = 0;
}

FpsCalc::FpsCalc(int _MaxHistoryLenMs)
  : mMaxHistoryLenMs(_MaxHistoryLenMs), mHistoryLenMs(0)
{
}

FpsCalc::FpsCalc(FpsCalc& other)
  : mMaxHistoryLenMs(other.mMaxHistoryLenMs), mHistoryLenMs(0)
{
}
