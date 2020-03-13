#pragma once

#ifndef NOLICENSE
#ifndef PROGRAM_ABBR
#error PROGRAM_ABBR must be defined
#endif

#define MAKE_STRINGL(A) #A
#define LICENSE_PREFIX_(prefix) MAKE_STRINGL(prefix)
#define LICENSE_PREFIX LICENSE_PREFIX_(PROGRAM_ABBR)

#define LICENSE_HEADER CounterL mCounterL;
const int kLcCount = 33;

class CounterL {
  int mCounter;
  int mPeriod;

public:
  bool Test()
  {
    if (++mCounter >= mPeriod) {
      mCounter = 0;
      return true;
    }
    return false;
  }

  CounterL()
    : mCounter(0), mPeriod(666)
  { }
};
#else
#define LICENSE_HEADER
#endif
