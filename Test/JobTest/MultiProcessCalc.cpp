#include "MultiProcessCalc.h"


bool MultiProcessCalc::DoJob()
{
  QString params = QString::fromLatin1(CurrentJobData());
  int circles = params.toInt();
  for (int i = CurrentJobIterFrom(); i <= CurrentJobIterTo(); i++) {
    mCalc.CalcCircles(i, circles);
  }
  return true;
}

MultiProcessCalc::MultiProcessCalc(const Db& _Db)
  : JobImp(_Db)
{
}

