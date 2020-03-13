#pragma once

#include <QElapsedTimer>


class Calc
{
  qreal         mResult;

  QElapsedTimer mTimer;
  qint64        mCircles;
  qint64        mWorkMs;

public:
  void CalcCircles(qint64 seed, qint64 circles);
  void CalcTime(qint64 seed, qint64 limitMs);

  qint64 WorkMs() { return mWorkMs; }
  qint64 WorkCircles() { return mCircles; }

private:
  void CalcOne();

public:
  Calc();
};
