#pragma once

#include <QObject>


class FormatTr: QObject
{
  static FormatTr* mSelf;

  Q_OBJECT

public:
  static FormatTr* Instance() { return mSelf; }

  static QString FormatTimeDeltaTr(qint64 ms, int prec = 2);
  static QString FormatTimeTr(qint64 ms, int prec = 2);

public:
  FormatTr(QObject* parent = 0);
};
