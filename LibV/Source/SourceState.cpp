#include <Lib/Db/ObjectState.h>
#include <Lib/Log/Log.h>

#include "SourceState.h"


const int kBeginConnectMs = 5000;
const int kUpdateMinimum = 1000;

bool SourceState::Init(int _Id)
{
  mId = _Id;
  if (!mDb->OpenDefault()) {
    return false;
  }
  mBeginConnectTimer.start();
  return true;
}

void SourceState::UpdateObjectState(bool connected)
{
  if (mBeginConnect) {
    if (!connected && mBeginConnectTimer.elapsed() < kBeginConnectMs) {
      return;
    }
    mBeginConnect = false;
  }

  if (!mDb->Connect()) {
    if (!mConnectDbWarning) {
      Log.Warning("State: connect db fail");
    }
    mConnectDbWarning = true;
    return;
  } else {
    mConnectDbWarning = false;
  }

  ObjectState::EConnectState state = (connected)? ObjectState::eConnected: ObjectState::eDisconnected;
  if (!mInit) {
    if (mId) {
      mInit = mObjectState->InitState(mId, ObjectState::eConnect, ObjectState::eOff, state);
      if (mInit) {
        mLastConnected = connected;
        mLastUpdateTimer.start();
      }
    } else {
      LOG_ERROR_ONCE("ObjectState not initialized");
    }
  } else if (connected != mLastConnected || mLastUpdateTimer.elapsed() > kUpdateMinimum) {
    mObjectState->UpdateState(state);
    mLastConnected = connected;
    mLastUpdateTimer.restart();
  }
}

SourceState::SourceState()
  : mDb(new Db(false)), mObjectState(new ObjectState(*mDb, false))
  , mId(0), mInit(false), mBeginConnect(true)
  , mConnectDbWarning(false)
{
}

SourceState::~SourceState()
{
}
