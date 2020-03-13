#include <Windows.h>
#include <QRect>

#include <LibV/Include/ModuleNames.h>
#include <LibV/CtrlV/CtrlV.h>
#include <LibV/CtrlV/Win/BackWnd.h>

#include "BackWndProc.h"


LRESULT CALLBACK BackWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static BackWnd* gBackWnd = nullptr;
  static CtrlV* gCtrlWnd = nullptr;

  switch (msg) {
  case WM_CREATE:
    gBackWnd = (BackWnd*)((CREATESTRUCT*)lparam)->lpCreateParams;
    if (gBackWnd) {
      gCtrlWnd = gBackWnd->GetControl();
    }
    break;

  case kMsgRegisterPlayerType:
    if (gCtrlWnd) {
      gCtrlWnd->RegisterPlayerType((int)wparam, (bool)lparam);
    }
    return 0;

  case kMsgRegisterPlayerTopLeft:
  case kMsgRegisterPlayerBottomRight:
    if (gBackWnd) {
      if (gBackWnd->UpdatePlayerWindow((int)wparam)) {
        QPoint point((short)LOWORD(lparam), (short)HIWORD(lparam));
        if (msg == kMsgRegisterPlayerTopLeft) {
          gBackWnd->ShowPoint(QPoint(point.x() + 1, point.y() + 1));
        } else {
          gBackWnd->ShowPoint(QPoint(point.x() - 1, point.y() - 1));
        }
        if (gCtrlWnd) {
          if (msg == kMsgRegisterPlayerTopLeft) {
            gCtrlWnd->RegisterPlayerTopLeft((int)wparam, point);
          } else {
            gCtrlWnd->RegisterPlayerBottomRight((int)wparam, point);
          }
        }
      } else {
        if (gBackWnd && msg == kMsgRegisterPlayerTopLeft) {
          gBackWnd->ShowSecondary((int)wparam);
        }
      }
    }
    return 0;

  case kMsgMovePlayer:
    if (gCtrlWnd) {
      gCtrlWnd->MovePlayer((int)wparam);
    }
    return 0;

  case kMsgShowTools:
    return 0;
  }
  return DefWindowProcA(hwnd, msg, wparam, lparam);
}


HWND CreateBackWnd(const QRect& screenRect, LPVOID parent)
{
  WNDCLASSEXA wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style         = CS_DBLCLKS;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = NULL;
  wcex.hIcon         = NULL;
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);
  wcex.lpszMenuName	 = NULL;
  wcex.hIconSm       = NULL;

  wcex.lpfnWndProc   = BackWndProc;
  wcex.lpszClassName = kControl;
  if (RegisterClassExA(&wcex) == (ATOM)0) {
    return nullptr;
  }

  HWND hwnd = CreateWindowExA(WS_EX_TOOLWINDOW
#ifdef QT_NO_DEBUG
                              | WS_EX_TOPMOST
#endif
                              , kControl, "Background window", WS_POPUP | WS_CLIPCHILDREN
                              , screenRect.left(), screenRect.top(), screenRect.width(), screenRect.height()
                              , nullptr, nullptr, nullptr, parent);

  return hwnd;
}
