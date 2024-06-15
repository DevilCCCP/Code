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

public:
  void Clear();
  void Inc(uchar value);
  void Inc(int value);
  const int* Data() const;
  int Size() const;
  int TotalCount() const;
  int LessCount(int value) const;
  int GreaterCount(int value) const;
  void Add(const HystFast& other);
  int GetValue(int per);
  int GetMidValue(int perFrom, int perTo);

public:
  HystFast();
};
