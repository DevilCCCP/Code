#pragma once

#include <QElapsedTimer>

#include <Lib/Include/Common.h>


DefineClassS(ObjectState);
DefineClassS(Db);

const int kFramesLostTimeoutMs = 5000;
const int kStreamOpenTimeoutMs = 30000;
const int kStatusPeriodMs = 1000;

class SourceState
{
  DbS                 mDb;
  ObjectStateS        mObjectState;

  int                 mId;
  bool                mInit;
  bool                mBeginConnect;
  QElapsedTimer       mBeginConnectTimer;
  bool                mLastConnected;
  QElapsedTimer       mLastUpdateTimer;

  bool                mConnectDbWarning;

public:
  bool Init(int _Id);
  void UpdateObjectState(bool connected);

public:
  SourceState();
  ~SourceState();
};

