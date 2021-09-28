#pragma once

#include <Lib/Db/Db.h>
#include <Lib/Dispatcher/JobAImp.h>

#include "Calc.h"


DefineClassS(MultiProcessCalc);

class MultiProcessCalc: public JobAImp
{
  DbTableBS     mJobTable;
  Calc          mCalc;

protected:
  /*override */virtual QString JobName() override;
  /*override */virtual DbTableBS JobTable() override;

  /*override */virtual qint64 IterationsPerAction() override;
  /*override */virtual qint64 LastIteration() override;
  /*override */virtual qint64 JobMaximumSecs() override;
  /*override */virtual qint64 JobUpdateMs() override;

  /*override */virtual bool DoJob(bool& result) override;
  /*override */virtual bool UpdateJob() override;

public:
  MultiProcessCalc(const Db& _Db);
};
