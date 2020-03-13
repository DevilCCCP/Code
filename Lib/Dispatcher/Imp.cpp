#include "Imp.h"
#include "Overseer.h"


Overseer *Imp::GetOverseer()
{
  return dynamic_cast<Overseer*>(GetManager());
}

Imp::Imp(int _WorkPeriodMs, bool _AutoWorkCalc, bool _Critical)
  : CtrlWorker(_WorkPeriodMs, _AutoWorkCalc, _Critical)
{
}
