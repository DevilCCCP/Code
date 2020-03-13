#include "JobImp.h"
#include "Overseer.h"

#include <Lib/Log/Log.h>


const int kWorkPeriodMs = 0;
const int kJobWaitMs = 500;
const int kJobRetakeMs = 2000;

bool JobImp::DoInit()
{
  mJobInitString = QString("SELECT %5 FROM %2_init(%3, %4) AS %1"
                           " %6")
      .arg(FunctionName()).arg(TableName())
      .arg(GetOverseer()->Id()).arg(JobMaximumSecs()).arg(QuerySelect(), QueryJoin());
  mJobTakeString = QString("SELECT %6 FROM %2_take(%3, %4, %5, ?) AS %1"
                           " %7")
      .arg(FunctionName()).arg(TableName())
      .arg(GetOverseer()->Id()).arg(JobMaximumSecs()).arg(WorkIterations()).arg(QuerySelect(), QueryJoin());
  mJobDoneString = QString("SELECT %1_done(?, ?);").arg(TableName());
  mJobCancelString = QString("UPDATE %1_action SET try = try - 1 WHERE _id = ?;").arg(TableName());
  return true;
}

bool JobImp::DoCircle()
{
  if (!mDb.Connect()) {
    return true;
  }

  if (!mJobActionId) {
    if (mTimer.elapsed() >= mNextInit) {
      JobInit();
      if (!mJobActionId) {
        mNextInit = mTimer.elapsed() + kJobRetakeMs;
      }
    }
    if (!mJobActionId) {
      JobTake();
    }
    if (mJobActionId) {
      Log.Info(QString("Job taken (id: %1)").arg(mJobActionId));
    }
  }
  if (mJobActionId) {
    if (!mJobDone) {
      mJobResult = DoJob();
      mJobDone = true;
    }
    if (SayWork() || mJobResult) {
      if (JobDone()) {
        Log.Info(QString("Job done (id: %1)").arg(mJobActionId));
        mJobActionId = 0;
      }
    } else {
      if (JobCancel()) {
        Log.Info(QString("Job canceled (id: %1)").arg(mJobActionId));
        mJobActionId = 0;
      }
    }
  } else {
    DoNoJob();
    if (SayWork()) {
      Rest(kJobWaitMs);
    }
  }
  return true;
}

QString JobImp::TableName()
{
  return "job";
}

QString JobImp::FunctionName()
{
  return "jt";
}

QString JobImp::QuerySelect()
{
  return QString("%1.action_id_, %1.job_id_, %1.iter_from_, %1.iter_to_, j.data").arg(FunctionName());
}

QString JobImp::QueryJoin()
{
  // ! never join job_action it doesn't exists in join time, use job_take function output instead !
  return QString("JOIN %2 j ON j._id = %1.job_id_").arg(FunctionName()).arg(TableName());
}

qint64 JobImp::WorkIterations()
{
  return 1;
}

qint64 JobImp::MaximumIteration()
{
  return 0;
}

qint64 JobImp::JobMaximumSecs()
{
  return 5;
}

qint64 JobImp::LoadJob(const QueryS& q)
{
  int index = 0;
  mJobActionId = q->value(index++).toLongLong();
  mJobId       = q->value(index++).toLongLong();
  mJobIterFrom = q->value(index++).toLongLong();
  mJobIterTo   = q->value(index++).toLongLong();
  mJobData     = q->value(index++).toByteArray();
  return mJobActionId;
}

bool JobImp::DoNoJob()
{
  return true;
}

void JobImp::JobInit()
{
  auto q = mDb.MakeQuery();
  q->prepare(mJobInitString);
  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return;
  }
  mJobActionId = LoadJob(q);
}

void JobImp::JobTake()
{
  auto q = mDb.MakeQuery();
  q->prepare(mJobTakeString);
  q->addBindValue(MaximumIteration());
  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return;
  }
  mJobActionId = LoadJob(q);
}

bool JobImp::JobDone()
{
  auto q = mDb.MakeQuery();
  q->prepare(mJobDoneString);
  q->addBindValue(mJobActionId);
  q->addBindValue(mJobResult);
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  mJobDone = false;
  return true;
}

bool JobImp::JobCancel()
{
  auto q = mDb.MakeQuery();
  q->prepare(mJobCancelString);
  q->addBindValue(mJobActionId);
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  return true;
}


JobImp::JobImp(const Db& _Db)
  : Imp(kWorkPeriodMs, true, true), mDb(_Db)
  , mNextInit(0), mJobActionId(0), mJobId(0), mJobIterFrom(0), mJobIterTo(0), mJobDone(false), mJobResult(false)
{
  mTimer.start();
}

