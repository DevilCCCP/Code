#pragma once

#include <Lib/Db/Db.h>
#include <Lib/Dispatcher/JobImp.h>

#include "Calc.h"


DefineClassS(MultiProcessCalc);

class MultiProcessCalc: public JobImp
{
  Calc      mCalc;

protected:
//  /*override */virtual char* TableName() Q_DECL_OVERRIDE;
//  /*override */virtual char* QuerySelect() Q_DECL_OVERRIDE;
//  /*override */virtual char* QueryJoin() Q_DECL_OVERRIDE;

//  /*override */virtual int  WorkIterations() Q_DECL_OVERRIDE;
//  /*override */virtual int  MaximumIteration() Q_DECL_OVERRIDE;
//  /*override */virtual int  JobMaximumSecs() Q_DECL_OVERRIDE;
//  /*override */virtual qint64 LoadJob(const QueryS& q) Q_DECL_OVERRIDE;
  /*override */virtual bool DoJob() Q_DECL_OVERRIDE;

public:
  MultiProcessCalc(const Db& _Db);
};
