#pragma once

#include <QMutex>
#include <QElapsedTimer>

#include <Lib/Include/Common.h>


DefineClassS(Db);
DefineClassS(ObjectState);

class StateInformer
{
  const int     mStateLastMs;
  DbS           mDb;
  ObjectStateS  mObjectState;

  int           mId;
  QMutex        mMutex;
  bool          mInit;
  bool          mEnded;
  int           mLastState;
  bool          mConnectDbWarning;
  QElapsedTimer mStateReset;
  QElapsedTimer mStateUpdate;

public:
  bool Init(int _Id);
  bool SetState(int state);
  bool UpdateState();
  bool End();

private:
  void ResetState(int state, bool updated);
  bool InitDbState(int state);
  bool SetDbState(int state);
  bool ConnectDb();

public:
  StateInformer(int _StateLastMs = 5000);
  StateInformer(DbS _Db, int _StateLastMs = 5000);
};

