#pragma once

#include <QMutex>
#include <QWaitCondition>

#include <LibV/Include/Frame.h>

#include "MediaPlayer.h"


DefineClassS(MediaPlayerV);
DefineClassS(MpController);
DefineClassS(CtrlManager);
DefineClassS(CameraPlayer);
DefineClassS(MediaPackager);
DefineClassS(VideoSysInfo);
DefineClassS(RtspInfo);
DefineClassS(Chater);

/*abstract*/class MediaPlayerV: public MediaPlayer
{
  enum EState {
    eNotInit,
    eInit,
    ePlay
  };

  int            mCameraId;
  MpControllerS  mMpController;
  VideoSysInfoS  mVideoSysInfo;
  CameraPlayerS  mVideoPlayer;
  MediaPackagerS mMediaPackager;

  EState         mCurState;
  EState         mReqState;

  QMutex         mPlayMutex;
  QWaitCondition mPlayWait;
  bool           mPlayRespond;
  bool           mPlayOk;
  qint64         mTimestamp;

protected:
  int CameraId() { return mCameraId; }
  const MediaPackagerS& GetMediaPackager() { return mMediaPackager; }
  bool PlayOk() { return mPlayOk; }
  const qint64&  Timestamp() { return mTimestamp; }

protected:
  /*override */virtual bool Open() Q_DECL_OVERRIDE;
public:
//  /*override */virtual bool Play(const ParamsMap& params, QByteArray& extraData) Q_DECL_OVERRIDE;
  /*override */virtual bool Pause() Q_DECL_OVERRIDE;
  /*override */virtual void Stop() Q_DECL_OVERRIDE;
  /*override */virtual void Release() Q_DECL_OVERRIDE;

protected:
  /*new */virtual bool OnVideoInfo(const qint64& timestamp);
  /*new */virtual bool OnVideoFrame(const Frame::Header* header, qint64& timestamp);

public: /*internal*/
  void VideoGranted(CameraPlayer* player);
  void VideoDenied(CameraPlayer* player);
  void VideoNoStorage(CameraPlayer* player);
  void VideoDropped(CameraPlayer* player);
  void VideoAborted(CameraPlayer* player);
  void VideoInfo(CameraPlayer* player, const qint64& timestamp);
  void VideoFrame(CameraPlayer* player, FrameS& frame);
  void Disconnected(CameraPlayer* player);

protected:
  bool PlayLive();
  bool PlayArchive(const QDateTime& ts, int speed, int speedDenum);

  bool WaitConnect(int timeoutMs, int warnTimeoutMs = 2000);
  bool WaitFrame(int timeoutMs, int warnTimeoutMs = 2000);

private:
  void OnInitPlay();
  bool OnInit();
  bool OnPlayLive();
  bool OnPlayArchive(const QDateTime& ts, int speed, int speedDenum);
  bool OnStop();
  bool OnPause();
  bool OnRelease();

  void CreateChater(bool newPlayer);

public:
  explicit MediaPlayerV(const VideoSysInfoS& _VideoSysInfo);
  /*override */virtual ~MediaPlayerV();
};
