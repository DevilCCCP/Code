#pragma once

#include <Lib/Include/Common.h>


DefineClassS(Test);

class Test
{
public:
  virtual const char* Name() { return typeid(*this).name(); }

  virtual bool Prepare() { return true; }
  virtual bool Do() = 0;

public:
  Test() { }
  virtual ~Test() { }
};

