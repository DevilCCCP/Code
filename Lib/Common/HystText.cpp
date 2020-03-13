#include <QDataStream>

#include "HystText.h"


void HystText::AddOneByte(const QByteArray& text)
{
  foreach (char ch, text) {
    mHyst[(int)(uchar)ch]++;
  }
  mTotal += text.size();
}

void HystText::AddOneByteCount(const QByteArray& text, int count)
{
  foreach (char ch, text) {
    mHyst[(int)(uchar)ch] += count;
  }
  mTotal += text.size() * count;
}

void HystText::AddTwoByte(const QByteArray& text)
{
  int size = text.size()/2;
  if (size <= 0) {
    return;
  }
  const quint16* textBegin = (const quint16*)text.constData();
  const quint16* textEnd = (const quint16*)text.constData() + size;
  for (const quint16* textPtr = textBegin; textPtr < textEnd; textPtr++) {
    int ch = (int)*textPtr;
    mHyst[ch]++;
  }
  mTotal += size;
}

void HystText::AddTwoByteCount(const QByteArray& text, int count)
{
  int size = text.size()/2;
  if (size <= 0) {
    return;
  }
  const quint16* textBegin = (const quint16*)text.constData();
  const quint16* textEnd = (const quint16*)text.constData() + size;
  for (const quint16* textPtr = textBegin; textPtr < textEnd; textPtr++) {
    int ch = (int)*textPtr;
    mHyst[ch] += count;
  }
  mTotal += size * count;
}

qreal HystText::CalcFitHyst(const HystText& dict) const
{
  int testSize = qMin(mHyst.size(), dict.mHyst.size());
  qreal totalDiff = (qreal)dict.mTotal;
  for (int i = 0; i < testSize; i++) {
    if (dict.mHyst.at(i)) {
      totalDiff -= qAbs((qreal)mHyst.at(i) * (qreal)dict.mTotal / (qreal)mTotal - (qreal)dict.mHyst.at(i));
    }
  }
  return totalDiff / (qreal)dict.mTotal;
}

qreal HystText::CalcTwoByteFitRange(const QByteArray& text) const
{
  int size = text.size()/2;
  if (size <= 0) {
    return 0;
  }
  int totalHit = 0;
  const quint16* textBegin = (const quint16*)text.constData();
  const quint16* textEnd = (const quint16*)text.constData() + size;
  for (const quint16* textPtr = textBegin; textPtr < textEnd; textPtr++) {
    int ch = (int)*textPtr;
    if (mHyst.at(ch)) {
      totalHit++;
    }
  }
  return (qreal)totalHit / size;
}

bool HystText::Save(QDataStream& stream) const
{
  stream << mHyst.size();
  for (int i = 0; i < mHyst.size(); i++) {
    if (mHyst.at(i)) {
      stream << i;
      stream << mHyst.at(i);
    }
  }
  stream << -1;
  return true;
}

bool HystText::Load(QDataStream& stream)
{
  int size;
  stream >> size;
  mHyst.fill(0, size);
  mTotal = 0;
  forever {
    int i;
    stream >> i;
    if (i < 0) {
      break;
    }
    stream >> mHyst[i];
    mTotal += mHyst[i];
  }
  return true;
}

QString HystText::Dump()
{
  QString dump;
  for (int i = 0; i < mHyst.size(); i++) {
    if (mHyst[i]) {
      dump.append(QString("[%1]: %2 ('%3')\n").arg(i, 2, 16, QChar('0')).arg(100.0 * mHyst[i] / mTotal, 0, 'f', 2).arg((char)i));
    }
  }
  return dump;
}

void HystText::AddChar(int ch)
{
  if (ch < 0x7f) {
    return;
  }
  mHyst[ch]++;
  mTotal++;
}

void HystText::AddChar(int ch, int count)
{
  if (ch < 0x7f) {
    return;
  }
  mHyst[ch] += count;
  mTotal += count;
}


HystText::HystText(int _ByteCount)
  : mHyst(0x1 << (8*_ByteCount), (int)0), mTotal(0)
{
}

