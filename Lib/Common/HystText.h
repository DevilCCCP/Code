#pragma once

#include <QVector>
#include <QByteArray>


class HystText
{
  QVector<int> mHyst;
  int          mTotal;

public:
  void AddOneByte(const QByteArray& text);
  void AddOneByteCount(const QByteArray& text, int count);
  void AddTwoByte(const QByteArray& text);
  void AddTwoByteCount(const QByteArray& text, int count);
  qreal CalcFitHyst(const HystText& dict) const;
  qreal CalcTwoByteFitRange(const QByteArray& text) const;

  bool Save(QDataStream& stream) const;
  bool Load(QDataStream& stream);
  QString Dump();

private:
  void AddChar(int ch);
  void AddChar(int ch, int count);

public:
  HystText(int _ByteCount = 1);
};
