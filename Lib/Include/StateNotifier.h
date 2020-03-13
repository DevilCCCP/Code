#pragma once
#include <Lib/Include/Common.h>


DefineClassS(StateNotifier);

class StateNotifier
{
public:
  /*new */ virtual void NotifyGood() = 0;
  /*new */ virtual void NotifyNothing() = 0;
  /*new */ virtual void NotifyWarning() = 0;
  /*new */ virtual void NotifyError() = 0;

public:
  /*new */ virtual ~StateNotifier() { }
};
