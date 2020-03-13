#include <QMutexLocker>

#include <Lib/Log/Log.h>
#include <Lib/Net/Chater.h>
#include <LibV/Include/VideoMsg.h>
#include <LibV/Player/CameraPlayer.h>

#include "MpController.h"


bool MpController::DoCircle()
{
  QMutexLocker lock(&mMutex);
  switch (mState) {
  case eNone:             break;
  case eConnectCamera:    Connect(); break;
  case eDisconnectCamera: Disconnect(); break;
  case ePlayCamera:       Play(); break;
  default:                LOG_WARNING_ONCE(QString("%1 at wrong state: %2").arg(Name()).arg(mState)); break;
  }

  return true;
}

void MpController::DoRelease()
{
  mCameraPlayer.reset();
}

void MpController::PlayLive()
{
  {
    QMutexLocker lock(&mMutex);
    mLive = true;
    mState = eConnectCamera;
  }

  WakeUpConfirmed();
}

void MpController::PlayArchive(const QDateTime& ts, int speedNum, int speedDenum)
{
  {
    QMutexLocker lock(&mMutex);
    mLive       = false;
    mTimestamp  = ts;
    mSpeedNum   = speedNum;
    mSpeedDenum = speedDenum;
    mState      = eConnectCamera;
  }

  WakeUpConfirmed();
}

void MpController::PlayRetry()
{
  {
    QMutexLocker lock(&mMutex);
    mState = eConnectCamera;
  }

  WakeUpConfirmed();
}

void MpController::PlayStop()
{
  {
    QMutexLocker lock(&mMutex);
    mState = eDisconnectCamera;
  }

  WakeUpConfirmed();
}

void MpController::Connect()
{
  ChaterS chater = mCameraPlayer->Chat();
  if (!chater) {
    return;
  }

  if (mLive) {
    Log.Info(QString("Request live (camId: %1)").arg(mCameraPlayer->CameraId()));
    LiveRequest* req;
    if (chater->PrepareMessage(eMsgLiveRequest, req)) {
      req->CameraId  = mCameraPlayer->CameraId();
      req->Priority  = 0;
    }
  } else {
    Log.Info(QString("Request arch (camId: %1, ts: %2, speed: %3/%4)").arg(mCameraPlayer->CameraId())
            .arg(mTimestamp.toString()).arg(mSpeedNum).arg(mSpeedDenum));
    ArchRequest* req;
    if (chater->PrepareMessage(eMsgArchRequest, req)) {
      req->CameraId   = mCameraPlayer->CameraId();
      req->Priority   = 0;
      req->Timestamp  = mTimestamp.toMSecsSinceEpoch();
      req->SpeedNum   = mSpeedNum;
      req->SpeedDenum = mSpeedDenum;
    }
  }

  if (chater->SendMessage()) {
    mState = ePlayCamera;
  }
}

void MpController::Disconnect()
{
  ChaterS chater = mCameraPlayer->Chat();
  if (chater) {
    Log.Info(QString("Request Stop (camId: %1)").arg(mCameraPlayer->CameraId()));
    if (chater->SendSimpleMessage(eMsgStop)) {
      mState = eNone;
    }
  }
}

void MpController::Play()
{
  if (!mCameraPlayer->NeedConfirm()) {
    return;
  }

  ChaterS chater = mCameraPlayer->Chat();
  if (chater) {
    chater->SendSimpleMessage(eMsgContinue);
  }
}


MpController::MpController(const CameraPlayerS& _CameraPlayer)
  : CtrlWorker(200)
  , mCameraPlayer(_CameraPlayer), mState(eNone)
{
}

