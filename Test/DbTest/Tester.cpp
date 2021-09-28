#include <Lib/Log/Log.h>
#include <Lib/Common/Format.h>

#include "Tester.h"


bool Tester::DoTest()
{
  if (!mTest->Prepare()) {
    return false;
  }
  Log.Info(QString("Run %1 test").arg(mTest->Name()));
  mTimer.start();

  bool ok = mTest->Do();

  qint64 time = mTimer.elapsed();
  Log.Info(QString("Done %1 in %2 (%3)").arg(mTest->Name()).arg(FormatTime(time)).arg(ok? "ok": "fail"));
  return ok;
}


Tester::Tester(const TestS& _Test)
  : mTest(_Test)
{
}

