#include <QMutexLocker>

#include <Lib/Settings/SettingsA.h>
#include <Lib/Ctrl/WorkerStat.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Db/ObjectLog.h>

#include "LogPublisher.h"


const int kWorkPeriodMs = 100;
const int kLogTruncHours = 30*24;
const int kTruncPeriodMs = 15*60*1000;

bool LogPublisher::LoadSettings(SettingsA* settings)
{
  settings->SetSilent(true);
  mLogTruncHours = settings->GetValue("LogTruncHours", kLogTruncHours).toInt();
  settings->SetSilent(false);
  mTruncTimer.start();
  return true;
}

bool LogPublisher::DoCircle()
{
  ProcessLogs();
  return true;
}

void LogPublisher::DoRelease()
{
  ProcessLogs();
}

void LogPublisher::PushLog(const QDateTime& startTime, const QDateTime& endTime, const QList<WorkerStatS>& statList)
{
  WorkerLog log;
  log.StartTime = startTime;
  log.EndTime   = endTime;
  log.StatList  = statList;

  QMutexLocker lock(&mLogMutex);
  mLogList.append(log);
}

void LogPublisher::ProcessLogs()
{
  QDateTime startTime;
  QDateTime endTime;
  QList<WorkerStatS> statList;

  while (GetFirstLog(startTime, endTime, statList)) {
    if (PublishLog(startTime, endTime, statList)) {
      PopLog();
    } else {
      break;
    }
  }
}

bool LogPublisher::PublishLog(const QDateTime& startTime, const QDateTime& endTime, const QList<WorkerStatS>& statList)
{
  if (!GetDb().Connect()) {
    return false;
  }
  SayWork();
  DbTransactionS transaction = GetDb().BeginTransaction();

  for (auto itr = statList.begin(); itr != statList.end(); itr++) {
    const WorkerStatS& workerStat = *itr;
    const QVector<WorkStat>& workStatList = workerStat->WorkStatList();
    int startIndex = (workStatList.size() == 1)? 0: 1;
    for (int i = startIndex; i < workStatList.size(); i++) {
      const WorkStat& workStat = workStatList.at(i);
      ObjectLogS objectLog(new ObjectLog());
      objectLog->ObjectId    = GetOverseer()->Id();
      objectLog->PeriodStart = startTime;
      objectLog->PeriodEnd   = endTime;
      objectLog->ThreadName  = workerStat->Name();
      objectLog->WorkName    = workStat.Name;
      objectLog->TotalTime   = workerStat->TotalTimeMs();
      objectLog->Circles     = workStat.Circles;
      objectLog->WorkTime    = workStat.WorkNsecs / 1000000;
      objectLog->LongestWork = workStat.LongestWorkNsecs / 1000000;
      if (!GetDb().getObjectLogTable()->Insert(objectLog)) {
        return false;
      }
      SayWork();
    }
  }

  SayWork();
  if (!transaction->Commit()) {
    return false;
  }

  if (mTruncTimer.elapsed() >= mNextTrunc && SayWork()) {
    if (GetDb().getObjectLogTable()->TruncHours(GetOverseer()->Id(), mLogTruncHours)) {
      mNextTrunc = mTruncTimer.elapsed() + kTruncPeriodMs;
    }
    SayWork();
  }
  return true;
}

bool LogPublisher::GetFirstLog(QDateTime& startTime, QDateTime& endTime, QList<WorkerStatS>& statList)
{
  QMutexLocker lock(&mLogMutex);
  if (mLogList.isEmpty()) {
    return false;
  }

  const WorkerLog& log = mLogList.first();
  startTime = log.StartTime;
  endTime   = log.EndTime;
  statList  = log.StatList;
  return true;
}

void LogPublisher::PopLog()
{
  QMutexLocker lock(&mLogMutex);
  mLogList.removeFirst();
}


LogPublisher::LogPublisher(const DbS& _LogDb)
  : ImpD(*_LogDb, kWorkPeriodMs)
  , mLogDb(_LogDb), mLogTruncHours(kLogTruncHours)
  , mNextTrunc(0)
{
}
