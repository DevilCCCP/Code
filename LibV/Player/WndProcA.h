#pragma once

#include <QElapsedTimer>
#include <QDateTime>
#include <QMutex>
#include <QRect>

#include <Lib/Ctrl/CtrlWorker.h>
#include <LibV/Include/Frame.h>
#include <LibV/Player/PlayerSettings.h>

#include "Icons.h"


DefineClassS(WndProcA);
DefineClassS(Render);
DefineClassS(Drawer);
DefineClassS(Overseer);
typedef Render* RenderPtr;
typedef Drawer* DrawerPtr;

class WndProcA: public CtrlWorker
{
  PROPERTY_GET(int,        Id)
  PROPERTY_GET(bool,       Single)
  PROPERTY_GET(bool,       Embedded)
  PROPERTY_GET(RenderPtr,  Render)
  PROPERTY_GET(DrawerPtr,  Drawer)
  PROPERTY_GET(bool,       PrimeWindow)
  PROPERTY_GET(bool,       ShowMouse)
  PROPERTY_GET(bool,       AutoHideMouse)
  PROPERTY_GET(bool,       AlwaysOnTop)
  PROPERTY_GET(bool,       PlaySound)
  PROPERTY_GET(EStyleType, Style)
  PROPERTY_GET(QRect,      SceneRect)

  enum EChangeState {
    eNoneState   = 0,
    eTimeChanged = 1 << 0,
    eInfoChanged = 1 << 1,
    eBoxChanged  = 1 << 2,
    eFpsChanged  = 1 << 3,
  };
  QMutex           mChangeMutex;
  int              mChangeState;
  QDateTime        mChangeTime;
  qreal            mChangeFps;
  QString          mChangeInfo;
  QRect            mChangeBox;

protected:
  void SetSize(const QSize& size) { mSceneRect.setSize(size); }

public:
  void UpdateTime(const QDateTime& timestamp);
  void UpdateFps(const qreal& fps);
  void UpdateInfo(const QString& cameraName);
  void UpdateBox(const QRect& box);

public:
  /*override */virtual const char* Name() override { return "Wnd proc"; }
  /*override */virtual const char* ShortName() override { return "W"; }
protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
  /*override */virtual void DoRelease() override;

public:
  void ShowRect(const QRect& rect);
  /*new */virtual void Show() = 0;
  /*new */virtual void Hide() = 0;

protected:
  /*new */virtual void ConnectBackWnd() = 0;
  /*new */virtual void ProcessMsgQueue() = 0;

  /*new */virtual void OnChangedTime(const QDateTime& changeTime);
  /*new */virtual void OnChangedFps(const qreal& changeFps);
  /*new */virtual void OnChangedInfo(const QString& changeInfo);
  /*new */virtual void OnChangedBox();

private:
  void ProcessChanges();

public:
  explicit WndProcA(Overseer* _Overseer, Render* _Render, Drawer* _Drawer, const QRect& _SceneRect
                   , bool _PrimeWindow, bool _ShowMouse, bool _AutoHideMouse, bool _AlwaysOnTop, bool _PlaySound, EStyleType _Style);
  /*new */virtual ~WndProcA();
};
