#pragma once

#include <QMutex>

#include <Lib/Ctrl/CtrlWorker.h>


DefineClassS(MpController);
DefineClassS(CameraPlayer);

class MpController: public CtrlWorker
{
  enum EState {
    eNone,
    eConnectCamera,
    eDisconnectCamera,
    ePlayCamera,
    eStateIllegal
  };

  CameraPlayerS mCameraPlayer;
  EState        mState;

  QMutex        mMutex;
  bool          mLive;
  QDateTime     mTimestamp;
  int           mSpeedNum;
  int           mSpeedDenum;

public:
  /*new */virtual const char* Name() { return "Media player controller"; }
  /*new */virtual const char* ShortName() { return "Mc"; }
protected:
//  /*new */virtual bool DoInit();
  /*new */virtual bool DoCircle();
  /*new */virtual void DoRelease();
public:
//  /*new */virtual void Stop();

public:
  void PlayLive();
  void PlayArchive(const QDateTime& ts, int speedNum, int speedDenum);
  void PlayRetry();
  void PlayStop();

private:
  void Connect();
  void Disconnect();
  void Play();

public:
  MpController(const CameraPlayerS& _CameraPlayer);
};

