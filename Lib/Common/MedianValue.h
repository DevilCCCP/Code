#pragma once

#include <QVector>


class MedianValue
{
  const int        mWindowSize;

  QVector<qreal>   mHistoryData;
  int              mInsertIndex;
  QVector<qreal>   mSortedData;

public:
  void AddValue(qreal value);
  bool HasValue() const;
  qreal GetCurrentValue() const;

public:
  MedianValue(int _WindowSize);
};
