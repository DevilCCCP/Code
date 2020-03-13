#pragma once

#include <QList>
#include <QElapsedTimer>

#include <Lib/Include/FrameA.h>
#include <Lib/Dispatcher/Imp.h>


DefineClassS(Conveyor);

class Conveyor: public Imp
{
  int mId;

  QList<Conveyor*>  mNextImps;
  QList<Conveyor*>  mPrevImps;
  QList<FrameAS>    mConveyorFrames;
  int               mFrameOverflowCount;
  FrameAS           mCurrentFrame;

  qint64            mFrameDone;
  qint64            mFrameLost;
  qint64            mFrameLast;
  qreal             mLossPercent;
  QElapsedTimer     mStuckTimer;

protected:
  const FrameAS& CurrentFrame() { return mCurrentFrame; }

public:
//  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return typeid(*this).name(); }
//  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return typeid(*this).name(); }
protected:
//  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;
public:
//  /*override */virtual void Stop() Q_DECL_OVERRIDE;
  /*override */virtual void ConnectModule(CtrlWorker* worker) Q_DECL_OVERRIDE;
  void DisconnectModule(CtrlWorker* worker);
//  /*override */virtual void ConnectManager(CtrlManager*) Q_DECL_OVERRIDE;

protected:
  // return true if process counted as target work, always false if EmergeFrame is target result
  /*new */virtual bool ProcessFrame() { return false; }
  /*new */virtual void OnOverflowWarn();
  /*new */virtual void OnOverflow(QList<FrameAS>& conveyorFrames) = 0;

protected:
  bool EmergeFrame(const FrameAS& frame);
  bool HasOverflowWarn() { return mConveyorFrames.size() >= mFrameOverflowCount/2; }

private:
  void ConnectParent(Conveyor* conveyor);
  void DisconnectParent(Conveyor* conveyor);

public:
  void ClearConveyor();
  bool PushFrame(const FrameAS& frame);

private:
  void ProcessOverflow();

public:
  Conveyor(int _FrameOverflowCount = -1, int _WorkPeriodMs = -1);
  /*override */virtual ~Conveyor();
};

