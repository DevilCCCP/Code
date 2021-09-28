#include "MultiProcessCalc.h"
#include "TestJob.h"


QString MultiProcessCalc::JobName()
{
  return "test";
}

DbTableBS MultiProcessCalc::JobTable()
{
  if (!mJobTable) {
    mJobTable.reset(new TestJobTable(GetDb()));
  }
  return mJobTable;
}

qint64 MultiProcessCalc::IterationsPerAction()
{
  return 1;
}

qint64 MultiProcessCalc::LastIteration()
{
  return -1;
}

qint64 MultiProcessCalc::JobMaximumSecs()
{
  return 30;
}

qint64 MultiProcessCalc::JobUpdateMs()
{
  return -1;
}

bool MultiProcessCalc::DoJob(bool& result)
{
  TestJob* testJob = dynamic_cast<TestJob*>(getJob().data());
  int circles = testJob->Circles;
  for (int i = getJobIterFrom(); i <= getJobIterTo(); i++) {
    mCalc.CalcCircles(i, circles);
  }
  result = true;
  return true;
}

bool MultiProcessCalc::UpdateJob()
{
  return true;
}

MultiProcessCalc::MultiProcessCalc(const Db& _Db)
  : JobAImp(_Db)
{
  setDebug(true);
}

