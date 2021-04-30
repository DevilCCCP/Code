#include <algorithm>

#include "MedianValue.h"


void MedianValue::AddValue(qreal value)
{
  if (mHistoryData.size() < mWindowSize) {
    mHistoryData.append(value);
    mSortedData.append(value);
  } else {
    qreal deadValue = mHistoryData[mInsertIndex];
    mHistoryData[mInsertIndex] = value;
    auto itr = std::lower_bound(mSortedData.begin(), mSortedData.end(), deadValue);
    *itr = value;
    std::sort(mSortedData.begin(), mSortedData.end());

    mInsertIndex++;
    if (mInsertIndex >= mWindowSize) {
      mInsertIndex = 0;
    }
  }
}

bool MedianValue::HasValue() const
{
  return !mSortedData.isEmpty();
}

qreal MedianValue::GetCurrentValue() const
{
  int midIndex = mSortedData.size() / 2;
  if (mSortedData.size() % 2) {
    return mSortedData.at(midIndex);
  } else {
    return 0.5 * (mSortedData.at(midIndex - 1) + mSortedData.at(midIndex));
  }
}


MedianValue::MedianValue(int _WindowSize)
  : mWindowSize(_WindowSize)
  , mInsertIndex(0)
{
  mHistoryData.reserve(mWindowSize);
  mSortedData.reserve(mWindowSize);
}
