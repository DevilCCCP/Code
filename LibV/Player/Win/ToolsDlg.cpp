#include <Windows.h>
#include <Windowsx.h>
#include <Commctrl.h>
#include <QDateTime>

#include <Lib/Log/Log.h>

#include "ToolsDlg.h"
#include "../Render.h"


const int kSliderPeriod = 60*60;
const int kChangeSkipMs = 3000;

const int kButtonPlayId = 1001;
const int kButtonBackId = 1002;
const int kButtonFwdId = 1003;
const int kButtonLiveId = 1004;
const int kDateTimePickerDateId = 1010;
const int kDateTimePickerTimeId = 1011;
const int kSliderSeekId = 1020;
const int kComboBoxCameraId = 1030;

/////////////////////////////////////////////////////////////////////////////
//
// Dialog
//
/*
IDD_CONTROL DIALOGEX 0, 0, 300, 20
STYLE DS_SYSMODAL | DS_SETFONT | WS_CHILD | WS_VISIBLE
FONT 8, "Verdana", 400, 0, 0x2
BEGIN
    PUSHBUTTON      "",1001,1,0,18,19,BS_ICON
    PUSHBUTTON      "",1002,21,0,18,19,BS_ICON
    PUSHBUTTON      "",1003,39,0,18,19,BS_ICON
    COMBOBOX        1030,235,3,63,30,CBS_DROPDOWN | WS_VSCROLL | WS_TABSTOP
    CONTROL         "",1010,"SysDateTimePick32",DTS_RIGHTALIGN | WS_TABSTOP,78,3,48,15
    CONTROL         "",1011,"SysDateTimePick32",DTS_RIGHTALIGN | DTS_UPDOWN | WS_TABSTOP | 0x8,127,3,43,15
    CONTROL         "",1020,"msctls_trackbar32",TBS_NOTICKS | TBS_NOTIFYBEFOREMOVE | WS_TABSTOP,172,4,62,12
    PUSHBUTTON      "",1004,59,0,18,19,BS_ICON
END
*/
static unsigned char gDialogResurce[] = {
0x01, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x50, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x01, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x90, 0x01, 0x00, 0x02, 0x56, 0x00, 0x65, 0x00,
0x72, 0x00, 0x64, 0x00, 0x61, 0x00, 0x6e, 0x00, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x01, 0x50, 0x01, 0x00, 0x00, 0x00, 0x12, 0x00, 0x13, 0x00, 0xe9, 0x03, 0x00, 0x00, 0xff, 0xff, 0x80, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x01, 0x50, 0x15, 0x00, 0x00, 0x00, 0x12, 0x00, 0x13, 0x00, 0xea, 0x03, 0x00, 0x00, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x40, 0x00, 0x01, 0x50, 0x27, 0x00, 0x00, 0x00, 0x12, 0x00, 0x13, 0x00, 0xeb, 0x03, 0x00, 0x00, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x21, 0x50, 0xeb, 0x00, 0x03, 0x00,
0x3f, 0x00, 0x1e, 0x00, 0x06, 0x04, 0x00, 0x00, 0xff, 0xff, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x01, 0x50, 0x4e, 0x00, 0x03, 0x00, 0x30, 0x00, 0x0f, 0x00, 0xf2, 0x03, 0x00, 0x00, 0x53, 0x00,
0x79, 0x00, 0x73, 0x00, 0x44, 0x00, 0x61, 0x00, 0x74, 0x00, 0x65, 0x00, 0x54, 0x00, 0x69, 0x00, 0x6d, 0x00, 0x65, 0x00, 0x50, 0x00, 0x69, 0x00, 0x63, 0x00, 0x6b, 0x00, 0x33, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x29, 0x00, 0x01, 0x50, 0x7f, 0x00, 0x03, 0x00, 0x2b, 0x00, 0x0f, 0x00, 0xf3, 0x03, 0x00, 0x00, 0x53, 0x00, 0x79, 0x00, 0x73, 0x00, 0x44, 0x00, 0x61, 0x00, 0x74, 0x00, 0x65, 0x00, 0x54, 0x00, 0x69, 0x00, 0x6d, 0x00, 0x65, 0x00,
0x50, 0x00, 0x69, 0x00, 0x63, 0x00, 0x6b, 0x00, 0x33, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x01, 0x50, 0xac, 0x00, 0x04, 0x00, 0x3e, 0x00, 0x0c, 0x00, 0xfc, 0x03, 0x00, 0x00,
0x6d, 0x00, 0x73, 0x00, 0x63, 0x00, 0x74, 0x00, 0x6c, 0x00, 0x73, 0x00, 0x5f, 0x00, 0x74, 0x00, 0x72, 0x00, 0x61, 0x00, 0x63, 0x00, 0x6b, 0x00, 0x62, 0x00, 0x61, 0x00, 0x72, 0x00, 0x33, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x01, 0x50, 0x3b, 0x00, 0x00, 0x00, 0x12, 0x00, 0x13, 0x00, 0xec, 0x03, 0x00, 0x00, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x03, 0x34, 0x00, 0x00, 0x00
};

static ToolsDlg* gToolsDlg = nullptr;
INT_PTR CALLBACK ToolDlgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  if (gToolsDlg) {
    gToolsDlg->mHwnd = hwnd;
    return gToolsDlg->DlgProc(msg, wparam, lparam);
  } else {
    LOG_ERROR_ONCE("ToolsDlg undefined");
    return 0;
  }
}

void ToolsDlg::UpdateTime(const QDateTime &timestamp)
{
  if (!mPlaySeekMode) {
    if (mChangeTimer.elapsed() > kChangeSkipMs) {
      mSelectedTime = timestamp;
      DrawTime(timestamp);
    }
  }
}

INT_PTR ToolsDlg::DlgProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg) {
  case WM_INITDIALOG:
    return OnInitDlg();

  case WM_SIZE:
    return OnSize();

  case WM_COMMAND:
    switch (LOWORD(wparam)) {
    case kButtonPlayId:
      SwitchPlay(!mPlayMode);
      break;

    case kButtonLiveId:
      SwitchLive();
      break;

    case kButtonBackId:
      JumpTime(mSelectedTime.addSecs(-30));
      break;

    case kButtonFwdId:
      JumpTime(mSelectedTime.addSecs(30));
      break;
    }
    break;

  case WM_NOTIFY:
    switch (LOWORD(wparam)) {
    case kSliderSeekId:
      switch (((LPNMHDR)lparam)->code) {
      case TRBN_THUMBPOSCHANGING:
        if (mPlayMode) {
          mPlaySeekMode = true;
          SwitchPlay(false);
        }
        DrawTime(mSelectedTime.addSecs(((NMTRBTHUMBPOSCHANGING*)lparam)->dwPos));
        break;

      case NM_RELEASEDCAPTURE:
        JumpTime(mSelectedTime.addSecs(SendDlgItemMessage(mHwnd, kSliderSeekId, TBM_GETPOS, 0, 0)));
        SendDlgItemMessage(mHwnd, kSliderSeekId, TBM_SETPOS, true, 0);
        break;
      }
      break;
    }
    break;
  }
  return 0;
}

INT_PTR ToolsDlg::OnInitDlg()
{
  SendDlgItemMessage(mHwnd, kButtonPlayId, BM_SETIMAGE, IMAGE_ICON, (LPARAM)mIconPause);
  SendDlgItemMessage(mHwnd, kButtonBackId, BM_SETIMAGE, IMAGE_ICON, (LPARAM)mIconBack);
  SendDlgItemMessage(mHwnd, kButtonFwdId, BM_SETIMAGE, IMAGE_ICON, (LPARAM)mIconFwd);
  SendDlgItemMessage(mHwnd, kButtonLiveId, BM_SETIMAGE, IMAGE_ICON, (LPARAM)mIconLive);
  EnableWindow(GetDlgItem(mHwnd, kButtonLiveId), false);

  SendDlgItemMessage(mHwnd, kSliderSeekId, TBM_SETRANGEMAX, false, kSliderPeriod);
  SendDlgItemMessage(mHwnd, kSliderSeekId, TBM_SETRANGEMIN, false, -kSliderPeriod);
  SendDlgItemMessage(mHwnd, kSliderSeekId, TBM_SETPOS, true, 0);

  mSelectedTime = QDateTime::currentDateTime();
  DrawTime(mSelectedTime);
  return 0;
}

INT_PTR ToolsDlg::OnSize()
{
  RECT wndRect;
  GetWindowRect(mHwnd, &wndRect);
  int width = wndRect.right - wndRect.left;
  int height = wndRect.bottom - wndRect.top;
  RECT r;
  GetWindowRect(GetDlgItem(mHwnd, kSliderSeekId), &r);
  int start = (r.left - wndRect.left);
  int h = r.bottom - r.top;
  width -= start;
  MoveWindow(GetDlgItem(mHwnd, kSliderSeekId), start, (height - h) / 2, width / 2 - 1, r.bottom - r.top, true);
  MoveWindow(GetDlgItem(mHwnd, kComboBoxCameraId), start + width / 2 + 1, (height - h) / 2, width / 2 - 3, r.bottom - r.top, true);
  return 0;
}

void ToolsDlg::SwitchPlay(bool _PlayMode)
{
  if (mPlayMode == _PlayMode) {
    return;
  }

  mPlayMode = _PlayMode;
  SendDlgItemMessage(mHwnd, kButtonPlayId, BM_SETIMAGE, IMAGE_ICON, (LPARAM)((mPlayMode)? mIconPause: mIconPlay));
  if (mPlayMode) {
    mRender->OnPlay();
  } else {
    mRender->OnPause();
  }
}

void ToolsDlg::SwitchLive()
{
  if (mLiveMode) {
    return;
  }

  mLiveMode = true;
  EnableWindow(GetDlgItem(mHwnd, kButtonLiveId), !mLiveMode);
  mRender->OnSwitchLive();
  SwitchPlay(true);
}

void ToolsDlg::SwitchArchive()
{
  if (mLiveMode) {
    mLiveMode = false;
    EnableWindow(GetDlgItem(mHwnd, kButtonLiveId), !mLiveMode);
  }

  mRender->OnSwitchArchive(mSelectedTime);
}

void ToolsDlg::SetTime(const QDateTime &timestamp)
{
  mChangeTimer.restart();
  if (mSelectedTime == timestamp) {
    return;
  }

  mSelectedTime = timestamp;
  DrawTime(mSelectedTime);
}

void ToolsDlg::JumpTime(const QDateTime &timestamp)
{
  mSelectedTime = timestamp;
  if (mSelectedTime > QDateTime::currentDateTime()) {
    SetTime(QDateTime::currentDateTime());
    SwitchLive();
  } else {
    SetTime(mSelectedTime);
    SwitchArchive();
  }

  if (mPlaySeekMode) {
    SwitchPlay(true);
    mPlaySeekMode = false;
  }
}

void ToolsDlg::DrawTime(const QDateTime &newTime)
{
  SYSTEMTIME time;
  time.wYear = newTime.date().year();
  time.wMonth = newTime.date().month();
  time.wDay = newTime.date().day();
  time.wDayOfWeek = newTime.date().dayOfWeek();
  time.wHour = newTime.time().hour();
  time.wMinute = newTime.time().minute();
  time.wSecond = newTime.time().second();
  time.wMilliseconds = newTime.time().msec();

  SendDlgItemMessage(mHwnd, kDateTimePickerDateId, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&time);
  SendDlgItemMessage(mHwnd, kDateTimePickerTimeId, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&time);
}

ToolsDlg::ToolsDlg(Render *_Render, HWND _MainHwnd)
  : mRender(_Render)
  , mLiveMode(true), mPlayMode(true), mPlaySeekMode(false)
{
  mChangeTimer.start();
  gToolsDlg = this;

  mIconPlay = (HICON)LoadImageA(nullptr, ":/Icons/Play.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
  mIconPause = (HICON)LoadImageA(nullptr, ":/Icons/Pause.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
  mIconBack = (HICON)LoadImageA(nullptr, ":/Icons/Back.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
  mIconFwd = (HICON)LoadImageA(nullptr, ":/Icons/Fwd.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
  mIconLive = (HICON)LoadImageA(nullptr, ":/Icons/Live.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);

  CreateDialogIndirect(nullptr, (DLGTEMPLATE*)gDialogResurce, _MainHwnd, ToolDlgProc);

  EnableWindow(mHwnd, true);
  ShowWindow(mHwnd, SW_HIDE);
}

ToolsDlg::~ToolsDlg()
{
  DestroyWindow(mHwnd);
}
