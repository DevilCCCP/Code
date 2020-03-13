#include <QMutexLocker>
#include <QThread>

#include <Lib/Db/Db.h>
#include <Lib/Db/ObjectState.h>
#include <Lib/Log/Log.h>

#include "StateInformer.h"


const int kUpdateMinimum = 1000;

bool StateInformer::Init(int _Id)
{
  QMutexLocker lock(&mMutex);
  mId = _Id;
  if (!mDb) {
    mDb.reset(new Db(false));
    mObjectState.reset(new ObjectState(*mDb, false));
    if (!mDb->OpenDefault()) {
      return false;
    }
  }
  mStateReset.start();
  return true;
}

bool StateInformer::SetState(int state)
{
  QMutexLocker lock(&mMutex);
  if (!mId) {
    QElapsedTimer timer;
    timer.start();
    while (!mId) {
      lock.unlock();
      if (timer.elapsed() >= 2000) {
        LOG_ERROR_ONCE("StateInformer: not got Id");
        return false;
      }
      QThread::msleep(1);
      lock.relock();
      if (mEnded) {
        return true;
      }
    }
  }
  mDb->MoveToThread();

  if (mEnded) {
    return true;
  }
  if (!mInit) {
    InitDbState(state);
  }

  if (mLastState == state) {
    bool updated = false;
    if (mStateUpdate.elapsed() > kUpdateMinimum) {
      if (!SetDbState(state)) {
        return false;
      }
      updated = true;
    }
    ResetState(state, updated);
    return true;
  }

  return SetDbState(state);
}

bool StateInformer::UpdateState()
{
  QMutexLocker lock(&mMutex);
  mDb->MoveToThread();
  if (mEnded) {
    return true;
  }
  if (!mInit) {
    if (mStateReset.elapsed() > mStateLastMs) {
      return InitDbState(ObjectState::eNotAvailable);
    }
    return false;
  }

  if (mLastState != ObjectState::eGood && mStateReset.elapsed() > mStateLastMs) {
    return SetDbState(ObjectState::eGood);
  } else if (mStateUpdate.elapsed() > kUpdateMinimum) {
    return SetDbState(mLastState);
  }
  return true;
}

bool StateInformer::End()
{
  QMutexLocker lock(&mMutex);
  mDb->MoveToThread();
  mEnded = true;
  if (!mInit) {
    return false;
  }

  if (mLastState != ObjectState::eNotAvailable) {
    return SetDbState(ObjectState::eNotAvailable);
  }
  return true;
}

void StateInformer::ResetState(int state, bool updated)
{
  mLastState = state;
  mStateReset.start();
  if (updated) {
    mStateUpdate.start();
  }
}

bool StateInformer::InitDbState(int state)
{
  if (!ConnectDb()) {
    return false;
  }

  mInit = mObjectState->InitState(mId, ObjectState::eService, ObjectState::eNotAvailable, state);
  if (mInit) {
    ResetState(state, true);
  }
  return mInit;
}

bool StateInformer::SetDbState(int state)
{
  if (!ConnectDb()) {
    return false;
  }

  if (mObjectState->UpdateState(state)) {
    ResetState(state, true);
    return true;
  } else {
    Log.Warning("State: update fail");
    return false;
  }
}

bool StateInformer::ConnectDb()
{
  if (!mDb->Connect()) {
    if (!mConnectDbWarning) {
      Log.Warning("State: connect db fail");
    }
    mConnectDbWarning = true;
    return false;
  } else {
    mConnectDbWarning = false;
  }
  return true;
}


StateInformer::StateInformer(int _StateLastMs)
  : mStateLastMs(_StateLastMs)
  , mId(0), mInit(false), mEnded(false), mLastState(0)
{
}

StateInformer::StateInformer(DbS _Db, int _StateLastMs)
  : mStateLastMs(_StateLastMs), mDb(_Db), mObjectState(new ObjectState(*mDb))
  , mId(0), mInit(false), mEnded(false), mLastState(0)
{
}
