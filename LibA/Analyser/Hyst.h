#pragma once

#include <QVector>
#include <QString>
#include <QStringList>

#include "HystFast.h"


const int kHystDefaultLength = 256;

class Hyst
{
  int          mHystLength;

  QVector<int> mHyst;
  mutable int  mTotalCount;

public:
  int Length() const { return mHystLength; }
  int GetHyst(int index) const { return mHyst[index]; }
  const QVector<int>& GetVector() const { return mHyst; }
  const int* Data() const { return mHyst.constData(); }
  int* Data() { return mHyst.data(); }
  int Size() const { return mHyst.size(); }

public:
  void Normalize(int maxCount);
  QString Serialize(int precision = 32) const;
  bool Deserialize(const QString& text, int precision = 32);
  void Clear();

  void Inc(int ind);
  void SetLine(int ind);
  void SetTotalCount(int _TotalCount);
  int TotalCount() const;
  int LessCount(int value) const;
  int GreaterCount(int value) const;
  void Add(const Hyst& other);
  int GetValue(int per, int totalCount = 0) const;
  int GetMidValue(int perFrom, int perTo, int totalCount = 0) const;
  int GetMaxRight();
  int GetValueB(int per, int totalCount = 0) const;
  int GetLocalMax(int from, int to) const;
  int GetLocalDisp(int maxValue, int direction = 0) const;
  int GetHystMass(int maxValue, int length) const;
  void MedianLow();

  void SetHyst(const Hyst& other);
  void SetHystFast(const HystFast& other);

public:
  Hyst(const int _HystLength = kHystDefaultLength);
};
