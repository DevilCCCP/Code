#pragma once

#include <QElapsedTimer>
#include <QRect>

#include <Lib/Ctrl/CtrlWorker.h>
#include <LibV/Include/Frame.h>
#include <LibV/VideoUi/ImageWithPoints.h>

#include "WndProcA.h"
#include "Icons.h"


DefineClassS(WndProcL);
DefineClassS(Render);
DefineClassS(Drawer);
DefineClassS(Overseer);
DefineClassS(ImageWithPoints);
DefineClassS(QEventLoop);
struct SDL_Window;

class WndProcL: public WndProcA
{
//  ImageWithPointsS  mMainWindow;
//  QEventLoopS       mEventLoop;
  SDL_Window*         mMainWindow;
  bool                mHidden;

public:
  /*override */virtual const char* Name() override { return "Wnd proc Qt"; }
  /*override */virtual const char* ShortName() override { return "W"; }
protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual void DoRelease() override;

public:
  /*override */virtual void Show();
  /*override */virtual void Hide();

protected:
  /*override */virtual void ConnectBackWnd();
  /*override */virtual void ProcessMsgQueue();

  /*override */virtual void OnChangedTime(const QDateTime& changeTime);
  /*override */virtual void OnChangedFps(const qreal& changeFps);
  /*override */virtual void OnChangedInfo(const QString& changeInfo);
  /*override */virtual void OnChangedBox();

public:
  SDL_Window* VideoWindow() { return mMainWindow; }

private:

public:
  explicit WndProcL(Overseer* _Overseer, Render* _Render, Drawer* _Drawer, const QRect& _SceneRect
                   , bool _PrimeWindow, bool _ShowMouse, bool _AutoHideMouse, bool _AlwaysOnTop, bool _PlaySound, EStyleType _Style);
  /*new */virtual ~WndProcL();
};
