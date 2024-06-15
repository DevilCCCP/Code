#include "HystFast.h"


void HystFast::Clear()
{
  memset(mHyst, 0, sizeof(mHyst));
}

void HystFast::Inc(uchar value)
{
  mHyst[value >> kHystFastShift]++;
}

void HystFast::Inc(int value)
{
  mHyst[value >> kHystFastShift]++;
}

const int* HystFast::Data() const
{
  return mHyst;
}

int HystFast::Size() const
{
  return kHystFastLength;
}

int HystFast::TotalCount() const
{
  mTotalCount = 0;
  for (int i = 0; i < kHystFastLength; i++) {
    mTotalCount += mHyst[i];
  }

  return mTotalCount;
}

int HystFast::LessCount(int value) const
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

int HystFast::GreaterCount(int value) const
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

void HystFast::Add(const HystFast& other)
{
  mTotalCount = 0;
  for (int i = 0; i < kHystFastLength; i++) {
    mHyst[i] += other.mHyst[i];
    mTotalCount += mHyst[i];
  }
}

int HystFast::GetValue(int per)
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

int HystFast::GetMidValue(int perFrom, int perTo)
{
  int min = GetValue(perFrom);
  int max = GetValue(perTo);
  return (min + max)/2;
}

HystFast::HystFast()
  : mTotalCount(0)
{
  Clear();
}
