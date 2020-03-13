#pragma once

#include <Lib/Ctrl/CtrlWorker.h>


class Overseer;

class Imp: public CtrlWorker
{
protected:
  Overseer* GetOverseer();

public:
  Imp(int _WorkPeriodMs, bool _AutoWorkCalc = true, bool _Critical = false);
  /*override */virtual ~Imp() { }
};

