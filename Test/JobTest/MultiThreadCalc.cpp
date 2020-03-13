#include "MultiThreadCalc.h"
#include "Calc.h"


MultiThreadCalc::MultiThreadCalc(QObject *parent) : QObject(parent)
{
}

void MultiThreadCalc::Calc(qint64 circles, int from, int to)
{
  ::Calc calc;
  for (int i = from; i < to; i++) {
    calc.CalcCircles(i, circles);
  }
}

