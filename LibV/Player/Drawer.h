#pragma once

#include <QRect>
#include <QMap>
#include <qsystemdetection.h>

#include <Lib/Common/FpsCalc.h>
#include <LibV/Include/Frame.h>
#include <LibV/Include/ConveyorV.h>
#include <LibV/Player/PlayerSettings.h>

#include "WndProcA.h"
#include "CameraInfo.h"
#include "Icons.h"


DefineClassS(Drawer);
DefineClassS(DeviceDrawer);
DefineClassS(DevicePlayer);
DefineClassS(Render);
DefineClassS(WndProcA);

enum EDrawerFlags {
  eScaleBest         = 1 << 0,
  eShowMouse         = 1 << 1,
  eAutoHideMouse     = 1 << 2,
  eAlwaysOnTop       = 1 << 3,

  ePlayAudio         = 1 << 8,

  eDrawFlagIllegal
};

enum EMonitorPlace {
  eWindow,
  eFullScreen,
  eHidden,
  eMPlaceIllegal
};

struct SceneInfo {
  int                   Monitor;
  int                   Width;
  int                   Height;
  QRect                 Box;
  int                   Flag;
  EMonitorPlace         Place;
};

class DeviceDrawer
{
  const bool   mScaleBest;
  const char** mStatusIcons;
  WndProcAS    mMainWindow;

protected:
  const bool& ScaleBest() { return mScaleBest; }
  const char* StatusIcon(int i) { return mStatusIcons[i]; }
  WndProcA* Windows() { return mMainWindow.data(); }

public:
  /*new */virtual void SetFrame(FrameS& frame) = 0;
  /*new */virtual void SetStatusFrame(FrameS& frame) = 0;
  /*new */virtual void SetStatus(EDrawStatus status) = 0;
  /*new */virtual void Redraw() = 0;
  /*new */virtual void Clear() = 0;

  /*new */virtual void SetZoom(int scale) = 0;
  /*new */virtual void MoveZoom(const QPointF& delta) = 0;

public:
  DeviceDrawer(WndProcAS& _MainWindow, const char** _StatusIcons, bool _ScaleBest)
    : mScaleBest(_ScaleBest), mStatusIcons(_StatusIcons), mMainWindow(_MainWindow)
  { }
  /*new */virtual ~DeviceDrawer() { }
};

class DevicePlayer
{
public:
  /*new */virtual void SetFrame(FrameS& frame) = 0;

public:
  DevicePlayer() { }
  /*new */virtual ~DevicePlayer() { }
};

class Drawer: public ConveyorV
{
  SceneInfo        mSceneInfo;
  QRect            mScreenRect;
  QRect            mWindowRect;

  WndProcAS        mMainWindow;
  Render*          mRender;
  DeviceDrawerS    mDeviceDrawer;
  DevicePlayerS    mDevicePlayer;

  EDrawStatus      mDrawStatus;
  bool             mPaused;
  bool             mMute;
  FpsCalc          mFpsCalc;

  bool             mRedraw;
  EDrawStatus      mNewStatus;
  QString          mCameraName;
  bool             mChangeCamera;
  int              mZoomScale;
  QPointF          mZoomMove;

  qint64           mDrawFixer;

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "Drawer"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "Dr"; }
protected:
//  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
//  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;

protected:
  /*override */virtual bool ProcessFrame() Q_DECL_OVERRIDE;

public:
  bool CreateScene(Render* _Render, const SceneInfo& _SceneInfo, EStyleType _Style, bool primary);
  bool UpdateSceneBox(const QRect& box);
  bool ResizeSceneBox(const QRect& box);
  void Show(EMonitorPlace place);
  void StatusChanged(EDrawStatus _DrawStatus);
  void CameraChanged(const QString& _CameraName);

public:
  void Mute(bool mute);
  void Pause();
  void Play();
  void Redraw();

  void AddZoom(int delta);
  void MoveZoom(const QPointF& delta);

private:
  void FixZoomPos();

  bool HasFlag(int flag);

  void ProcessVideoFrame();
  void SyncNextFrame();

  void ProcessStatusFrame();

public:
  Drawer();
};

