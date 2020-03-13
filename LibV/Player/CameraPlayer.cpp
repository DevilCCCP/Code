#include <Lib/Net/Chater.h>
#include <LibV/Include/VideoMsg.h>

#include "CameraPlayer.h"


void CameraPlayer::Clear()
{
  mChater.clear();
}

bool CameraPlayer::NeedConfirm()
{
  if (mConfirmTimer.elapsed() > kConfirmFramesMs / 2) {
    mConfirmTimer.restart();
    return true;
  }
  return false;
}

bool CameraPlayer::NeedTest()
{
  if (mConfirmTimer.elapsed() > kConfirmFramesMs / 4) {
    mConfirmTimer.restart();
    return true;
  }
  return false;
}


CameraPlayer::CameraPlayer(ChaterS& _Chater, int _CameraId)
  : mChater(_Chater), mCameraId(_CameraId)
{
  mStartTimestamp = 0;
  mConfirmTimer.start();
}

CameraPlayer::~CameraPlayer()
{
}
