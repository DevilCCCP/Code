#include <Lib/Log/Log.h>

#include "FpsDown.h"


bool FpsDown::TakeFrame()
{
  if (mDestFps == 0) {
    return true;
  }

  if (!mTimer.isValid()) {
    mTimer.start();
    return true;
  } else {
    return TakeFrame((int)mTimer.restart());
  }
}

bool FpsDown::TakeFrame(int periodMs)
{
  if (mDestFps == 0) {
    return true;
  }

  mHistoryLenMs += periodMs;
  ClearLateHistory();

  if (mHistory.isEmpty()) {
    mPeriodEnd = 0;
    mHistory.append(mHistoryLenMs);
    return true;
  }

  int limitHistory = (1000 * (mHistory.size()) + 200) / mDestFps;
  if (mHistoryLenMs > limitHistory) {
    mHistory.append(mPeriodEnd + periodMs);
    mPeriodEnd = 0;
    return true;
  }

  mPeriodEnd += periodMs;
  return false;
}

void FpsDown::ClearLateHistory()
{
  if (mHistoryLenMs <= mMaxHistoryLenMs) {
    return;
  }

  while (mHistory.size() > 0) {
    int period1 = mHistory.first();
    if (mHistoryLenMs - period1 >= mMaxHistoryLenMs) {
      mHistoryLenMs -= period1;
      mHistory.removeFirst();
    } else {
      period1 -= mHistoryLenMs - mMaxHistoryLenMs;
      mHistory.first() = period1;
      break;
    }
  }
  mHistoryLenMs = mMaxHistoryLenMs;
}


FpsDown::FpsDown(int _DestFps, int _MaxHistoryLenMs)
  : mMaxHistoryLenMs(_MaxHistoryLenMs), mDestFps(_DestFps)
  , mHistoryLenMs(0), mPeriodEnd(0)
{
  mTimer.invalidate();
}

FpsDown::FpsDown(FpsDown& other)
  : mMaxHistoryLenMs(0), mDestFps(other.mDestFps)
  , mHistoryLenMs(0), mPeriodEnd(0)
{
  mTimer.invalidate();
}
