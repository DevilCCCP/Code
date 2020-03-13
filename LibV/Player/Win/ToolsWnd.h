#pragma once

#include <Windows.h>
#include <gdiplus.h>

#include <QDateTime>
#include <QRect>

#include <Lib/Include/Common.h>
#include <LibV/Player/PlayerSettings.h>


typedef QSharedPointer<Gdiplus::Image> ImageS;
typedef QSharedPointer<Gdiplus::Pen> PenS;
typedef QSharedPointer<Gdiplus::Brush> BrushS;
typedef QSharedPointer<Gdiplus::Font> FontS;
typedef QSharedPointer<Gdiplus::StringFormat> StringFormatS;

DefineClassS(Render);
DefineStructS(Button);
DefineStructS(BarInfo);

struct BarInfo {
  QRect            Header;
  QVector<ButtonS> Buttons;
  QRect            IconRect;
  ImageS           IconImage;

  QRect            WorkArea;
  int              WidthPercent;
  int              WidthBase;
  bool             Opened;
  int              PosEnd;
};

struct Button {
  QRect           Header;
  QRect           IconRect;
  QVector<ImageS> IconImages;
  int             ImageIndex;

  int             Id;
};

enum EBarIndex {
  eClock      = 0,
  eCalendar   = 1,
  eInform     = 2,
  eRecords    = 3,
  eDownload   = 4,
  eFps        = 5,
  eBarIllegal = 6,
};

enum EButtonIndex {
  eMute,
  eSpeed,
  eReverse,
  eSwitchLive,
  eLeftPos,
  eRightPos,
  eSave,
};

struct SeekInfo {
  int       Scale;
  qint64    PeriodStart;
  qint64    PeriodFinish;
  QDateTime NextMoveTime;
  QDateTime PrevMoveTime;
  bool      Hand;
  int       HandPosX;
  qint64    SeekPos;

  SeekInfo(): Scale(0), Hand(false)
  { }
};

class ToolsWnd
{
  const bool        mPlaySound;
  const EStyleType  mStyle;
  HWND              mHwnd;
  Render*           mRender;
  QString           mCameraInfo;
  bool              mShow;

  QVector<BarInfoS> mBars;
  int               mBarCaptions;
  QVector<ButtonS>  mButtons;
  BrushS            mTextBrush;
  PenS              mControlPen;
  PenS              mControlPen2;
  BrushS            mBackgroundBrush;
  FontS             mFont;
  StringFormatS     mTextStringFormat;
  FontS             mSmallFont;
  StringFormatS     mSmallTextStringFormat;
  BrushS            mSmallTextBrush;

  HCURSOR           mCursorArrow;
  HCURSOR           mCursorHand;

  int               mWidth;
  int               mHeight;
  int               mFixedWidth;
  int               mFixedButtons;
  int               mPercentWidth;
  int               mButtonSize;
  int               mButtonMargin;

  QDateTime         mTimestamp;
  QDateTime         mLeftTimestamp;
  QDateTime         mRightTimestamp;
  SeekInfo          mSeekInfo;
  int               mSpeed;
  bool              mReverse;
  ButtonS           mSpeedBtn;
  ButtonS           mReverseBtn;
  qreal             mFps;

public:
  HWND Hwnd() const { return mHwnd; }
  int Height() const { return mHeight; }

public:
  void UpdateTime(const QDateTime& timestamp);
  void UpdateFps(const qreal& fps);
  void SetWidth(int _Width);
  void Show(bool show);
  void SetCameraText(const QString& _CameraInfo);

private:
  void UpdateFixedWidth();
  void RecalcCaption(bool update = false);
  int CalcButtonsSize(int buttonSize, int buttonMargin);

  bool UpdateSeek();
  void UpdateSeekPeriod();
  void SeekChange(int x);
  void SeekApply();

private:/*internal*/
  LRESULT WindowProc(UINT msg, WPARAM wparam, LPARAM lparam);

private:
  ImageS ImageFromFile(const QString& filename);
  void InitBars();
  void InitBarsModern();

  void Draw(Gdiplus::Graphics& g);
  void UpdateOneBar(int index);
  void UpdateBar(Gdiplus::Graphics& g, int index);
  void DrawButton(Gdiplus::Graphics& g, ImageS& image, const QRect& place, const QRect& background);
  void DrawClock(Gdiplus::Graphics& g, const Gdiplus::RectF& rect);
  void DrawCalendar(Gdiplus::Graphics& g, const Gdiplus::RectF& rect);
  void DrawInfo(Gdiplus::Graphics& g, const Gdiplus::RectF& rect);
  void DrawRecord(Gdiplus::Graphics& g, const Gdiplus::RectF& rect);
  int GetRecordPosition(const qint64& ts, int width);
  void DrawDownload(Gdiplus::Graphics& g, const Gdiplus::RectF& rect);
  void DrawFps(Gdiplus::Graphics& g, const Gdiplus::RectF& rect);

  void Resize(int width, int height);

  void Click(int x, int y);
  void ClickClock(int x, int y);
  void ClickCalendar(int x, int y);
  void ClickInfo(int x, int y);
  void ClickRecord(int x, int y);
  void ClickDownload(int x, int y);
  void ClickFps(int x, int y);

  void ClickUp(int x, int y);

  void ClickCancel();

  void MouseMove(int x, int y);

  void MouseWheel(int x, int y, int wheel);
  void MouseWheelRecord(int wheel);

  void ClickBtnMute(Button& btn);
  void ClickBtnSpeed(Button& btn);
  void ClickBtnReverse(Button& btn);
  void ClickBtnLive();
  void ClickBtnLeftPos();
  void ClickBtnRightPos();
  void ClickBtnSave();

public:
  explicit ToolsWnd(Render* _Render, HWND _MainHwnd, bool _PlaySound, EStyleType _Style);
  ~ToolsWnd();

  friend LRESULT CALLBACK ToolWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

