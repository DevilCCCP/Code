#include "JobAImp.h"
#include "Overseer.h"

#include <Lib/Log/Log.h>


const int kWorkPeriodMs = 200;

bool JobAImp::DoCircle()
{
  if (!GetDb().Connect()) {
    return true;
  }

  if (DoUpdate()) {
    while (TakeAction()) {
      if (!DoAction()) {
        break;
      }
    }
  }

  return true;
}

bool JobAImp::LoadSettings(SettingsA* settings)
{
  Q_UNUSED(settings);

  return true;
}

qint64 JobAImp::LastIteration()
{
  return -1;
}

qint64 JobAImp::JobMaximumSecs()
{
  return 30;
}

int JobAImp::JobMaxTries()
{
  return 3;
}

qint64 JobAImp::JobUpdateMs()
{
  return -1;
}

bool JobAImp::DoUpdate()
{
  if (JobUpdateMs() >= 0 && mUpdateTimer.elapsed() >= mNextUpdate) {
    if (!UpdateJob()) {
      return false;
    }
    mNextUpdate = mUpdateTimer.elapsed() + JobUpdateMs();
  }
  return true;
}

bool JobAImp::DoAction()
{
  if (JobMaxTries() > 0 && mJobTry > JobMaxTries()) {
    if (getDebug()) {
      Log.Info(QString("Job too many tries (job: %1, action: %2, itr: [%3, %4], try: %5)")
               .arg(mJob->Id).arg(mActionId).arg(mJobIterFrom).arg(mJobIterTo).arg(mJobTry));
    }
    ApplyAction(false);
    return SayWork();
  }

  if (getDebug()) {
    Log.Info(QString("Job taken (job: %1, action: %2, itr: [%3, %4], try: %5)")
             .arg(mJob->Id).arg(mActionId).arg(mJobIterFrom).arg(mJobIterTo).arg(mJobTry));
  }

  bool result = false;
  bool ok = DoJob(result);
  if (ok) {
    ApplyAction(result);
  } else {
    CancelAction();
  }
  return SayWork();
}

bool JobAImp::TakeAction()
{
  mJob.clear();

  if (!mInit) {
    QStringList argList;
    argList << QString::number(GetOverseer()->Id());
    argList << QString::number(JobMaximumSecs());
    if (!JobTake("job_init", argList)) {
      return false;
    }
    if (getJob()) {
      return true;
    }
    mInit = true;
  }

  {
    QStringList argList;
    argList << QString::number(GetOverseer()->Id());
    argList << QString::number(JobMaximumSecs());
    argList << QString::number(IterationsPerAction());
    argList << QString::number(LastIteration());
    if (!JobTake("job_take", argList)) {
      return false;
    }
  }
  return getJob();
}

bool JobAImp::ApplyAction(bool result)
{
  auto q = GetDb().MakeQuery();
  q->prepare(QString("SELECT %1_job_done(%2, %3);").arg(JobName()).arg(mActionId).arg(result? "true": "false"));
  if (!GetDb().ExecuteQuery(q)) {
    return false;
  }
  return true;
}

bool JobAImp::CancelAction()
{
  auto q = GetDb().MakeQuery();
  q->prepare(QString("SELECT %1_job_cancel(%2);").arg(JobName()).arg(mActionId));
  if (!GetDb().ExecuteQuery(q)) {
    return false;
  }
  return true;
}

bool JobAImp::JobTake(const QString& function, const QStringList& args)
{
  auto q = GetDb().MakeQuery();
  q->prepare(QString("SELECT a.action_id_, a.iter_from_, a.iter_to_, a.try_"
                     ", %2 FROM %1_%3(%4) AS a"
                     " JOIN %1_job j ON j._id = a.job_id_;")
             .arg(JobName(), JobTable()->GetColumnsWithTag("j"), function, args.join(", ")));
  if (!GetDb().ExecuteQuery(q)) {
    return false;
  }
  if (!q->next()) {
    return true;
  }

  int index = 0;
  mActionId    = q->value(index++).toLongLong();
  mJobIterFrom = q->value(index++).toLongLong();
  mJobIterTo   = q->value(index++).toLongLong();
  mJobTry      = q->value(index++).toInt();
  JobTable()->FillItem(q, index, mJob);
  return true;
}


JobAImp::JobAImp(const Db& _Db, int _WorkPeriodMs)
  : ImpD(_Db, _WorkPeriodMs >= 0? _WorkPeriodMs: kWorkPeriodMs, true, true)
  , mInit(false), mNextUpdate(0)
{
  GetDb().MoveToThread(this);

  mUpdateTimer.start();
}

