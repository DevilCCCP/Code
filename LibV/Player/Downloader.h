#pragma once

#include <QSharedPointer>
#include <QElapsedTimer>
#include <QDateTime>

#include <Lib/Db/Db.h>
#include <Lib/Common/Uri.h>
#include <Lib/Net/NetMessage.h>
#include <LibV/Include/Frame.h>
#include <LibV/Include/ConveyorV.h>

#include "CameraInfo.h"


DefineClassS(Downloader);
DefineClassS(FileSaver);
DefineClassS(Chater);
DefineClassS(CameraPlayer);
DefineClassS(Drawer);
DefineClassS(Decoder);

class DownPercent {
public:
  /*new */virtual void OnPercent(const qint64& taskId, int percent) = 0;
};
typedef DownPercent* DownPercentPtr;

class Downloader: public ConveyorV
{
public:
  enum EState {
    eNone,
    eSeekCamera,
    eConnectCamera,
    ePlayCamera,
    eReconnectCamera,
    eDelayedReconnectCamera,
    eWaitReconnectCamera,
    eDone,
    eFail,
  };

private:
  DbS                   mDb;
  const int             mCameraId;
  const int             mRequestSpeed;
  const QString         mSavePath;
  const bool            mFullname;
  const bool            mExitOnDone;

  PROPERTY_GET_SET(DownPercentPtr, DownPercent)
  PROPERTY_GET_SET(qint64,         TaskId)

  EState                mState;
  qint64                mStartTime;
  qint64                mEndTime;
  qint64                mCurrentTime;
  int                   mCurrentPercent;
  QElapsedTimer         mReconnectTimer;
  bool                  mConnecting;
  int                   mTooOldFrames;

  CameraInfo            mCameraInfo;
  CameraPlayerS         mCameraPlayer;
  FileSaverS            mCurrentSaver;
  bool                  mCloseSaver;

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "Downloader"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "D"; }
protected:
//  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;
public:
//  /*override */virtual void Stop() Q_DECL_OVERRIDE;

protected:
//  /*override */virtual void OnOverflowWarn() Q_DECL_OVERRIDE;

public:
  int GetPercent();

private:
  bool LoadCameraInfo();
  bool ConnectCamera();
  void DisconnectCamera();
  void ConfirmCamera();
  bool ReconnectCamera();
  void DelayedReconnect();

  QString GenerateFilename();

  void ProcessOverflow();

public: /*internal*/
  void VideoGranted(CameraPlayer* player);
  void VideoDenied(CameraPlayer* player);
  void VideoNoStorage(CameraPlayer* player);
  void VideoDropped(CameraPlayer* player);
  void VideoAborted(CameraPlayer* player);
  void VideoInfo(CameraPlayer* player, const qint64& timestamp);
  void VideoFrame(CameraPlayer* player, FrameS& frame);
  void Disconnected(CameraPlayer* player);

private:
  void DoReconnect();

public:
  Downloader(const DbS& _Db, const int _CameraId, const qint64& _StartTime, const qint64& _EndTime
             , const int& _RequestSpeed, const QString& _SavePath, bool _Fullname, bool _ExitOnDone);
  virtual /*override */~Downloader();
};
