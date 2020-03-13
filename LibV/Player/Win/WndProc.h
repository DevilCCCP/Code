#pragma once

#include <QElapsedTimer>
#include <QDateTime>
#include <QMutex>
#include <QRect>

#include <Lib/Ctrl/CtrlWorker.h>
#include <LibV/Include/Frame.h>
#include <LibV/Player/PlayerSettings.h>

#include "../WndProcA.h"
#include "../Icons.h"


DefineClassS(WndProc);
DefineClassS(Render);
DefineClassS(Drawer);
DefineClassS(ToolsWnd);
DefineClassS(Tools2Wnd);
DefineClassS(Overseer);
DefineClassS(GdiplusEngine);

class WndProc: public WndProcA
{
  HWND             mMainWindow;
  HWND             mDrawWindow;
  ToolsWndS        mToolsWnd;
  bool             mToolsMode;
  QElapsedTimer    mToolsEnds;

  HWND             mControlWindow;
  QElapsedTimer    mControlWait;
  bool             mControlConnected;

  GdiplusEngineS   mGdiplusEngine;

public:
  HWND GetDrawWnd() { return mDrawWindow; }

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

private:/*internal*/
  void ShowTools();
  void HideTools();
  void DoubleClick();
  void MouseMove(int key, const QPoint& p);
  void MouseWheel(int key, int delta);
  void ResendMsg(UINT msg, WPARAM wparam, LPARAM lparam);

private:
  void UpdateTools();
  void AutoHideTools();

private:/*internal*/
  void Confirm();

  void ShowMainTools(const QPoint& place);
  void Resize(int width, int height);
  void RedrawScene();

public:
  explicit WndProc(Overseer* _Overseer, Render* _Render, Drawer* _Drawer, const QRect& _SceneRect
                   , bool _PrimeWindow, bool _ShowMouse, bool _AutoHideMouse, bool _AlwaysOnTop, bool _PlaySound, EStyleType _Style);
  /*new */virtual ~WndProc();

  friend LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  friend LRESULT CALLBACK DrawWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};
