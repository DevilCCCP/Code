#include "Hyst.h"


void Hyst::Normalize(int maxCount)
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

QString Hyst::Serialize(int precision) const
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

bool Hyst::Deserialize(const QString& text, int precision)
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

void Hyst::Clear()
{
  mHyst.fill(0);
  mTotalCount = 0;
}

void Hyst::Inc(int ind)
{
  mHyst[ind]++;
  mTotalCount++;
}

void Hyst::SetLine(int ind)
{
  mTotalCount -= mHyst[ind];
  mHyst[ind] = mTotalCount;
  mTotalCount += mTotalCount;
}

void Hyst::SetTotalCount(int _TotalCount)
{
  mTotalCount = _TotalCount;
}

int Hyst::TotalCount() const
{
  if (!mTotalCount) {
    for (int i = 0; i < mHystLength; i++) {
      mTotalCount += mHyst[i];
    }
  }

  return mTotalCount;
}

int Hyst::LessCount(int value) const
{
  value = qBound(0, value, mHystLength - 1);
  int totalCount = 0;
  for (int i = 0; i < value; i++) {
    totalCount += mHyst[i];
  }

  return totalCount;
}

int Hyst::GreaterCount(int value) const
{
  value = qBound(0, value, mHystLength - 1);
  int totalCount = 0;
  for (int i = value + 1; i < mHystLength; i++) {
    totalCount += mHyst[i];
  }

  return totalCount;
}

void Hyst::Add(const Hyst& other)
{
  mHystLength = qMax(mHystLength, other.mHystLength);
  mHyst.resize(mHystLength);
  mTotalCount = 0;
  for (int i = 0; i < other.mHystLength; i++) {
    mHyst[i] += other.mHyst[i];
    mTotalCount += mHyst[i];
  }
  for (int i = other.mHystLength; i < mHystLength; i++) {
    mTotalCount += mHyst[i];
  }
}

int Hyst::GetValue(int per, int totalCount) const
{
  if (!totalCount) {
    totalCount = TotalCount();
  }

  int cut = (qint64)totalCount * (qint64)per / 1000LL;
  int count = 0;
  for (int i = 0; i < mHystLength; i++) {
    count += mHyst[i];
    if (count > cut) {
      return i;
    }
  }
  return 0;
}

int Hyst::GetMidValue(int perFrom, int perTo, int totalCount) const
{
  int min = GetValue(perFrom, totalCount);
  int max = GetValue(perTo, totalCount);
  return (min + max)/2;
}

int Hyst::GetMaxRight()
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

int Hyst::GetValueB(int per, int totalCount) const
{
  if (!totalCount) {
    totalCount = TotalCount();
  }

  int cut = (qint64)totalCount * (qint64)per / 1000LL;
  int count = 0;
  for (int i = mHystLength - 1; i >= 0; i--) {
    count += mHyst[i];
    if (count > cut) {
      return i;
    }
  }
  return 0;
}

int Hyst::GetLocalMax(int from, int to) const
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

int Hyst::GetLocalDisp(int maxValue, int direction) const
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

int Hyst::GetHystMass(int maxValue, int length) const
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
  return (qint64)mass * 255LL / mTotalCount;
}

void Hyst::MedianLow()
{
  mTotalCount = mHyst[0] + mHyst[mHystLength - 1];
  for (int i = 1; i < mHystLength - 1; i++) {
    if (mHyst[i] < mHyst[i-1] && mHyst[i] < mHyst[i+1]) {
      mHyst[i] = qMin(mHyst[i-1], mHyst[i+1]);
    }
    mTotalCount += mHyst[i];
  }
}

void Hyst::SetHyst(const Hyst& other)
{
  mHystLength = other.mHystLength;
  mHyst       = other.mHyst;
  mTotalCount = other.mTotalCount;
}

void Hyst::SetHystFast(const HystFast& other)
{
  mHystLength = other.GetLength();
  mHyst.resize(mHystLength);
  memcpy(mHyst.data(), other.Data(), sizeof(int) * mHystLength);
  mTotalCount = other.TotalCount();
}

Hyst::Hyst(const int _HystLength)
  : mHystLength(_HystLength)
  , mHyst(_HystLength), mTotalCount(0)
{
  Clear();
}
