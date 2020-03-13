#pragma once

#include <QSharedMemory>
#include <QMutex>
#include <QDateTime>

#include <Lib/Db/Db.h>
#include <Lib/Common/Uri.h>
#include <Lib/Net/NetMessage.h>
#include <LibV/Include/Frame.h>
#include <LibV/Include/ConveyorV.h>
#include <LibV/Player/ArmState.h>

#include "Drawer.h"


DefineClassS(Render);
DefineClassS(Chater);
DefineClassS(CameraPlayer);
DefineClassS(Drawer);
DefineClassS(Decoder);
DefineClassS(PlayerSettings);

class Render: public ConveyorV
{
  enum EType {
    eDb,
    eCustom,
    eShmem,
  };

  enum EState {
    eNone,
    eSwitchCamera,
    ePlayCamera,
    eStateIllegal
  };

  Db&                   mDb;
  ObjectTableS          mObjectTable;

  DrawerS               mDrawer;
  EType                 mType;
  int                   mArmId;
  int                   mCustomCamera;
  qint64                mTimestamp;
  bool                  mPrime;
  PlayerSettingsS       mPlayerSettings;

  EState                mState;
  bool                  mPlayLive;
  bool                  mPlayPause;

  SceneInfo             mScene;
  QMap<int, CameraInfo> mCameras;
  QMutex                mPlayerMutex;
  CameraPlayerS         mCurrentPlayer;
  CameraPlayerS         mNextPlayer;
  QList<CameraPlayerS>  mPlayerTrash;

  QSharedMemory         mStateShmem;
  ArmState              mArmState;
  volatile int          mCameraCounter;

  QSharedMemory         mMonitorShmem;
  MonitorState*         mMonitorState;

public:
  /*new */virtual const char* Name() { return "Render"; }
  /*new */virtual const char* ShortName() { return "R"; }
protected:
  /*new */virtual bool DoInit();
  /*new */virtual bool DoCircle();
  /*new */virtual void DoRelease();
public:
//  /*new */virtual void Stop();

public:
  bool IsPrime() { return mPrime; }
  const PlayerSettings* GetSettings() const { return mPlayerSettings.data(); }

public:
  void AttachToShmem(int shmemIndex);
  bool InitSingleScene(int camId, int armId);
  bool InitCustomScene(int id, int type, int count, int monitor, int camId, int armId, const qint64& ts);
  bool InitUserCamera();
private:
  void InitTable(int count, int id);
  void InitCorner(int count, int id);
  void InitHorz(int count, int id);
  void InitVert(int count, int id);

private:
  bool DoUserCamera();

  void DisconnectCurrentCamera();
  void ConnectNextCamera();
  void ConfirmCamera();

  void SwitchToNextCamera();
  void PlayNextFrame(FrameS &frame);
  void ClearCurrentPlayer(bool andNext = false);

private:
  bool LoadScene();
  bool LoadTrueScene();
  bool LoadCustomScene();
  bool LoadCustomCamera();
  bool LoadShmemScene();
  void SetSceneFlags();
  void UpdateMonitorState();
  bool ReadShmem();
  void UpdateShmemScene();
  void UpdateShmemCameras();
  void ConnectCamera(CameraInfo& info);

public: /*internal*/
  void VideoGranted(CameraPlayer* player);
  void VideoDenied(CameraPlayer* player);
  void VideoNoStorage(CameraPlayer* player);
  void VideoDropped(CameraPlayer* player);
  void VideoAborted(CameraPlayer* player);
  void VideoInfo(CameraPlayer* player, const qint64& timestamp);
  void VideoFrame(CameraPlayer* player, FrameS& frame);
  void Disconnected(CameraPlayer* player);

public: /*internal*/
  void OnMute(bool mute);
  void OnPlay();
  void OnPause();
  void OnSwitchLive();
  void OnSwitchArchive(const QDateTime& timestamp, int speedNum, int speedDenum);
  void OnDownload(const QDateTime& tsFrom, const QDateTime& tsTo);
  void OnScreenPlaceChange();

private:
  bool OnReceiveNew(CameraPlayer* player);

public:
  Render(Db& _Db, DrawerS& _Drawer);
  virtual /*new */~Render();
};
