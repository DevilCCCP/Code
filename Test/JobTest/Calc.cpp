#include "Calc.h"


void Calc::CalcCircles(qint64 seed, qint64 circles)
{
  mResult = seed * 0.01;

  mTimer.start();
  for (mCircles = 0; mCircles < circles; mCircles++) {
    CalcOne();
  }
  mWorkMs = mTimer.elapsed();
}

void Calc::CalcTime(qint64 seed, qint64 limitMs)
{
  mResult = seed * 0.01;

  mTimer.start();
  for (mCircles = 0; ; ) {
    for (int i = 0; i < 100; i++) {
      CalcOne();
    }
    mCircles += 100;
    mWorkMs = mTimer.elapsed();
    if (mWorkMs > limitMs) {
      break;
    }
  }
}

void Calc::CalcOne()
{
  for (int i = 0; i < 1000; i++) {
    mResult = sin(mResult) + cos(mResult);
  }
}


Calc::Calc()
{

}

