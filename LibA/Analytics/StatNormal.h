#pragma once
#include <memory>
#include <QtGlobal>

class StatNormal
{
  static const int kCount = 16;

  int mValues[kCount];
  int mTotalCount;

public:
  int TotalCount() const { return mTotalCount; }

  int HighValue() const { return kCount; }

protected:
  void StatShiftLeft()
  {
    //mTotalCount = 0;
    for (int i = 0; i < kCount/2; i++) {
      mValues[i] = mValues[2*i] + mValues[2*i + 1];
      //mTotalCount += mValues[i];
    }
    for (int i = kCount/2; i < kCount; i++) {
      mValues[i] = 0;
    }
  }

public:
  void Recalc()
  {
    mTotalCount = 0;
    for (int i = 0; i < kCount; i++) {
      mTotalCount += mValues[i];
    }
  }

  int Count(int value, const int maxValue) const
  {
    int ind = value * (kCount - 1) / maxValue;
    return mValues[ind];
  }

  int MidValue(const int maxValue) const
  {
    int count = 0;
    int midCount = mTotalCount / 2;
    for (int i = 0; i < kCount; i++) {
      count += mValues[i];
      if (count >= midCount) {
        return (i * maxValue / (kCount - 1));
      }
    }
    return 0;
  }

  int MaxValue(const int maxValue) const
  {
    int count = 0;
    int mid2Count = mTotalCount * 9 / 10;
    for (int i = 0; i < kCount; i++) {
      count += mValues[i];
      if (count >= mid2Count) {
        return (i * maxValue / (kCount - 1));
      }
    }
    return 0;
  }

  int Normalized(int value, const int maxValue) const
  {
    int ind = value * (kCount - 1) / maxValue;
    if (mTotalCount < 20) {
       ind = qMax(ind - 8, 0);
    } else { //if (mTotalCount < 200) {
       ind = qMax(ind - 4, 0);
    } /*else {
      ind = qMax(ind - 2, 0);
    }*/

    int count = 0;
    int midCount = mTotalCount / 2;
    int mid2Count = mTotalCount * 9 / 10;
    for (int i = 0; i < ind; i++) {
      count += mValues[i];
    }
    if (count <= midCount) {
      return 0;
    } else if (count <= mid2Count) {
      return 1;
    } else {
      return 2;
    }
  }

  void Update(int value, const int maxValue)
  {
    int ind = value * (kCount - 1) / maxValue;
    mValues[ind]++;
    mTotalCount++;
  }
};

class StatNormalAuto: public StatNormal
{
  int mMaxValue;

public:
  void Reset()
  {
    memset(this, 0, sizeof(StatNormalAuto));
  }

  int Normalized(int value) const
  {
    if (value < mMaxValue) {
      return StatNormal::Normalized(value, mMaxValue);
    } else {
      return 2;
    }
  }

  void Update(int value)
  {
    if (value >= mMaxValue) {
      if (mMaxValue == 0) {
        mMaxValue = value + 1;
      } else if (MaxValue(mMaxValue) == mMaxValue) {
        mMaxValue = qMax(2*mMaxValue, value/8);        
        StatNormal::StatShiftLeft();
      }
      value = qMin(value, mMaxValue);
    }
    StatNormal::Update(value, mMaxValue);
  }
};
