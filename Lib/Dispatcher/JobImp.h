#pragma once

#include <QElapsedTimer>

#include <Lib/Db/Db.h>

#include "Imp.h"


DefineClassS(JobImp);

class JobImp: public Imp
{
  const Db&     mDb;
  QString       mJobInitString;
  QString       mJobTakeString;
  QString       mJobDoneString;
  QString       mJobCancelString;

  QElapsedTimer mTimer;
  qint64        mNextInit;
  qint64        mJobActionId;
  qint64        mJobId;
  qint64        mJobIterFrom;
  qint64        mJobIterTo;
  QByteArray    mJobData;
  bool          mJobDone;
  bool          mJobResult;

protected:
  const Db& GetDb()           { return mDb; }
  qint64 CurrentJobId()       { return mJobId; }
  qint64 CurrentJobIterFrom() { return mJobIterFrom; }
  qint64 CurrentJobIterTo()   { return mJobIterTo; }
  QByteArray CurrentJobData() { return mJobData; }

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "JobImp"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "J"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
//  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;
protected:
  /*new */virtual QString TableName();
  /*new */virtual QString FunctionName();
  /*new */virtual QString QuerySelect();
  /*new */virtual QString QueryJoin();

  /*new */virtual qint64 WorkIterations();
  /*new */virtual qint64 MaximumIteration();
  /*new */virtual qint64 JobMaximumSecs();
  /*new */virtual qint64 LoadJob(const QueryS& q);
  /*new */virtual bool DoJob() = 0;
  /*new */virtual bool DoNoJob();

private:
  void JobInit();
  void JobTake();
  bool JobDone();
  bool JobCancel();

public:
  JobImp(const Db& _Db);
};
