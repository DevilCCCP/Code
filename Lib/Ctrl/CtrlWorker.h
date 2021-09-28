#pragma once

#include <typeinfo>
#include <QVector>
#include <QMap>
#include <QSharedPointer>
#include <QThread>
#include <QMutex>
#include <QElapsedTimer>
#include <QWaitCondition>

#include <Lib/Common/FpsCalc.h>
#include <Lib/Include/License_h.h>


DefineClassS(CtrlManager);
DefineClassS(WorkerStat);

class CtrlWorker: public QThread
{
  const int              mWorkPeriodMs;
  const bool             mAutoWorkCalc;
  const bool             mCritical;
  int                    mCriticalFailMs;
  PROPERTY_GET_SET(bool, FastThread)

  CtrlManager*           mManager;
  QMutex                 mMutex;
  QWaitCondition         mSleepCondition;
  QElapsedTimer          mLastWork;
  FpsCalc                mFpsCalc;
  volatile bool          mWakeUp;

  volatile bool          mStop;
  qintptr                mTid;

  volatile bool          mFinished;
  QMutex                 mFinishMutex;
  QWaitCondition         mFinishCondition;
  int                    mCriticalWarnMs;
  int                    mCriticalWarnExtraMs;

  WorkerStatS            mWorkerStat;
  WorkerStatS            mAggregateWorkerStat;

  bool                   mFailState;
  QElapsedTimer          mFailTimer;
  int                    mFailStateDeadMs;

  LICENSE_HEADER
  ;
public:
  void SetCriticalWarnMs(int _CriticalWarnMs) { mCriticalWarnMs = _CriticalWarnMs; }
  void SetCriticalFailMs(int _CriticalFailMs) { mCriticalFailMs = _CriticalFailMs; }
  void SetFailStateDeadMs(int _FailStateDeadMs) { mFailStateDeadMs = _FailStateDeadMs; }
  void SetManager(CtrlManager* _Manager) { mManager = _Manager; }
  QMutex& Mutex() { return mMutex; }

  quint64 GetId() { return mTid; }
  bool Watch();

  int WorkPeriodMs() { return mWorkPeriodMs; }
  bool WorkStep() { return SayWork(); }

protected:
  CtrlManager* GetManager() { return mManager; }

  void AddWorkInfo(const QString& name, int index = -1);
  void ChangeWork(int index);
  void ReturnWork(int index);
  void CountWork(int index, int count = 1);

  bool SayWork();
  bool IsAlive() { return !mStop; }
  bool IsStop() { return mStop; }

  void SayOk();
  void SayFail(const QString& reason);

  void Rest();
  void Rest(int period);
  void Sleep(int period);
  void WakeUp();
  void WakeUpConfirmed();

private:
  void WorkStart();
  void NotifyFinished();
  void NotifyFinished2();

public:
  /*new */virtual const char* Name() { return typeid(*this).name(); }
  /*new */virtual const char* ShortName() { return typeid(*this).name(); }
private:
  /*override */virtual void	run() override;
protected:
  /*new */virtual bool DoInit();
  /*new */virtual bool DoCircle() = 0;
  /*new */virtual void DoRelease();
public:
  /*new */virtual void Stop();
  /*new */virtual void ConnectModule(CtrlWorker*) { }
  /*new */virtual void ConnectManager(CtrlManager*) { }

public:
  bool WaitFinish(unsigned long time = ULONG_MAX);

  void SetAggregateWorkerStat(const WorkerStatS& _AggregateWorkerStat);
  void CollectStats(QList<WorkerStatS>* statList, QMap<WorkerStat*, WorkerStat*>* aggregateStatMap);
  void ClearWorkerStat();
  WorkerStatS CloseStat();

public:
  explicit CtrlWorker(int _WorkPeriodMs, bool _AutoWorkCalc = true, bool _Critical = false, int _CriticalFailMs = 0);
  /*new */virtual ~CtrlWorker();
};

