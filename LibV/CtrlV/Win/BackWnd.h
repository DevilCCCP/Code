#pragma once

#include <Windows.h>
#include <QElapsedTimer>
#include <QVector>
#include <QMap>

#include <LibV/Player/Win/CtrlWndDef.h>


DefineClassS(BackWnd);
DefineClassS(ToolWnd);
DefineClassS(CtrlV);

class BackWnd
{
  CtrlV*          mCtrlWnd;
  QRect           mScreenRect;
  bool            mCtrlTools;
  HWND            mMainWindow;
  bool            mMainWindowShow;

  HWND            mToolWindow;
  QVector<HWND>   mButtonsWnd;
  QPoint          mToolWindowPlace;
  QRect           mToolRect;
  QElapsedTimer   mHidingTools;
  int             mAlpha;

  QMap<int, HWND> mPlayerWnds;
  bool            mShowSecondary;
  QMap<int, HWND> mSecondaryWnds;

public: /*internal*/
  CtrlV* GetControl() { return mCtrlWnd; }
  void AddButtonWnd(HWND btn) { mButtonsWnd.append(btn); }

public:
  bool Init();
  void DoCircle();
  void Release();

private:
  void ProcessMsgQueue();
  void TestTools();
  void HideTools();
  void UpdateAlpha(int _Alpha);

public:
  void ShowTools(const QPoint& where = QPoint(0, -1));
  void UpdateTools();

public: /*internal*/
  void ShowPoint(const QPoint& point);
  bool UpdatePlayerWindow(int id);
  void ShowSecondary(int id);
  void ShowSecondaryChange(bool show);

  void PlayerShowRect(int id, QRect& rect);
  void PlayerHide(int id);
  void PlayerConfirm(int id, bool confirm);

private:
  HWND GetPlayerWnd(int id);
  HWND GetSecondaryWnd(int id);

public:
  BackWnd(CtrlV* _CtrlWnd, const QRect& _ScreenRect, bool _CtrlTools);
  /*override */virtual ~BackWnd();
};
