#include <Windows.h>
#include <commctrl.h>

#include <Lib/Log/Log.h>
#include <LibV/Include/ModuleNames.h>

#include "BackWnd.h"
#include "BackWndProc.h"
#include "ToolWndProc.h"
#include "../CtrlV.h"


bool BackWnd::Init()
{
  mMainWindow = CreateBackWnd(mScreenRect, (LPVOID)this);
  if (!mMainWindow) {
    return false;
  }
  if (mCtrlTools) {
    mToolWindow = CreateToolWnd(mScreenRect, mMainWindow, (LPVOID)this);
    if (!mToolWindow) {
      return false;
    }
    SetLayeredWindowAttributes(mToolWindow, RGB(0,255,0), 200, /*LWA_COLORKEY | */LWA_ALPHA);
    ShowWindow(mToolWindow, SW_HIDE);
    mAlpha = 0;
  }

  ShowWindow(mMainWindow, SW_HIDE);
  mMainWindowShow = false;
  return true;
}

void BackWnd::DoCircle()
{
  ProcessMsgQueue();

  if (mCtrlTools) {
    TestTools();
  }
}

void BackWnd::Release()
{
  if (mToolWindow) {
    DestroyWindow(mToolWindow);
  }
  if (mMainWindow) {
    DestroyWindow(mMainWindow);
  }
}

void BackWnd::ProcessMsgQueue()
{
  MSG msg;
  while (::PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
}

void BackWnd::TestTools()
{
  POINT p;
  if (!GetCursorPos(&p)) {
    return;
  }

  if (mToolRect.contains(p.x, p.y)) {
    mHidingTools.restart();
    UpdateAlpha(kToolsMaxAlpha);
    return;
  }

  if (qAbs(p.y - mScreenRect.top()) <= kToolsBorder) {
    int dx = p.x - (mScreenRect.left() + mScreenRect.right())/2;
    if (dx <= -mScreenRect.width() / 4) {
      if (p.x < mToolRect.left() || p.x > mToolRect.right()) {
        ShowTools(QPoint(-1, -2));
      } else {
        UpdateTools();
      }
    } else if (dx >= mScreenRect.width() / 4) {
      if (p.x < mToolRect.left() || p.x > mToolRect.right()) {
        ShowTools(QPoint(1, -2));
      } else {
        UpdateTools();
      }
    } else {
      if (p.x < mToolRect.left() || p.x > mToolRect.right()) {
        ShowTools(QPoint(0, -2));
      } else {
        UpdateTools();
      }
    }
//  } else if (qAbs(p.y - mScreenRect.bottom()) <= kToolsBorder) {
//    int dx = p.x - (mScreenRect.left() + mScreenRect.right())/2;
//    if (dx <= -mScreenRect.width() / 4) {
//      ShowTools(QPoint(-1, 2));
//    } else if (dx >= mScreenRect.width() / 4) {
//      ShowTools(QPoint(1, 2));
//    } else {
//      ShowTools(QPoint(0, 2));
//    }
  } else if (qAbs(p.x - mScreenRect.left()) <= kToolsBorder) {
    int dy = p.y - (mScreenRect.top() + mScreenRect.bottom())/2;
    if (dy <= -mScreenRect.height() / 4) {
      if (mToolWindowPlace.x() >= -1 || p.y < mToolRect.top() || p.y > mToolRect.bottom()) {
        ShowTools(QPoint(-2, -1));
      } else {
        UpdateTools();
      }
    } else if (dy >= mScreenRect.height() / 4) {
      if (mToolWindowPlace.x() >= -1 || p.y < mToolRect.top() || p.y > mToolRect.bottom()) {
        ShowTools(QPoint(-2, 1));
      } else {
        UpdateTools();
      }
    } else {
      if (mToolWindowPlace.x() >= -1 || p.y < mToolRect.top() || p.y > mToolRect.bottom()) {
        ShowTools(QPoint(-2, 0));
      } else {
        UpdateTools();
      }
    }
  } else if (qAbs(p.x - mScreenRect.right()) <= kToolsBorder) {
    int dy = p.y - (mScreenRect.top() + mScreenRect.bottom())/2;
    if (dy <= -mScreenRect.height() / 4) {
      if (mToolWindowPlace.x() <= 1 || p.y < mToolRect.top() || p.y > mToolRect.bottom()) {
        ShowTools(QPoint(2, -1));
      }
    } else if (dy >= mScreenRect.height() / 4) {
      if (mToolWindowPlace.x() <= 1 || p.y < mToolRect.top() || p.y > mToolRect.bottom()) {
        ShowTools(QPoint(2, 1));
      } else {
        UpdateTools();
      }
    } else {
      if (mToolWindowPlace.x() <= 1 || p.y < mToolRect.top() || p.y > mToolRect.bottom()) {
        ShowTools(QPoint(2, 0));
      } else {
        UpdateTools();
      }
    }
  } else if (mAlpha) {
    HideTools();
  }
}

void BackWnd::HideTools()
{
  int hide = mHidingTools.elapsed();
  if (hide >= kToolsFullHideMs) {
    ShowWindow(mToolWindow, SW_HIDE);
    mAlpha = 0;
  } else if (hide > kToolsStartHideMs) {
    UpdateAlpha(kToolsMaxAlpha * (kToolsFullHideMs - hide) / (kToolsFullHideMs - kToolsStartHideMs));
  }
}

void BackWnd::UpdateAlpha(int _Alpha)
{
  if (qAbs(mAlpha - _Alpha) >= 10) {
    mAlpha = _Alpha;
    SetLayeredWindowAttributes(mToolWindow, 0, mAlpha, /*LWA_COLORKEY | */LWA_ALPHA);
  }
}

void BackWnd::ShowTools(const QPoint& where)
{
  Log.Trace(QString("ShowTools (x: %1, y: %2)").arg(where.x()).arg(where.y()));
  if (mToolWindowPlace != where) {
    ShowWindow(mToolWindow, SW_HIDE);
    bool horzOld = qAbs(mToolWindowPlace.x()) <= qAbs(mToolWindowPlace.y());
    mToolWindowPlace = where;
    bool horz = qAbs(mToolWindowPlace.x()) <= qAbs(mToolWindowPlace.y());

    int l, t;
    if (mToolWindowPlace.x() == 0) {
      l = (mScreenRect.left() + mScreenRect.right())/2 - kToolLong / 2;
    } else if (mToolWindowPlace.x() < 0) {
      l = mScreenRect.left() + ((horz)? kToolMargin: kToolButtonMargin);
    } else {
      l = mScreenRect.right() - ((horz)? kToolLong + kToolMargin: kToolWidth + kToolButtonMargin);
    }
    if (mToolWindowPlace.y() == 0) {
      t = (mScreenRect.top() + mScreenRect.bottom())/2 - kToolLong / 2;
    } else if (mToolWindowPlace.y() < 0) {
      t = mScreenRect.top() + ((!horz)? kToolMargin: kToolButtonMargin);
    } else {
      t = mScreenRect.bottom() - ((!horz)? kToolLong + kToolMargin: kToolWidth + kToolButtonMargin);
    }
    mToolRect.setLeft(l);
    mToolRect.setTop(t);
    mToolRect.setWidth((horz)? kToolLong: kToolWidth);
    mToolRect.setHeight((horz)? kToolWidth: kToolLong);

    MoveWindow(mToolWindow, mToolRect.left(), mToolRect.top(), mToolRect.width(), mToolRect.height(), false);

    if (horz != horzOld) {
      if (horz) {
        for (int i = 0; i < kToolButtonCount; i++) {
          MoveWindow(mButtonsWnd[i], kToolButtonMargin + i * (kToolButtonMargin + kToolButtonWidth), kToolButtonMargin
                     , kToolButtonWidth, kToolButtonWidth, false);
        }
      } else {
        for (int i = 0; i < kToolButtonCount; i++) {
          MoveWindow(mButtonsWnd[i], kToolButtonMargin, kToolButtonMargin + i * (kToolButtonMargin + kToolButtonWidth)
                     , kToolButtonWidth, kToolButtonWidth, false);
        }
      }
    }
  }

  UpdateTools();
}

void BackWnd::UpdateTools()
{
  mHidingTools.start();
  UpdateAlpha(kToolsMaxAlpha);
  ShowWindow(mToolWindow, SW_SHOW);
}

void BackWnd::ShowPoint(const QPoint& point)
{
  if (!mMainWindowShow && mScreenRect.contains(point, true)) {
    ShowWindow(mMainWindow, SW_SHOW);
    mMainWindowShow = true;
  }
}

bool BackWnd::UpdatePlayerWindow(int id)
{
  HWND hwnd = FindWindowExA(mMainWindow, nullptr, GetPlayerWndClassName(id).toLatin1().constData(), "Video frame");
  if (!hwnd) {
    hwnd = FindWindowExA(nullptr, nullptr, GetPlayerWndClassName(id).toLatin1().constData(), "Video frame");
    if (!hwnd) {
      Log.Warning(QString("Player window not found (id: %1)").arg(id));
      return false;
    } else {
      mSecondaryWnds[id] = hwnd;
      return false;
    }
  } else {
    mPlayerWnds[id] = hwnd;
    return true;
  }
}

void BackWnd::ShowSecondary(int id)
{
  if (HWND hwnd = GetSecondaryWnd(id)) {
    Log.Info(QString("Send show secondary wnd (id: %1)").arg(id));
    if (mShowSecondary) {
      PostMessageA(hwnd, kMsgSetPlayerState, (WPARAM)-1, (LPARAM)-1);
    } else {
      PostMessageA(hwnd, kMsgSetPlayerState, (WPARAM)0, (LPARAM)0);
    }
  }
}

void BackWnd::ShowSecondaryChange(bool show)
{
  mShowSecondary = show;
  for (auto itr = mSecondaryWnds.begin(); itr != mSecondaryWnds.end(); itr++) {
    HWND hwnd = itr.value();
    if (mShowSecondary) {
      PostMessageA(hwnd, kMsgSetPlayerState, (WPARAM)-1, (LPARAM)-1);
    } else {
      PostMessageA(hwnd, kMsgSetPlayerState, (WPARAM)0, (LPARAM)0);
    }
  }
}

void BackWnd::PlayerShowRect(int id, QRect &rect)
{
  if (HWND hwnd = GetPlayerWnd(id)) {
    QPoint tl = rect.topLeft() - mScreenRect.topLeft();
    QPoint br = rect.bottomRight() - mScreenRect.topLeft();
    Log.Info(QString("Send show (id: %1, rect: (%2, %3, %4, %5))")
             .arg(id).arg(tl.x()).arg(tl.y()).arg(br.x()).arg(br.y()));
    PostMessageA(hwnd, kMsgSetPlayerState
                 , MAKEWPARAM((short)tl.x(), (short)tl.y()), MAKELPARAM((short)br.x(), (short)br.y()));
  }
}

void BackWnd::PlayerHide(int id)
{
  if (HWND hwnd = GetPlayerWnd(id)) {
    Log.Info(QString("Send hide (id: %1)").arg(id));
    PostMessageA(hwnd, kMsgSetPlayerState, 0, 0);
  }
}

void BackWnd::PlayerConfirm(int id, bool confirm)
{
  if (HWND hwnd = GetPlayerWnd(id)) {
    PostMessageA(hwnd, kMsgRegisterConfirm, (int)confirm, 0);
  }
}

HWND BackWnd::GetPlayerWnd(int id)
{
  auto itr = mPlayerWnds.find(id);
  if (itr == mPlayerWnds.end()) {
    Log.Warning(QString("Unregistered player %1").arg(id));
    return nullptr;
  }
  return itr.value();
}

HWND BackWnd::GetSecondaryWnd(int id)
{
  auto itr = mSecondaryWnds.find(id);
  if (itr == mSecondaryWnds.end()) {
    Log.Warning(QString("Unregistered player %1").arg(id));
    return nullptr;
  }
  return itr.value();
}


BackWnd::BackWnd(CtrlV* _CtrlWnd, const QRect& _ScreenRect, bool _CtrlTools)
  : mCtrlWnd(_CtrlWnd), mScreenRect(_ScreenRect), mCtrlTools(_CtrlTools), mMainWindow(nullptr), mMainWindowShow(false)
  , mToolWindow(nullptr), mToolWindowPlace(0, 0)
  , mShowSecondary(true)
{
}

BackWnd::~BackWnd()
{
}
