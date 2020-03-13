#pragma once

#include <QVector>
#include <QString>
#include <QStringList>


const int kHystFastLength = 16;
const int kHystFastShift = 4;

class HystFast
{
  int         mHyst[kHystFastLength];
  mutable int mTotalCount;

public:
  int GetLength() const { return kHystFastLength; }

  void Clear()
  {
    memset(mHyst, 0, sizeof(mHyst));
  }

  void Inc(int value)
  {
    mHyst[value>>kHystFastShift]++;
  }

  const int* Data() const
  {
    return mHyst;
  }

  int Size() const
  {
    return kHystFastLength;
  }

  int TotalCount() const
  {
    mTotalCount = 0;
    for (int i = 0; i < kHystFastLength; i++) {
      mTotalCount += mHyst[i];
    }

    return mTotalCount;
  }

  int LessCount(int value) const
  {
    value = qBound(0, value, kHystFastLength - 1);
    int index = value >> kHystFastShift;
    int extraPart = value - (index << kHystFastShift);
    int totalCount = 0;
    if (extraPart > 0) {
      totalCount += mHyst[index] * extraPart / (1 << kHystFastShift);
    }
    for (int i = 0; i < index; i++) {
      totalCount += mHyst[i];
    }

    return totalCount;
  }

  int GreaterCount(int value) const
  {
    value = qBound(0, value, kHystFastLength - 1);
    int index = value >> kHystFastShift;
    int extraPart = value - (index << kHystFastShift);
    int totalCount = 0;
    if (extraPart > 0) {
      extraPart = (1 << kHystFastShift) - extraPart;
      totalCount += mHyst[index] * extraPart / (1 << kHystFastShift);
      index++;
    }
    for (int i = index; i < kHystFastLength; i++) {
      totalCount += mHyst[i];
    }

    return totalCount;
  }

  void Add(const HystFast& other)
  {
    mTotalCount = 0;
    for (int i = 0; i < kHystFastLength; i++) {
      mHyst[i] += other.mHyst[i];
      mTotalCount += mHyst[i];
    }
  }

  int GetValue(int per)
  {
    if (!mTotalCount) {
      mTotalCount = TotalCount();
    }

    int cut = mTotalCount * (quint64)per / 1000LL;
    int count = 0;
    for (int i = 0; i < kHystFastLength; i++) {
      count += mHyst[i];
      if (count > cut) {
        return i << kHystFastShift;
      }
    }
    return 0;
  }

  int GetMidValue(int perFrom, int perTo)
  {
    int min = GetValue(perFrom);
    int max = GetValue(perTo);
    return (min + max)/2;
  }

  HystFast()
    : mTotalCount(0)
  {
    Clear();
  }
};
