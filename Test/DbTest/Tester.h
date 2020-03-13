#pragma once

#include <QElapsedTimer>

#include "Test.h"


class Tester
{
  TestS         mTest;
  QElapsedTimer mTimer;

public:
  bool DoTest();

public:
  Tester(const TestS& _Test);
};

