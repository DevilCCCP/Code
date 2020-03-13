#pragma once

#include <QDateTime>
#include <QElapsedTimer>

#include <Lib/Include/Common.h>


DefineClassS(Render);
DefineClassS(ToolsDlg);

class ToolsDlg
{
  HWND          mHwnd;
  Render*       mRender;

  HICON         mIconPlay;
  HICON         mIconPause;
  HICON         mIconBack;
  HICON         mIconFwd;
  HICON         mIconLive;

  bool          mLiveMode;
  bool          mPlayMode;
  bool          mPlaySeekMode;
  QDateTime     mSelectedTime;
  QElapsedTimer mChangeTimer;

public:
  HWND Hwnd() const { return mHwnd; }

public:/*internal*/
  void UpdateTime(const QDateTime& timestamp);

private:/*internal*/
  INT_PTR DlgProc(UINT msg, WPARAM wparam, LPARAM lparam);
  INT_PTR OnInitDlg();
  INT_PTR OnSize();

  void SwitchPlay(bool _PlayMode);
  void SwitchLive();
  void SwitchArchive();
  void SetTime(const QDateTime& timestamp);
  void JumpTime(const QDateTime& timestamp);

  void DrawTime(const QDateTime& newTime);

public:
  explicit ToolsDlg(Render* _Render, HWND _MainHwnd);
  ~ToolsDlg();

  friend INT_PTR CALLBACK ToolDlgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

