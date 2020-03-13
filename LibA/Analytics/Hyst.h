#pragma once

#include <QVector>
#include <QString>
#include <QStringList>


const int kHystDefaultLength = 256;
const int kHystFastLength = 16;
const int kHystFastShift = 4;

class Hyst
{
  const int    mHystLength;

  QVector<int> mHyst;
  mutable int  mTotalCount;

public:
  int Length() const { return mHystLength; }
  int GetHyst(int index) const { return mHyst[index]; }
  const QVector<int>& GetVector() const { return mHyst; }
  const int* Data() const { return mHyst.constData(); }
  int* Data() { return mHyst.data(); }
  int Size() const { return mHyst.size(); }

  void Normalize(int maxCount)
  {
    if (mTotalCount < maxCount || maxCount <= 0) {
      return;
    }

    int divider = 2;
    for (mTotalCount /= 2; divider < 1024; mTotalCount /= 2, divider *= 2) {
      if (mTotalCount < maxCount) {
        break;
      }
    }

    mTotalCount = 0;
    for (int i = 0; i < mHystLength; i++) {
      mHyst[i] /= divider;
      mTotalCount += mHyst[i];
    }
  }

  QString Serialize(int precision = 32) const
  {
    if (precision <= 0 || TotalCount() <= 0) {
      return "";
    } else if (precision > mHystLength) {
      return "not implemented";
    }
    QVector<double> hyst(precision);
    double totalCount = 0;
    for (int i = 0; i < precision; i++) {
      int from = (int)i * mHystLength / precision;
      int to = (int)(i + 1) * mHystLength / precision;
      hyst[i] = 0;
      for (int ii = from; ii < to; ii++) {
        hyst[i] += mHyst[ii];
      }
      totalCount += hyst[i];
    }

    QStringList resultList;
    int zero = 0;
    for (int i = 0; i < precision; i++) {
      double perc = hyst[i] * 100.0 / totalCount;
      if (perc < 0.01) {
        zero++;
      } else {
        zero = 0;
      }
      resultList.append(QString::number(perc, 'f', 2));
    }
    while (zero > 0) {
      resultList.removeLast();
      zero--;
    }
    return QString::number(mTotalCount) + "|" + resultList.join(';');
  }

  bool Deserialize(const QString& text, int precision = 32)
  {
    int p1 = text.indexOf('|');
    if (p1 < 0) {
      return false;
    }
    bool ok;
    mTotalCount = text.left(p1).toInt(&ok);
    if (!ok) {
      return false;
    }
    QStringList resultList = text.mid(p1 + 1).split(QChar(';'));
    mHyst.fill(0);
    QVector<double> hyst(mHystLength, (double)0);
    double totalPerc = 0;
    for (int i = 0; i < mHystLength; i++) {
      int ind = i * precision / mHystLength;
      if (ind >= resultList.size()) {
        break;
      }
      const QString& percText = resultList.at(ind);
      double value = percText.toDouble(&ok);
      if (!ok) {
        break;
      }
      hyst[i] = value;
      totalPerc += value;
    }

    int totalCount = mTotalCount;
    mTotalCount = 0;
    for (int i = 0; i < mHystLength; i++) {
      mHyst[i] = (int)(hyst[i] / totalPerc * totalCount);
      mTotalCount += mHyst[i];
    }
    return true;
  }

  void Clear()
  {
    mHyst.fill(0);
    mTotalCount = 0;
  }

  void Inc(int ind)
  {
    mHyst[ind]++;
    mTotalCount++;
  }

  void SetLine(int ind)
  {
    mTotalCount -= mHyst[ind];
    mHyst[ind] = mTotalCount;
    mTotalCount += mTotalCount;
  }

  void SetTotalCount(int _TotalCount)
  {
    mTotalCount = _TotalCount;
  }

  int TotalCount() const
  {
    if (!mTotalCount) {
      for (int i = 0; i < mHystLength; i++) {
        mTotalCount += mHyst[i];
      }
    }

    return mTotalCount;
  }

  int GetValue(int per, int totalCount = 0) const
  {
    if (!totalCount) {
      totalCount = TotalCount();
    }

    int cut = totalCount * (quint64)per / 1000LL;
    int count = 0;
    for (int i = 0; i < mHystLength; i++) {
      count += mHyst[i];
      if (count > cut) {
        return i;
      }
    }
    return 0;
  }

  int GetMidValue(int perFrom, int perTo, int totalCount = 0) const
  {
    int min = GetValue(perFrom, totalCount);
    int max = GetValue(perTo, totalCount);
    return (min + max)/2;
  }

  int GetMaxRight()
  {
    mTotalCount = 0;
    int realCount = 0;
    for (int i = 0; i < mHystLength; i++) {
      if (mHyst[i]) {
        mTotalCount += mHyst[i];
        realCount++;
      }
    }
    int midValue = mTotalCount / realCount;
    if (midValue > 0) {
      int mid1 = 2*midValue - 1;
      int mid2 = 4*midValue - 1;
      int count1 = 0;
      int count2 = 0;
      for (int i = 0; i < mHystLength; i++) {
        if (mHyst[i] > midValue) {
          count1++;
          if (mHyst[i] > mid2) {
            count2++;
          }
        }
      }
      if (count2 > realCount / 10 + 2) {
        midValue = mid2;
      } else if (count1 > realCount / 10 + 2) {
        midValue = mid1;
      }
    }

    int maxPos = mHystLength - 1;
    int maxValue = midValue;
    int maxConfirm = -1;
    int confirmLength = mHystLength / 100 + 1;
    for (int i = mHystLength - 1; i >= 0; i--) {
      if (mHyst[i] > maxValue) {
        if (maxConfirm < confirmLength || mHyst[i] > 8 * maxValue) {
          maxPos = i;
          maxValue = mHyst[maxPos];
          maxConfirm = 0;
        }
      } else if (maxConfirm >= 0) {
        maxConfirm++;
      }
    }
    return maxPos;
  }

  int GetValueB(int per, int totalCount = 0) const
  {
    if (!totalCount) {
      totalCount = TotalCount();
    }

    int cut = totalCount * (quint64)per / 1000LL;
    int count = 0;
    for (int i = mHystLength - 1; i >= 0; i--) {
      count += mHyst[i];
      if (count > cut) {
        return i;
      }
    }
    return 0;
  }

  int GetLocalMax(int from, int to) const
  {
    int maxValue = 0;
    int maxIndex = from;
    for (int i = from; i < to; i++) {
      if (mHyst[i] > maxValue) {
        maxIndex = i;
        maxValue = mHyst[i];
      }
    }

    return maxIndex;
  }

  int GetLocalDisp(int maxValue, int direction = 0) const
  {
    int max = mHyst[maxValue];
    for (int i = 1; i < 256; i++) {
      if (direction <= 0) {
        int indl = maxValue - i;
        if (indl >= 0 && mHyst[indl] > max/4) {
          continue;
        }
      }
      if (direction >= 0) {
        int indr = maxValue + i;
        if (indr < 256 && mHyst[indr] > max/4) {
          continue;
        }
      }
      return i;
    }
    return 255;
  }

  int GetHystMass(int maxValue, int length) const
  {
    int mass = mHyst[maxValue];
    for (int i = 1; i < length; i++) {
      int indl = maxValue - i;
      int indr = maxValue + i;

      if (indl >= 0) {
        mass += mHyst[indl];
      } if (indr < 256) {
        mass += mHyst[indr];
      }
    }
    return mass * 255LL / mTotalCount;
  }

  void MedianLow()
  {
    mTotalCount = mHyst[0] + mHyst[mHystLength - 1];
    for (int i = 1; i < mHystLength - 1; i++) {
      if (mHyst[i] < mHyst[i-1] && mHyst[i] < mHyst[i+1]) {
        mHyst[i] = qMin(mHyst[i-1], mHyst[i+1]);
      }
      mTotalCount += mHyst[i];
    }
  }

  Hyst(const int _HystLength = kHystDefaultLength)
    : mHystLength(_HystLength)
    , mHyst(_HystLength), mTotalCount(0)
  {
    Clear();
  }
};

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
