#include <QMutexLocker>
#include <QElapsedTimer>

#include <Lib/Log/Log.h>
#include <Lib/Ctrl/CtrlManager.h>
#include <Lib/Net/Chater.h>
#include <LibV/Player/FrameReceiver.h>
#include <LibV/Player/CameraPlayer.h>
#include <LibV/MediaServer/VideoSysInfo.h>
#include <LibV/MediaServer/TrFrame.h>

#include "MediaPlayerV.h"
#include "MpController.h"
#include "Media.h"


bool MediaPlayerV::Open()
{
  if (mReqState < eInit) {
    mReqState = eInit;
  }
  return OnInit();
}

bool MediaPlayerV::Pause()
{
  mReqState = eInit;
  if (mCurState == eNotInit) {
    return false;
  }
  return OnPause();
}

void MediaPlayerV::Stop()
{
  mReqState = eInit;
  if (mCurState == eNotInit) {
    return;
  }
  OnStop();
}

void MediaPlayerV::Release()
{
  mReqState = eNotInit;
  if (mCurState == eNotInit) {
    return;
  }
  if (mCurState != eInit) {
    OnStop();
  }
  OnRelease();
}

bool MediaPlayerV::OnVideoInfo(const qint64& timestamp)
{
  Q_UNUSED(timestamp);
  return true;
}

bool MediaPlayerV::OnVideoFrame(const Frame::Header* header, qint64& timestamp)
{
  timestamp = header->Timestamp;
  return true;
}

void MediaPlayerV::VideoGranted(CameraPlayer* player)
{
  Log.Info(QString("Video granted (%1)").arg(player->CameraId()));
  QMutexLocker lock(&mPlayMutex);
  mCurState = ePlay;
  mPlayRespond = true;
  mPlayOk = true;
  mPlayWait.wakeAll();
}

void MediaPlayerV::VideoDenied(CameraPlayer* player)
{
  Log.Info(QString("Video granted (%1)").arg(player->CameraId()));
  QMutexLocker lock(&mPlayMutex);
  mCurState = eInit;
  mPlayRespond = true;
  mPlayOk = false;
  mPlayWait.wakeAll();
}

void MediaPlayerV::VideoNoStorage(CameraPlayer* player)
{
  Log.Info(QString("Video NoStorage (%1)").arg(player->CameraId()));
  QMutexLocker lock(&mPlayMutex);
  mCurState = eInit;
  mPlayRespond = true;
  mPlayOk = false;
  mPlayWait.wakeAll();
}

void MediaPlayerV::VideoDropped(CameraPlayer* player)
{
  Log.Info(QString("Video denied (%1)").arg(player->CameraId()));
  QMutexLocker lock(&mPlayMutex);
  mCurState = eInit;
  mPlayRespond = true;
  mPlayOk = false;
  mPlayWait.wakeAll();
}

void MediaPlayerV::VideoAborted(CameraPlayer* player)
{
  Log.Info(QString("Video aborted (%1)").arg(player->CameraId()));
  QMutexLocker lock(&mPlayMutex);
  mCurState = eInit;
  mPlayRespond = true;
  mPlayOk = false;
  mPlayWait.wakeAll();
}

void MediaPlayerV::VideoInfo(CameraPlayer* player, const qint64& timestamp)
{
  Q_UNUSED(player);

  if (mTimestamp == 0) {
    QMutexLocker lock(&mPlayMutex);
    mTimestamp = timestamp;
    mPlayWait.wakeAll();
  }
  OnVideoInfo(timestamp);
}

void MediaPlayerV::VideoFrame(CameraPlayer* player, FrameS& frame)
{
  Q_UNUSED(player);

  if (mReqState != ePlay) {
    mCurState = eInit;
    return;
  }

  if (mTimestamp == 0) {
    mCurState = ePlay;
    QMutexLocker lock(&mPlayMutex);
    mTimestamp = frame->GetHeader()->Timestamp;
    mPlayWait.wakeAll();
  }

  qint64 playTimestamp;
  if (OnVideoFrame(frame->GetHeader(), playTimestamp)) {
    mMediaPackager->InFrame(playTimestamp, frame->VideoData(), frame->VideoDataSize());
    TrFrameS trFrame;
    while (mMediaPackager->GetNextFrame(trFrame)) {
      GetOutChannel()->InFrame(trFrame);
    }
  }
}

void MediaPlayerV::Disconnected(CameraPlayer* player)
{
  Log.Info(QString("Video disconnected (%1)").arg(player->CameraId()));
  CreateChater(false);
  MpControllerS mpc = mMpController;
  if (mpc) {
    mpc->PlayRetry();
  } else {
    QMutexLocker lock(&mPlayMutex);
    mCurState = eInit;
    mPlayRespond = false;
    mPlayOk = false;
    mPlayWait.wakeAll();
  }
}

bool MediaPlayerV::PlayLive()
{
  mReqState = ePlay;
  if (!OnInit()) {
    return false;
  }

  OnInitPlay();
  return OnPlayLive();
}

bool MediaPlayerV::PlayArchive(const QDateTime& ts, int speed, int speedDenum)
{
  mReqState = ePlay;
  if (!OnInit()) {
    return false;
  }

  OnInitPlay();
  return OnPlayArchive(ts, speed, speedDenum);
}

bool MediaPlayerV::WaitConnect(int timeoutMs, int warnTimeoutMs)
{
  QElapsedTimer timer;
  timer.start();
  bool warn = false;
  QMutexLocker lock(&mPlayMutex);
  while (!mPlayRespond) {
    mPlayWait.wait(&mPlayMutex, 200);
    if (timer.elapsed() > warnTimeoutMs) {
      if (!warn) {
        Log.Warning(QString("Wait frame too long (CamId: %1)").arg(mCameraId));
        warn = true;
      } else if (timer.elapsed() > timeoutMs) {
        Log.Warning(QString("Wait frame timeout (CamId: %1, time: %2)").arg(mCameraId).arg(timer.elapsed()));
        return false;
      }
    }
  }
  return true;
}

bool MediaPlayerV::WaitFrame(int timeoutMs, int warnTimeoutMs)
{
  QElapsedTimer timer;
  timer.start();
  bool warn = false;
  QMutexLocker lock(&mPlayMutex);
  while (mTimestamp == 0) {
    mPlayWait.wait(&mPlayMutex, 200);
    if (timer.elapsed() > warnTimeoutMs) {
      if (!warn) {
        Log.Warning(QString("Wait frame too long (CamId: %1)").arg(mCameraId));
        warn = true;
      } else if (timer.elapsed() > timeoutMs) {
        Log.Warning(QString("Wait frame timeout (CamId: %1, time: %2)").arg(mCameraId).arg(timer.elapsed()));
        return false;
      }
    }
  }
  return true;
}

void MediaPlayerV::OnInitPlay()
{
  QMutexLocker lock(&mPlayMutex);
  mPlayRespond = false;
  mPlayOk = false;
  mTimestamp = 0;
}

bool MediaPlayerV::OnInit()
{
  if (mCurState >= eInit) {
    return true;
  }

  QString mediaPath = (GetMedia()->Id().startsWith('/'))? GetMedia()->Id().mid(1): GetMedia()->Id();
  mCameraId = mediaPath.toInt();
  if (!mCameraId) {
    return false;
  }

  CreateChater(true);
  mMediaPackager = mVideoSysInfo->CreateMediaPackager();

  mMpController.reset(new MpController(mVideoPlayer));
  mVideoSysInfo->GetCtrlManager()->RegisterWorker(mMpController);

  mCurState = eInit;
  return true;
}

bool MediaPlayerV::OnPlayLive()
{
  if (!mMpController) {
    return false;
  }
  mMpController->PlayLive();
  return true;
}

bool MediaPlayerV::OnPlayArchive(const QDateTime& ts, int speed, int speedDenum)
{
  if (!mMpController) {
    return false;
  }
  mMpController->PlayArchive(ts, speed, speedDenum);
  return true;
}

bool MediaPlayerV::OnStop()
{
  if (!mMpController) {
    return false;
  }
  mMpController->PlayStop();
  mVideoPlayer->Chat()->Close();
  mCurState = eInit;
  return true;
}

bool MediaPlayerV::OnPause()
{
  if (!mMpController) {
    return false;
  }
  mMpController->PlayStop();
  mCurState = eInit;
  return true;
}

bool MediaPlayerV::OnRelease()
{
  mMpController->Stop();
  mMpController.reset();
  mVideoPlayer.reset();
  mCurState = eNotInit;
  return true;
}

void MediaPlayerV::CreateChater(bool newPlayer)
{
  FrameReceiver<MediaPlayerV>* rcv;
  ReceiverS receiver = ReceiverS(rcv = new FrameReceiver<MediaPlayerV>(this, mCameraId));
  ChaterS chater = Chater::CreateChater(mVideoSysInfo->GetCtrlManager(), Uri::FromString(mVideoSysInfo->GetVideoUri(mCameraId)), receiver);
  if (newPlayer) {
    mVideoPlayer.reset(new CameraPlayer(chater, mCameraId));
  } else {
    mVideoPlayer->SetChat(chater);
  }
  rcv->ConnectPlayer(mVideoPlayer.data());
}


MediaPlayerV::MediaPlayerV(const VideoSysInfoS& _VideoSysInfo)
  : mCameraId(0), mVideoSysInfo(_VideoSysInfo)
  , mCurState(eNotInit), mReqState(eNotInit), mTimestamp(0)
{
}

MediaPlayerV::~MediaPlayerV()
{
}
