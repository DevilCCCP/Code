#include <Windows.h>
#include <Windowsx.h>
#include <Commctrl.h>
#include <QMutexLocker>

#include <Lib/Log/Log.h>
#include <Lib/Dispatcher/Overseer.h>
#include <LibV/Include/ModuleNames.h>

#include "WndProc.h"
#include "CtrlWndDef.h"
#include "ToolsWnd.h"
#include "../Drawer.h"
#include "../Render.h"
#include "GdiplusEngine.h"


#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

const int kHideToolsMs = 3000;
const int kHideTools2Ms = 500;
const int kConnectControlWarnMs = 1000;
const int kConnectControlDeadMs = 5000;

const int kGestureThreshold = 50;
const int kGestureDirectionThreshold = 20;

static const char gClassNameDraw[] = "VideoDraw";

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static WndProc* gWndProc = nullptr;

  switch (msg) {
  case WM_CREATE:
    gWndProc = (WndProc*)((CREATESTRUCT*)lparam)->lpCreateParams;
    break;

  case WM_ACTIVATE:
  case WM_SETFOCUS:
    gWndProc->ResendMsg(msg, wparam, lparam);
    return 0;

  case WM_KEYDOWN:
    if (gWndProc) {
      gWndProc->ShowTools();
      gWndProc->ShowMainTools(QPoint(0, -1));
    }
    break;

  case WM_MOUSEWHEEL:
    if (gWndProc) {
      gWndProc->MouseWheel(GET_KEYSTATE_WPARAM(wparam), GET_WHEEL_DELTA_WPARAM(wparam) / 120);
    }
    return 0;

  case WM_SIZE:
    if (gWndProc) {
      gWndProc->Resize(LOWORD(lparam), HIWORD(lparam));
      gWndProc->RedrawScene();
    }
    break;

  case WM_PAINT:
    if (gWndProc) {
      gWndProc->RedrawScene();
    }
    break;

  case kMsgSetPlayerState:
    if (gWndProc) {
      if (wparam != 0 || lparam != 0) {
        QRect rect(QPoint((short)LOWORD(wparam), (short)HIWORD(wparam))
                   , QPoint((short)LOWORD(lparam), (short)HIWORD(lparam)));
        gWndProc->ShowRect(rect);
      } else {
        gWndProc->Hide();
      }
    }
    break;

  case kMsgRegisterConfirm:
    if (gWndProc) {
      gWndProc->Confirm();
    }
    break;

  case WM_CLOSE:
    return 0;
  }
  return DefWindowProcA(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK DrawWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static WndProc* gWndProc = nullptr;
  static bool mDown = false;
  static QPoint mDownPos(0, 0);
  static int mGesture = 0;

  switch (msg) {
  case WM_LBUTTONDBLCLK:
    if (gWndProc) {
      gWndProc->DoubleClick();
    }
    return 0;

  case WM_LBUTTONDOWN:
    mGesture = 0;
    mDown = true;
    mDownPos.setX(GET_X_LPARAM(lparam));
    mDownPos.setY(GET_Y_LPARAM(lparam));
    return 0;

  case WM_LBUTTONUP:
    mDown = false;
    mGesture = 0;
    return 0;

  case WM_PAINT:
    if (gWndProc) {
      gWndProc->RedrawScene();
    }
    break;

  case WM_MOUSEMOVE:
    if (gWndProc) {
      static WPARAM wparamLast = -1;
      static int xposLast = -1;
      static int yposLast = -1;

      int xpos = GET_X_LPARAM(lparam);
      int ypos = GET_Y_LPARAM(lparam);
      if (xpos != xposLast || ypos != yposLast || wparam != wparamLast) {
        gWndProc->MouseMove(wparam, QPoint(xpos - xposLast, ypos - yposLast));
        xposLast = xpos;
        yposLast = ypos;
        wparamLast = wparam;
        if (wparamLast != (WPARAM)-1) {
          gWndProc->ShowTools();
        }
      }
    }
    if (mDown && mGesture == 0) {
      QPoint pos(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
      int dx = qAbs(pos.x() - mDownPos.x());
      int dy = qAbs(pos.y() - mDownPos.y());
      if (dx > kGestureThreshold || dy > kGestureThreshold) {
        mGesture = 1;
        QPoint place;
        if (dx > kGestureDirectionThreshold) {
          if (dx > dy) {
            place.setX((pos.x() - mDownPos.x() > 0)? 2: -2);
          } else {
            place.setX((pos.x() - mDownPos.x() > 0)? 1: -1);
          }
        } else {
          place.setX(0);
        }
        if (dy > kGestureDirectionThreshold) {
          if (dy > dx) {
            place.setY((pos.y() - mDownPos.y() > 0)? 2: -2);
          } else {
            place.setY((pos.y() - mDownPos.y() > 0)? 1: -1);
          }
        } else {
          place.setY(0);
        }
        gWndProc->ShowMainTools(place);
      }
    }
    break;

  case WM_CREATE:
    gWndProc = (WndProc*)((CREATESTRUCT*)lparam)->lpCreateParams;
    break;
  }
  return DefWindowProcA(hwnd, msg, wparam, lparam);
}

bool WndProc::DoInit()
{
  InitCommonControls();

  mGdiplusEngine = GdiplusEngineS(new GdiplusEngine());

  WNDCLASSEXA wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  QByteArray classNameRaw = GetPlayerWndClassName(getId()).toLatin1();
  const char* className = classNameRaw.constData();
  wcex.style         = CS_DBLCLKS;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = NULL;
  wcex.hIcon         = NULL;
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = NULL;
  wcex.lpszMenuName	 = NULL;
  wcex.hIconSm       = NULL;

  wcex.lpfnWndProc   = MainWndProc;
  wcex.lpszClassName = className;
  if (!RegisterClassExA(&wcex)) {
    Log.Error("RegisterClassExA main fail");
    return false;
  }

  wcex.lpfnWndProc   = DrawWndProc;
  wcex.lpszClassName = gClassNameDraw;
  if (!RegisterClassExA(&wcex)) {
    Log.Error("RegisterClassExA draw fail");
    return false;
  }

  mMainWindow = CreateWindowExA(WS_EX_TOOLWINDOW
#ifdef QT_NO_DEBUG
                                | WS_EX_TOPMOST
#endif
                                , className, "Video frame", WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS
                                   , getSceneRect().left(), getSceneRect().top(), getSceneRect().width(), getSceneRect().height()
                                   , nullptr, nullptr, nullptr, this);
  if (!mMainWindow) {
    Log.Error("CreateWindowExA fail");
    return false;
  }
  mDrawWindow = CreateWindowA(gClassNameDraw, "Draw frame", WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS
                                   , 0, 0, getSceneRect().width(), getSceneRect().height()
                                   , mMainWindow, nullptr, nullptr, this);
  if (!mDrawWindow) {
    Log.Error("CreateWindowA fail");
    return false;
  }

  mToolsWnd = ToolsWndS(new ToolsWnd(getRender(), mMainWindow, getPlaySound(), getStyle()));
  mToolsWnd->SetWidth(getSceneRect().width());
  MoveWindow(mToolsWnd->Hwnd(), 0, getSceneRect().height() - mToolsWnd->Height(), getSceneRect().width(), mToolsWnd->Height(), true);

  ShowWindow(mDrawWindow, SW_SHOW);
  ShowWindow(mToolsWnd->Hwnd(), SW_SHOW);
//  if (mSingle) {
//    ShowWindow(mMainWindow, SW_SHOW);
//  }
  return WndProcA::DoInit();
}

void WndProc::DoRelease()
{
  WndProcA::DoRelease();

  mToolsWnd.clear();
  DestroyWindow(mDrawWindow);
  DestroyWindow(mMainWindow);
  mGdiplusEngine.clear();
}

void WndProc::Show()
{
  if (getPrimeWindow()) {
    Log.Info(QString("Get show (rect: (%1, %2, %3, %4))")
             .arg(getSceneRect().left()).arg(getSceneRect().top()).arg(getSceneRect().width()).arg(getSceneRect().height()));
    HideTools();
    MoveWindow(mMainWindow, getSceneRect().left(), getSceneRect().top(), getSceneRect().width(), getSceneRect().height(), true);
  } else {
    Log.Info(QString("Get show"));
  }
  //SetWindowPos(mMainWindow, HWND_TOPMOST, getSceneRect().left(), getSceneRect().top(), getSceneRect().width(), getSceneRect().height()
  //              , SWP_SHOWWINDOW);
  ShowWindow(mMainWindow, SW_SHOW);
  mControlConnected = true;

  UpdateTools();
}

void WndProc::Hide()
{
  Log.Info(QString("Get hide"));

  ShowWindow(mMainWindow, SW_HIDE);
  mControlConnected = true;
}

void WndProc::ConnectBackWnd()
{
  if (!mControlWindow || !IsWindow(mControlWindow)) {
    HWND controlWindow = FindWindowExA(nullptr, nullptr, kControl, "Background window");
    if (mControlWindow == controlWindow) {
      return;
    } else {
      mControlWindow = controlWindow;
    }
    if (getPrimeWindow()) {
      Log.Info(QString("Get new control window"));
      SetParent(mMainWindow, mControlWindow);
    }
    PostMessageA(mControlWindow, kMsgRegisterPlayerTopLeft, (WPARAM)getId()
                 , MAKELPARAM((short)getSceneRect().left(), (short)getSceneRect().top()));

    PostMessageA(mControlWindow, kMsgRegisterPlayerBottomRight, (WPARAM)getId()
                 , MAKELPARAM((short)getSceneRect().right(), (short)getSceneRect().bottom()));

    PostMessageA(mControlWindow, kMsgRegisterPlayerType, (WPARAM)getId(), (LPARAM)getRender()->IsPrime());
    mControlConnected = false;
    mControlWait.start();
  } else if (!mControlConnected && mControlWait.elapsed() > kConnectControlWarnMs) {
    LOG_WARNING_ONCE("Wait connect control too long");
    if (mControlWait.elapsed() > kConnectControlDeadMs) {
      Log.Fatal("Wait connect control timeout", true);
    }
  }
}

void WndProc::ProcessMsgQueue()
{
  if (mToolsMode) {
    if (getStyle() == eStyleModerm) {
      AutoHideTools();
    }
  }

  MSG msg;
  while (::PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
}

void WndProc::OnChangedTime(const QDateTime& changeTime)
{
  if (mToolsWnd) {
    mToolsWnd->UpdateTime(changeTime);
  }
}

void WndProc::OnChangedFps(const qreal& changeFps)
{
  if (mToolsWnd) {
    mToolsWnd->UpdateFps(changeFps);
  }
}

void WndProc::OnChangedInfo(const QString& changeInfo)
{
  if (mToolsWnd) {
    mToolsWnd->SetCameraText(changeInfo);
  }
}

void WndProc::OnChangedBox()
{
  if (!getSceneRect().isEmpty()) {
    SetWindowPos(mMainWindow, HWND_TOPMOST, getSceneRect().left(), getSceneRect().top(), getSceneRect().width(), getSceneRect().height(), 0);
    ShowWindow(mMainWindow, SW_SHOW);
  } else {
    ShowWindow(mMainWindow, SW_HIDE);
  }
  UpdateTools();
}

void WndProc::ShowTools()
{
  if (!mToolsMode) {
    mToolsMode = true;
    UpdateTools();
    mToolsEnds.start();
  } else {
    mToolsEnds.restart();
  }
}

void WndProc::HideTools()
{
  if (mToolsMode) {
    mToolsMode = false;
    UpdateTools();
  }
}

void WndProc::DoubleClick()
{
  getRender()->OnScreenPlaceChange();
  if (mControlWindow) {
    PostMessageA(mControlWindow, kMsgMovePlayer, (WPARAM)getId(), 0);
  }
}

void WndProc::MouseMove(int key, const QPoint& p)
{
  if (key & MK_LBUTTON) {
    RECT rect;
    if (!GetClientRect(mDrawWindow, &rect)) {
      return;
    }
    QPointF scaled((qreal)p.x() / rect.right, (qreal)p.y() / rect.bottom);
    getDrawer()->MoveZoom(scaled);
  }
}

void WndProc::MouseWheel(int key, int delta)
{
  Q_UNUSED(key);

  getDrawer()->AddZoom(delta);
}

void WndProc::ResendMsg(UINT msg, WPARAM wparam, LPARAM lparam)
{
  if (mControlWindow) {
    PostMessageA(mControlWindow, msg, wparam, lparam);
  }
}

void WndProc::UpdateTools()
{
  if (!mToolsWnd) {
    return;
  }

  if (mToolsMode) {
    mToolsWnd->SetWidth(getSceneRect().width());
    mToolsWnd->Show(true);
    MoveWindow(mDrawWindow, 0, 0, getSceneRect().width(), getSceneRect().height() - mToolsWnd->Height(), true);
    MoveWindow(mToolsWnd->Hwnd(), 0, getSceneRect().height() - mToolsWnd->Height(), getSceneRect().width(), mToolsWnd->Height(), true);
    ShowWindow(mToolsWnd->Hwnd(), SW_SHOW);
    mToolsEnds.start();
  } else {
    mToolsWnd->Show(false);
    ShowWindow(mToolsWnd->Hwnd(), SW_HIDE);
    MoveWindow(mDrawWindow, 0, 0, getSceneRect().width(), getSceneRect().height(), true);
  }
}

void WndProc::AutoHideTools()
{
  POINT p;
  if (GetCursorPos(&p)) {
    RECT r;
    if (GetWindowRect(mToolsWnd->Hwnd(), &r)) {
      if (p.x >= r.left && p.x <= r.right && p.y >= r.top && p.y <= r.bottom) {
        mToolsEnds.restart();
      } else if (mToolsEnds.elapsed() >= kHideToolsMs) {
        HideTools();
      }
    }
  }
}

void WndProc::Confirm()
{
  mControlConnected = true;
}

void WndProc::ShowMainTools(const QPoint &place)
{
  PostMessageA(mControlWindow, kMsgShowTools, (WPARAM)getId()
               , MAKELPARAM((short)place.x(), (short)place.y()));
}

void WndProc::Resize(int width, int height)
{
  if (getEmbedded()) {
    SetSize(QSize(width, height));
    UpdateTools();
  }
}

void WndProc::RedrawScene()
{
  getDrawer()->Redraw();
}


WndProc::WndProc(Overseer* _Overseer, Render *_Render, Drawer *_Drawer, const QRect &_SceneRect
                 , bool _PrimeWindow, bool _ShowMouse, bool _AutoHideMouse, bool _AlwaysOnTop, bool _PlaySound, EStyleType _Style)
  : WndProcA(_Overseer, _Render, _Drawer, _SceneRect, _PrimeWindow, _ShowMouse, _AutoHideMouse, _AlwaysOnTop, _PlaySound, _Style)
  , mMainWindow(nullptr), mDrawWindow(nullptr), mToolsMode(false)
  , mControlWindow(nullptr), mControlConnected(false)
{
}

WndProc::~WndProc()
{
}


