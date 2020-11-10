#pragma once

#include <QSharedPointer>
#include <QMutex>
#include <QWaitCondition>
#include <QList>
#include <QSet>
#include <QThread>
#include <QElapsedTimer>
#include <QDateTime>

#include <Lib/Include/Common.h>
#include <Lib/Include/License_h.h>


const int kDefaultEndWorkersTimoutMs = 2000;

DefineClassS(CtrlWorker);
DefineClassS(WorkerStat);

class CtrlManager
{
  const bool         mConsole;
  const int          mEndWorkersTimoutMs;

  QList<CtrlWorkerS> mWorkers;
  QList<CtrlWorkerS> mGarbageWorkers;
  CtrlWorker*        mLogWorker;
  QMutex             mMutex; // sync manipulating with workers, mStop and mAliveCount
  QWaitCondition     mStopCondition;
  QElapsedTimer      mLastWatch;
  volatile bool      mStarted;
  volatile bool      mStop;
  volatile int       mAliveCount;

  QDateTime          mPublishStart;
  int                mPublishPeriodMs;
  QElapsedTimer      mPublishTimer;
  QSet<WorkerStatS>  mDeadWorkerStatList;

  LICENSE_HEADER;

public:
  template <typename CtrlWorkerT>
  void RegisterWorker(const QSharedPointer<CtrlWorkerT>& _WorkerPtr)
  { CtrlWorkerS w = _WorkerPtr.template staticCast<CtrlWorker>(); RegisterWorker(w); }
  void RegisterWorker(CtrlWorkerS& _WorkerPtr);
  void SetConsoleBreak();

  int Run();

  /*new */virtual void Stop();

protected:
  void SetLogWorker(CtrlWorker* _LogWorker);

private:
  void Start();

  void Watch();
  void CollectWorkerStats(CtrlWorker* worker);
  void LogWorkerStats(bool force = false);
  void DumpWorkerStats(const QList<WorkerStatS>& statList);
  void ClearWorkerStats();

protected:
  /// return continue or end work
  /*new */virtual bool InitReport();
  /*new */virtual bool DoReport();
  /*new */virtual void FinalReport();
  /*new */virtual void CriticalFail();

protected:
  /*new */virtual bool IsPublicStats();
  /*new */virtual void PublishStats(const QDateTime& startTime, const QDateTime& endTime, const QList<WorkerStatS>& statList);

private:
  void EndChilds();
  bool EndChildsAsk();
  bool EndChildsWaitAll();
  bool EndChildsTerminateAlive();
  bool EndChildsClear();

public:
  void OnWorkerFinished();

  explicit CtrlManager(bool _Console, int _EndWorkersTimoutMs = kDefaultEndWorkersTimoutMs);
  /*new */virtual ~CtrlManager();
};

