#pragma once

#include <QMutex>
#include <QWaitCondition>
#include <QMap>

#include <Lib/Ctrl/CtrlWorker.h>
#include <Lib/Db/Db.h>
#include <Lib/Net/Receiver.h>
#include <LibV/Include/VideoMsg.h>


DefineClassS(RtspInit);
DefineClassS(RtspServer);
DefineClassS(MediaPlayerManager);
DefineClassS(RtspCamReceiver);
DefineClassS(VideoSysInfo);

class RtspInit: public CtrlWorker
{
  VideoSysInfoS           mVideoSysInfo;
  RtspServerS             mRtspServer;
  MediaPlayerManagerS     mMpManager;

  QMutex                  mMutex;
  QWaitCondition          mWaitEnd;
  QMap<int, ObjectItemS>  mQueryList;
  QMap<int, ChaterS>      mChaters;
  bool                    mInit;

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "Rstp media init"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "I"; }
protected:
//  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;
public:
//  /*override */virtual void Stop() Q_DECL_OVERRIDE;

public: /*internal*/
  void GetSpsPps(const ObjectItemS& item, const char* data, int size);
  void GetFail(const ObjectItemS& item);
  void RemoveCamera(const ObjectItemS& item);

private:
  bool InitCameras();
  void InitOneCamera(const ObjectItemS& item);
  bool QueryCameras();

public:
  explicit RtspInit(const VideoSysInfoS& _VideoSysInfo, const RtspServerS& _RtspServer, const MediaPlayerManagerS& _MpManager);
  /*override */virtual ~RtspInit();
};
