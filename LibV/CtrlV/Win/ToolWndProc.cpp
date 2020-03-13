#include <commctrl.h>

#include <Lib/Log/Log.h>
#include <LibV/Include/ModuleNames.h>

#include "ToolWndProc.h"
#include "BackWnd.h"
#include "../CtrlV.h"


const int kToolButtonIds[kToolButtonCount] = { kExitButtonId
                                               , kLayout0ButtonId
                                               , kLayout1ButtonId
                                               , kLayout2ButtonId
                                               , kLayout3ButtonId
                                               , kDesktopOnlyButtonId
                                               , kDesktopNoneButtonId
                                             };

const char* kToolButtonNames[kToolButtonCount] = { "Выход"
                                                   , "Исходная раскладка"
                                                   , "Раскладка углом"
                                                   , "Раскладка горизонтально"
                                                   , "Раскладка вертикально"
                                                   , "Только основной монитор"
                                                   , "Скрыть основной монитор"
                                             };

const char* kToolButtonIcons[kToolButtonCount] = { "./Icons/Power.ico"
                                                   , "./Icons/Layout0.ico"
                                                   , "./Icons/Layout1.ico"
                                                   , "./Icons/Layout2.ico"
                                                   , "./Icons/Layout3.ico"
                                                   , "./Icons/PrimeOnly.ico"
                                                   , "./Icons/OtherOnly.ico"
                                             };

LRESULT CALLBACK ToolsWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static BackWnd* gWndProc = nullptr;
  static CtrlV*   gCtrlWnd = nullptr;

  switch (msg) {
  case WM_CREATE:
    gWndProc = (BackWnd*)((CREATESTRUCT*)lparam)->lpCreateParams;
    if (gWndProc) {
      gCtrlWnd = gWndProc->GetControl();
    }

    HWND hwndTooltip;
    hwndTooltip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP
                                 , CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT
                                 , hwnd, nullptr, nullptr, nullptr);

    for (int i = 0; i < kToolButtonCount; i++) {
      QString name = QString::fromUtf8(kToolButtonNames[i]);
      HWND btn = CreateWindowW(L"BUTTON", (const wchar_t*)name.utf16(), BS_PUSHBUTTON | BS_ICON | WS_VISIBLE | WS_CHILD
                               , kToolButtonMargin + i * (kToolButtonMargin + kToolButtonWidth), kToolButtonMargin
                               , kToolButtonWidth, kToolButtonWidth
                               , hwnd, (HMENU)kToolButtonIds[i], 0, 0);
      HICON icon = (HICON)LoadImageA(nullptr, kToolButtonIcons[i], IMAGE_ICON
                                     , kToolButtonImageWidth, kToolButtonImageWidth, LR_LOADFROMFILE | LR_SHARED);
      SendMessageA(btn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)icon);

      TOOLINFOW tinfo;
      memset(&tinfo, 0, sizeof(TOOLINFO));
      tinfo.cbSize = sizeof(TOOLINFO);
      tinfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
      tinfo.uId = (UINT)btn;
      tinfo.lpszText = const_cast<wchar_t*>((const wchar_t*)name.utf16());
      SendMessageW(hwndTooltip, TTM_ADDTOOL, 0, (LPARAM)&tinfo);
      if (gWndProc) {
        gWndProc->AddButtonWnd(btn);
      }
    }
    break;

  case WM_COMMAND:
    if (!gCtrlWnd) {
      break;
    }
    switch (LOWORD(wparam)) {
    case kExitButtonId: gCtrlWnd->DoExit(); return 0;
    case kLayout0ButtonId: gCtrlWnd->DoLayout(ePreloadLayout, 0, 0, 0); return 0;
    case kLayout1ButtonId: gCtrlWnd->DoLayout(ePrimeHv, -1, 0, 0); return 0;
    case kLayout2ButtonId: gCtrlWnd->DoLayout(ePrimeHorz, -1, 0, 0); return 0;
    case kLayout3ButtonId: gCtrlWnd->DoLayout(ePrimeVert, -1, 0, 0); return 0;
    case kDesktopOnlyButtonId: gCtrlWnd->DoSwitchDesktop(); return 0;
    case kDesktopNoneButtonId: gCtrlWnd->DoSwitchOther(); return 0;
    }
    break;
  }
  return DefWindowProcA(hwnd, msg, wparam, lparam);
}

HWND CreateToolWnd(const QRect& screenRect, HWND mainWnd, LPVOID parent)
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

  wcex.lpfnWndProc   = ToolsWndProc;
  wcex.lpszClassName = "ToolWnd";
  if (RegisterClassExA(&wcex) == (ATOM)0) {
    return nullptr;
  }

  HWND hwnd = CreateWindowExA(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW
                              , "ToolWnd", "Tool window", WS_POPUP | WS_CLIPSIBLINGS
                              , screenRect.left() + kToolMargin, screenRect.top() + kToolMargin, kToolLong, kToolWidth
                              , mainWnd, nullptr, nullptr, parent);

  return hwnd;
}
