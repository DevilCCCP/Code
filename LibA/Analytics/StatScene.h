#pragma once

#include <Lib/Log/Log.h>


class StatScene
{
  const char* mStatName;
  const int   mThresholdMin;

  int         mThreshold;
  int         mThresholdOk;
  int         mThresholdFails;
  int         mCountTotal;

public:
  int Threshold() const { return mThreshold; }

public:
  void InitTotalCount() { mCountTotal = 0; }
  void AddTotalCount(int count) { mCountTotal += count; }
  void UpdateThreshold(int totalCount)
  {
    int percent = 100 * mCountTotal / totalCount;
    if (percent > 30) {
      mThresholdFails++;
      if (mThresholdOk > 0) {
        mThresholdOk /= 2;
      }
    } else {
      if (mThresholdFails > 0) {
        mThresholdFails--;
      }
      mThresholdOk++;
    }

    if (mThresholdFails > 30) {
      mThresholdOk = 0;
      mThreshold++;
      Log.Warning(QString("%1 threshold too low for this image (threshold: %2)").arg(mStatName).arg(mThreshold));
    }
    if (mThreshold > mThresholdMin && mThresholdOk > 200) {
      mThreshold--;
      Log.Info(QString("%1 threshold decrease (threshold: %2)").arg(mStatName).arg(mThreshold));
    }
  }

public:
  StatScene(const char* _StatName, int _MinThreshold)
    : mStatName(_StatName), mThresholdMin(_MinThreshold)
    , mThreshold(_MinThreshold), mThresholdOk(0), mThresholdFails(0)
  { }
};

