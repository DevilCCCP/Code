#include <QMutexLocker>
#include <qsystemdetection.h>

#include <Lib/Log/Log.h>
#include <Lib/Common/Format.h>
#include <Lib/Ctrl/CtrlManager.h>
#include <Lib/Dispatcher/Overseer.h>
#include <LibV/Player/PlayerSettings.h>

#include "Drawer.h"
#include "Render.h"
#include "Sdl/WndProcL.h"
#include "Sdl/SdlDrawer.h"
#ifdef Q_OS_WIN32
#include "Win/WndProc.h"
#include "Win/DdrawDrawer.h"
#endif
#include "Sdl/SdlPlayer.h"


#ifdef Q_OS_WIN32
const bool gUseSdl = false;
#else
const bool gUseSdl = true;
#endif
const int kMaxSyncSleepMs = 1000;
const int kZoomMax = 8;
const int kZoomScales[kZoomMax] = { 1, 2, 3, 4, 6, 8, 16, 24 };
typedef const char* ConstString;
ConstString kStatusIcons[eDrawStatusIllegal + 1] = { ":/Icons/StatusGray.png"
                                                     , ":/Icons/StatusYellow.png"
                                                     , ":/Icons/StatusGreen.png"
                                                     , ":/Icons/StatusRed.png"
                                                     , nullptr
                                                   };


bool Drawer::DoCircle()
{
  if (mRedraw) {
    QMutexLocker lock(&Mutex());
    mRedraw = false;
    int zoomScale = mZoomScale;
    QPointF zoomMove = mZoomMove;
    lock.unlock();
    mDeviceDrawer->SetZoom(kZoomScales[zoomScale]);
    mDeviceDrawer->MoveZoom(zoomMove);
    mDeviceDrawer->Redraw();
    return true;
  } else if (mNewStatus != mDrawStatus) {
    mDrawStatus = mNewStatus;
    mDeviceDrawer->SetStatus(mDrawStatus);
    return true;
  } else if (mChangeCamera) {
    QMutexLocker lock(&Mutex());
    QString cameraName = QString("%1").arg(mCameraName);
    mChangeCamera = false;
    lock.unlock();
    mMainWindow->UpdateInfo(cameraName);
    mDeviceDrawer->Clear();
    return true;
  } else {
    return Conveyor::DoCircle();
  }
}

bool Drawer::ProcessFrame()
{
  if (!mPaused) {
    if (CurrentVFrame()->GetHeader()->HeaderSize == sizeof(Frame::Header)) {
      ProcessVideoFrame();
    } else if (CurrentVFrame()->GetHeader()->HeaderSize == sizeof(Frame::StatusHeader)) {
      ProcessStatusFrame();
    }
  }
  return true;
}

bool Drawer::CreateScene(Render* _Render, const SceneInfo& _SceneInfo, EStyleType _Style, bool primary)
{
  mRender = _Render;
  mSceneInfo = _SceneInfo;

  Log.Info(QString("Create draw (box: (%1, %2, %3, %4), prim: %5)")
           .arg(mSceneInfo.Box.left()).arg(mSceneInfo.Box.top())
           .arg(mSceneInfo.Box.width()).arg(mSceneInfo.Box.height()).arg(primary));

  if (mSceneInfo.Width <= 0 || mSceneInfo.Height <= 0 || mSceneInfo.Monitor < 0) {
    if (mSceneInfo.Box.isEmpty()) {
      return false;
    }
    if (gUseSdl) {
      mMainWindow = WndProcAS(new WndProcL(GetOverseer(), mRender, this, mSceneInfo.Box
                                         , false, HasFlag(eShowMouse), HasFlag(eAutoHideMouse)
                                         , HasFlag(eAlwaysOnTop), HasFlag(ePlayAudio), _Style));
      GetManager()->RegisterWorker(mMainWindow);
      mDeviceDrawer = DeviceDrawerS(new QtDrawer(mMainWindow, kStatusIcons, HasFlag(eScaleBest)));
    }
#ifdef Q_OS_WIN32
    else {
      mMainWindow = WndProcAS(new WndProc(GetOverseer(), mRender, this, mSceneInfo.Box
                                         , false, HasFlag(eShowMouse), HasFlag(eAutoHideMouse)
                                         , HasFlag(eAlwaysOnTop), HasFlag(ePlayAudio), _Style));
      GetManager()->RegisterWorker(mMainWindow);
      mDeviceDrawer = DeviceDrawerS(new DdrawDrawer(mMainWindow, kStatusIcons, HasFlag(eScaleBest)));
    }
#endif
    if (HasFlag(ePlayAudio)) {
      mDevicePlayer = DevicePlayerS(new SdlPlayer(16000, 1));
    }
    return true;
  }

  int monitorNumber = (mSceneInfo.Monitor > 0)? mSceneInfo.Monitor - 1: qApp->desktop()->primaryScreen();
  int monitorMapNumber = mRender->GetSettings()->getMonitorMap().value(monitorNumber, 0);
  if (monitorMapNumber > 0) {
    Log.Info(QString("Using remapped monitor %1 -> %2").arg(monitorNumber + 1).arg(monitorMapNumber));
    monitorNumber = monitorMapNumber - 1;
  }
  if (monitorNumber < qApp->desktop()->screenCount()) {
    //qApp->desktop()->repaint();
    mScreenRect = qApp->desktop()->screenGeometry(monitorNumber);
    int left = mScreenRect.left();
    int top = mScreenRect.top();
    int width = mScreenRect.width();
    int height = mScreenRect.height();
    QRect box = mSceneInfo.Box;
    mWindowRect.setLeft(left + width * box.left() / mSceneInfo.Width);
    mWindowRect.setRight(left + width * (box.right() + 1) / mSceneInfo.Width - 1);
    mWindowRect.setTop(top + height * box.top() / mSceneInfo.Height);
    mWindowRect.setBottom(top + height * (box.bottom() + 1) / mSceneInfo.Height - 1);
    Log.Info(QString("Create scene [(%1, %2),(%3, %4)] (%5, %6, %7, %8)")
      .arg(box.left()).arg(box.top()).arg(box.right()).arg(box.bottom())
      .arg(mWindowRect.left()).arg(mWindowRect.top()).arg(mWindowRect.right()).arg(mWindowRect.bottom()));
    bool primeMonitor = (primary)? true: monitorNumber == qApp->desktop()->primaryScreen();
    if (gUseSdl) {
      mMainWindow = WndProcAS(new WndProcL(GetOverseer(), mRender, this, mWindowRect
                                           , primeMonitor, HasFlag(eShowMouse), HasFlag(eAutoHideMouse)
                                           , HasFlag(eAlwaysOnTop), HasFlag(ePlayAudio), _Style));
      GetManager()->RegisterWorker(mMainWindow);
      mDeviceDrawer = DeviceDrawerS(new QtDrawer(mMainWindow, kStatusIcons, HasFlag(eScaleBest)));
    }
#ifdef Q_OS_WIN32
    else {
      mMainWindow = WndProcAS(new WndProc(GetOverseer(), mRender, this, mWindowRect
                                          , primeMonitor, HasFlag(eShowMouse), HasFlag(eAutoHideMouse)
                                          , HasFlag(eAlwaysOnTop), HasFlag(ePlayAudio), _Style));
      GetManager()->RegisterWorker(mMainWindow);
      mDeviceDrawer = DeviceDrawerS(new DdrawDrawer(mMainWindow, kStatusIcons, HasFlag(eScaleBest)));
    }
#endif
    if (HasFlag(ePlayAudio)) {
      mDevicePlayer = DevicePlayerS(new SdlPlayer(16000, 1));
    }
    Show(mSceneInfo.Place);
    return true;
  } else {
    return false;
  }
}

bool Drawer::UpdateSceneBox(const QRect& box)
{
  mSceneInfo.Box = box;
  if (!mMainWindow) {
    return false;
  }
  mMainWindow->UpdateBox(mSceneInfo.Box);

  Log.Info(QString("Update draw (box: (%1, %2, %3, %4))")
           .arg(mSceneInfo.Box.left()).arg(mSceneInfo.Box.top())
           .arg(mSceneInfo.Box.width()).arg(mSceneInfo.Box.height()));
  return true;
}

void Drawer::Show(EMonitorPlace place)
{
  switch (place) {
  case eWindow:        mMainWindow->UpdateBox(mWindowRect); break;
  case eFullScreen:    mMainWindow->UpdateBox(mScreenRect); break;
  case eHidden:
  case eMPlaceIllegal: mMainWindow->UpdateBox(QRect()); break;
  }
}

void Drawer::StatusChanged(EDrawStatus _DrawStatus)
{
  QMutexLocker lock(&Mutex());
  if (mNewStatus != _DrawStatus) {
    mNewStatus = _DrawStatus;
    WakeUp();
  }
}

void Drawer::CameraChanged(const QString &_CameraName)
{
  QMutexLocker lock(&Mutex());
  mCameraName = _CameraName;
  mChangeCamera = true;
  WakeUp();
}

void Drawer::Mute(bool mute)
{
  mMute = mute;
}

void Drawer::Pause()
{
  mPaused = true;
}

void Drawer::Play()
{
  mPaused = false;
}

void Drawer::Redraw()
{
  QMutexLocker lock(&Mutex());
  mRedraw = true;
  WakeUp();
}

void Drawer::AddZoom(int delta)
{
  QMutexLocker lock(&Mutex());
  mZoomScale = qBound(0, mZoomScale + delta, kZoomMax);
  FixZoomPos();
  mRedraw = true;
  WakeUp();
}

void Drawer::MoveZoom(const QPointF& delta)
{
  QMutexLocker lock(&Mutex());
  mZoomMove += delta;
  FixZoomPos();
  mRedraw = true;
  WakeUp();
}

void Drawer::FixZoomPos()
{
  int scale = kZoomScales[mZoomScale];
  qreal zMax = (scale - 1) * 0.5;
  mZoomMove.rx() = qBound(-zMax, mZoomMove.x(), zMax);
  mZoomMove.ry() = qBound(-zMax, mZoomMove.y(), zMax);
}

bool Drawer::HasFlag(int flag)
{
  return (mSceneInfo.Flag & flag) != 0;
}

void Drawer::ProcessVideoFrame()
{
  SyncNextFrame();
  FrameS currentFrame = CurrentVFrame();
  const Frame::Header* header = currentFrame->GetHeader();
  if (header->Compression) {
    mDeviceDrawer->SetFrame(currentFrame);
    mMainWindow->UpdateTime(QDateTime::fromMSecsSinceEpoch(header->Timestamp));
    mFpsCalc.AddFrame();
    mMainWindow->UpdateFps(mFpsCalc.GetFps());
  }
  if (HasFlag(ePlayAudio) && !mMute && header->CompressionAudio) {
    mDevicePlayer->SetFrame(currentFrame);
  }
}

void Drawer::SyncNextFrame()
{
  qint64 frameTs = CurrentVFrame()->Timestamp() + mDrawFixer;
  qint64 drawTs = QDateTime::currentMSecsSinceEpoch();
  if (drawTs >= frameTs) { // too late
    qint64 delta = drawTs - frameTs;
    mDrawFixer += delta;
    Log.Info(QString("Draw too late, change fixer (fx: %1, delta: %2)").arg(mDrawFixer).arg(FormatTimeDelta(delta)));
  } else if (frameTs - drawTs > kMaxSyncSleepMs) {
    qint64 delta = drawTs - frameTs;
    if (mDrawFixer) {
      mDrawFixer += delta;
      Log.Info(QString("Draw too fast, change fixer (fx: %1, delta: %2)").arg(mDrawFixer).arg(FormatTimeDelta(delta)));
    } else {
      mDrawFixer = delta;
      Log.Info(QString("Set draw fixer (fx: %1 (%2))").arg(FormatTimeDelta(mDrawFixer)).arg(QDateTime::fromMSecsSinceEpoch(mDrawFixer).toString()));
    }
  } else {
    Sleep(frameTs - drawTs);
  }
}

void Drawer::ProcessStatusFrame()
{
  FrameS currentFrame = CurrentVFrame();
  mDeviceDrawer->SetStatusFrame(currentFrame);
  mFpsCalc.UpdateFrame();
  mMainWindow->UpdateFps(mFpsCalc.GetFps());
}


Drawer::Drawer()
  : ConveyorV(10)
  , mRender(nullptr)
  , mDrawStatus(eDrawStatusIllegal), mPaused(false), mMute(true)
  , mRedraw(false), mNewStatus(eDrawStatusIllegal), mChangeCamera(false), mZoomScale(0), mZoomMove(0, 0)
  , mDrawFixer(0)
{
  Q_INIT_RESOURCE(Player);
}
