#include <Windowsx.h>
#include <QFile>
#include <QList>

#include <Lib/Log/Log.h>

#include "ToolsWnd.h"
#include "../Render.h"

const char* gClassName = "PlayerToolWnd";
const int kButtonSize = 32;
const int kButtonMargin = 4;
const int kHeight = 40;

const int kScaleMinimum = 2*60;
const int kScaleMaximum = 32*60*60;
const int kScaleDefault = 60*60;
const int kScaleMilestones[] = { 60, 2*60, 5*60, 10*60, 15*60, 30*60, 60*60, 2*60*60, 4*60*60, 8*60*60, 12*60*60, 24*60*60, 0 };
const int kScaleMilestonesPixel = 80;
const QList<int> kSpeedValues = QList<int>() << 1 << 2 << 4 << 8;
const int kSpeedDefaultIndex = 0;


using namespace Gdiplus;

LRESULT CALLBACK ToolWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static ToolsWnd* gToolsWnd = nullptr;

  switch (msg) {
  case WM_CREATE:
    gToolsWnd = (ToolsWnd*)((CREATESTRUCT*)lparam)->lpCreateParams;
    return 0;

  default:
    if (gToolsWnd && hwnd == gToolsWnd->Hwnd()) {
      return gToolsWnd->WindowProc(msg, wparam, lparam);
    }
    break;
  }
  return DefWindowProcA(hwnd, msg, wparam, lparam);
}



void ToolsWnd::UpdateTime(const QDateTime &timestamp)
{
  if (mTimestamp == timestamp) {
    return;
  }
  bool dateEq = mTimestamp.date() == timestamp.date();

  mTimestamp = timestamp;
  if (mSpeed > 1 && !mReverse && mTimestamp > QDateTime::currentDateTime()) {
    mSpeed = 1;
    mRender->OnSwitchLive();
    mSpeedBtn->ImageIndex = kSpeedDefaultIndex;
    UpdateOneBar(eRecords);
  }

  if (mShow && !mSeekInfo.Hand) {
    BarInfo& info = *mBars[eClock];
    if (info.Opened) {
      HDC hdc = GetDC(mHwnd);
      Graphics g(hdc);
      UpdateBar(g, eClock);
      if (!dateEq) {
        UpdateBar(g, eCalendar);
      }
      if (UpdateSeek()) {
        UpdateBar(g, eRecords);
      }
      ReleaseDC(mHwnd, hdc);
    }
  }
}

void ToolsWnd::UpdateFps(const qreal& fps)
{
  if (mStyle == eStyleModerm) {
    return;
  }

  mFps = fps;
  HDC hdc = GetDC(mHwnd);
  Graphics g(hdc);
  UpdateBar(g, eFps);
  ReleaseDC(mHwnd, hdc);
}

void ToolsWnd::SetWidth(int _Width)
{
  UpdateFixedWidth();

  mWidth = _Width;
  if (CalcButtonsSize(kButtonSize, kButtonMargin) + mFixedWidth < mWidth / 2) {
    mButtonSize = kButtonSize;
    mHeight = kHeight;
    mButtonMargin = kButtonMargin;
  } else {
    mButtonSize = kButtonSize / 2;
    mHeight = kHeight / 2;
    mButtonMargin = kButtonMargin / 2;
  }

  RecalcCaption();
}

void ToolsWnd::Show(bool show)
{
  mShow = show;
  if (show) {
    UpdateSeek();
  }
}

void ToolsWnd::SetCameraText(const QString &_CameraInfo)
{
  mCameraInfo = _CameraInfo;
  UpdateOneBar(eInform);
}

void ToolsWnd::UpdateFixedWidth()
{
  mFixedWidth = 0;
  mPercentWidth = 0;
  mFixedButtons = 0;
  mBarCaptions = 0;
  for (int i = 0; i < mBars.size(); i++) {
    const BarInfo& info = *mBars[i];
    if (info.IconImage) {
      mBarCaptions++;
    }
    if (info.Opened) {
      mFixedButtons += info.Buttons.size();
      mFixedWidth += info.WidthBase;
      mPercentWidth += info.WidthPercent;
    }
  }
}

void ToolsWnd::RecalcCaption(bool update)
{
  if (update) {
    UpdateFixedWidth();
  }

  int freeWidth = mWidth - (mFixedWidth + CalcButtonsSize(mButtonSize, mButtonMargin));
  int freePercent = mPercentWidth;
  int pos = 0;
  for (int i = 0; i < mBars.size(); i++) {
    BarInfo& info = *mBars[i];
    if (info.IconImage) {
      info.Header.setCoords(pos, 0, pos + mButtonSize + mButtonMargin - 1, mHeight - 1);
      info.IconRect.setCoords(pos, mButtonMargin, pos + mButtonSize - 1, mButtonSize + mButtonMargin - 1);
      pos += mButtonSize + mButtonMargin;
    } else {
      info.Header.setCoords(pos, 0, pos - 1, mHeight - 1);
      info.IconRect.setCoords(pos, mButtonMargin, pos - 1, mButtonSize + mButtonMargin - 1);
    }
    if (info.Opened) {
      int extraWidth = (freePercent > 0 && freeWidth > 0)? freeWidth * info.WidthPercent / freePercent: 0;
      freePercent -= info.WidthPercent;
      freeWidth -= extraWidth;
      int workWidth = info.WidthBase + extraWidth;
      info.WorkArea.setCoords(pos, 0, pos + workWidth - 1, mHeight - 1);
      pos += workWidth;

      for (int j = 0; j < info.Buttons.size(); j++) {
        Button& btn = *info.Buttons[j];
        if (btn.ImageIndex < btn.IconImages.size() && btn.ImageIndex >= 0) {
          btn.Header.setCoords(pos, 0, pos + mButtonSize + mButtonMargin - 1, mHeight - 1);
          btn.IconRect.setCoords(pos, mButtonMargin, pos + mButtonSize - 1, mButtonSize + mButtonMargin - 1);
          pos += mButtonSize + mButtonMargin;
        }
      }
    }
    info.PosEnd = pos;
  }
}

int ToolsWnd::CalcButtonsSize(int buttonSize, int buttonMargin)
{
  return (buttonSize + buttonMargin) * (mBarCaptions + mButtons.size() + mFixedButtons);
}

bool ToolsWnd::UpdateSeek()
{
  BarInfo& info = *mBars[eRecords];
  if (!info.Opened) {
    return false;
  }
  if (!mSeekInfo.Scale) {
    mSeekInfo.Scale = kScaleDefault;
  } else if (mTimestamp < mSeekInfo.NextMoveTime && mTimestamp > mSeekInfo.PrevMoveTime) {
    return false;
  }

  UpdateSeekPeriod();
  return true;
}

void ToolsWnd::UpdateSeekPeriod()
{
//  qint64 nowTs = QDateTime::currentDateTime().toMSecsSinceEpoch();
  qint64 curTs = (mSeekInfo.Hand)? mSeekInfo.SeekPos: mTimestamp.toMSecsSinceEpoch();

  mSeekInfo.PeriodFinish = curTs + mSeekInfo.Scale * 1000 / 2;
//  if (mSeekInfo.PeriodFinish >= nowTs) {
//    mSeekInfo.PeriodFinish = nowTs + 1;
//  }
  mSeekInfo.PeriodStart = mSeekInfo.PeriodFinish - mSeekInfo.Scale * 1000;
}

void ToolsWnd::SeekChange(int x)
{
  int delta = x - mSeekInfo.HandPosX;
  if (!delta) {
    return;
  }
  BarInfo& info = *mBars[eRecords];
  int width = info.WorkArea.width() - 3;
  mSeekInfo.HandPosX = x;
  qint64 msStep = delta * 1000 * mSeekInfo.Scale / width;
  mSeekInfo.SeekPos -= msStep;

  HDC hdc = GetDC(mHwnd);
  Graphics g(hdc);

  UpdateSeekPeriod();
  UpdateBar(g, eClock);
  UpdateBar(g, eCalendar);
  UpdateBar(g, eRecords);

  ReleaseDC(mHwnd, hdc);
}

void ToolsWnd::SeekApply()
{
  QDateTime newTime = QDateTime::fromMSecsSinceEpoch(mSeekInfo.SeekPos);
  mSeekInfo.NextMoveTime = mSeekInfo.PrevMoveTime = newTime;
  if (newTime > QDateTime::currentDateTime()) {
    mSpeed = 1;
    mReverse = false;
    mRender->OnSwitchLive();
    mSpeedBtn->ImageIndex = kSpeedDefaultIndex;
    mReverseBtn->ImageIndex = 0;
    UpdateOneBar(eRecords);
  } else {
    mRender->OnSwitchArchive(newTime, mReverse? -mSpeed: mSpeed, 1);
  }
}

LRESULT ToolsWnd::WindowProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg) {
  case WM_SIZE:
    Resize(LOWORD(lparam), HIWORD(lparam));
    return 0;

  case WM_LBUTTONDOWN:
    Click(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
    SetFocus(mHwnd);
    return 0;

  case WM_LBUTTONUP:
    ClickUp(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
    return 0;

  case WM_RBUTTONDOWN:
    ClickCancel();
    return 0;

  case WM_MOUSEMOVE:
    MouseMove(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
    return 0;

  case WM_MOUSEWHEEL:
    MouseWheel(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA);
    return 0;

  case WM_PAINT:
    if (true) {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(mHwnd, &ps);
      Graphics g(hdc);
      Draw(g);
      EndPaint(mHwnd, &ps);
    }
    return 0;
  }
  return DefWindowProcA(mHwnd, msg, wparam, lparam);
}

ImageS ToolsWnd::ImageFromFile(const QString& filename)
{
  QImage img(filename);
  Bitmap* bmp;
  ImageS image(bmp = new Bitmap(img.width(), img.height()));
  for (int j = 0; j < img.height(); j++) {
    for (int i = 0; i < img.width(); i++) {
      QRgb c = img.pixel(i, j);
      Color col(qAlpha(c), qRed(c), qGreen(c), qBlue(c));
      bmp->SetPixel(i, j, col);
    }
  }
  return image;
}

void ToolsWnd::InitBars()
{
  switch (mStyle) {
  case eStyleModerm:  return InitBarsModern();
  }
}

void ToolsWnd::InitBarsModern()
{
  ButtonS btn;
  BarInfoS info;
  mBars.resize(eBarIllegal);

  info = BarInfoS(new BarInfo());
  info->IconImage = ImageFromFile(":/Icons/Clock.png");
  info->WidthBase = 72;
  info->WidthPercent = 0;
  info->Opened = true;
  mBars[eClock] = info;

  info = BarInfoS(new BarInfo());
  info->IconImage = ImageFromFile(":/Icons/Calendar.png");
  info->WidthBase = 72;
  info->WidthPercent = 0;
  info->Opened = false;
  mBars[eCalendar] = info;

  info = BarInfoS(new BarInfo());
  info->IconImage = ImageFromFile(":/Icons/Info.png");
  info->WidthBase = 100;
  info->WidthPercent = 30;
  info->Opened = true;

  if (mPlaySound) {
    btn = ButtonS(new Button());
    btn->IconImages.append(ImageFromFile(":/Icons/SoundOff.png"));
    btn->IconImages.append(ImageFromFile(":/Icons/SoundOn.png"));
    btn->ImageIndex = 0;
    btn->Id = eMute;
    info->Buttons.append(btn);
  }
  mBars[eInform] = info;

  info = BarInfoS(new BarInfo());
  info->IconImage = ImageFromFile(":/Icons/Record.png");
  info->WidthBase = 200;
  info->WidthPercent = 70;
  info->Opened = true;

  btn = ButtonS(new Button());
  btn->IconImages.append(ImageFromFile(":/Icons/Speed1.png"));
  btn->IconImages.append(ImageFromFile(":/Icons/Speed2.png"));
  btn->IconImages.append(ImageFromFile(":/Icons/Speed4.png"));
  btn->IconImages.append(ImageFromFile(":/Icons/Speed8.png"));
  btn->ImageIndex = kSpeedDefaultIndex;
  mSpeed = 1;
  btn->Id = eSpeed;
  mSpeedBtn = btn;
  info->Buttons.append(btn);

  btn = ButtonS(new Button());
  btn->IconImages.append(ImageFromFile(":/Icons/ReverseNo.png"));
  btn->IconImages.append(ImageFromFile(":/Icons/Reverse.png"));
  btn->ImageIndex = 0;
  mReverse = false;
  btn->Id = eReverse;
  mReverseBtn = btn;
  info->Buttons.append(btn);

  btn = ButtonS(new Button());
  btn->IconImages.append(ImageFromFile(":/Icons/Live.png"));
  btn->ImageIndex = 0;
  btn->Id = eSwitchLive;
  info->Buttons.append(btn);
  mBars[eRecords] = info;

  info = BarInfoS(new BarInfo());
  info->IconImage = ImageFromFile(":/Icons/Download.png");
  info->WidthBase = 0;
  info->WidthPercent = 0;
  info->Opened = false;

  btn = ButtonS(new Button());
  btn->IconImages.append(ImageFromFile(":/Icons/LeftPos.png"));
  btn->ImageIndex = 0;
  btn->Id = eLeftPos;
  info->Buttons.append(btn);

  btn = ButtonS(new Button());
  btn->IconImages.append(ImageFromFile(":/Icons/RightPos.png"));
  btn->ImageIndex = 0;
  btn->Id = eRightPos;
  info->Buttons.append(btn);

  btn = ButtonS(new Button());
  btn->IconImages.append(ImageFromFile(":/Icons/Save.png"));
  btn->ImageIndex = 0;
  btn->Id = eSave;
  info->Buttons.append(btn);
  mBars[eDownload] = info;

  info = BarInfoS(new BarInfo());
  info->WidthBase = 0;
  info->WidthPercent = 0;
  info->Opened = false;
  mBars[eFps] = info;

  mBackgroundBrush = BrushS(new SolidBrush(Color(145, 184, 74)));
  mTextBrush = BrushS(new SolidBrush(Color(0, 0, 0)));
  mSmallTextBrush = BrushS(new SolidBrush(Color(255, 255, 255)));
  mControlPen = PenS(new Pen(Color(255, 255, 255)));
  mControlPen2 = PenS(new Pen(Color(127, 127, 127)));
  mFont = FontS(new Font(L"Calibri", 16.5f, FontStyleRegular, UnitPixel));
  mTextStringFormat = StringFormatS(new StringFormat(0, LANG_NEUTRAL));
  mTextStringFormat->SetAlignment(StringAlignmentNear);
  mTextStringFormat->SetLineAlignment(StringAlignmentCenter);
  mSmallFont = FontS(new Font(L"Calibri", 12.0f, FontStyleRegular, UnitPixel));
  mSmallTextStringFormat = StringFormatS(new StringFormat(0, LANG_NEUTRAL));
  mSmallTextStringFormat->SetAlignment(StringAlignmentCenter);
  mSmallTextStringFormat->SetLineAlignment(StringAlignmentCenter);
}

void ToolsWnd::Draw(Gdiplus::Graphics &g)
{
  RECT captionRect;
  GetClientRect(mHwnd, &captionRect);
  int width = captionRect.right - captionRect.left;
  int height = captionRect.bottom - captionRect.top;
  if (mWidth != width) {
    RecalcCaption(false);
  }
  BarInfo& info = *mBars.last();
  Rect captionRect_(info.PosEnd, captionRect.top, width - info.PosEnd, height);
  g.FillRectangle(mBackgroundBrush.data(), captionRect_);

  for (int i = 0; i < mBars.size(); i++) {
    BarInfo& info = *mBars[i];
    if (info.IconImage) {
      DrawButton(g, info.IconImage, info.IconRect, info.Header);
    }

    UpdateBar(g, i);
  }
}

void ToolsWnd::UpdateOneBar(int index)
{
  HDC hdc = GetDC(mHwnd);
  Graphics g(hdc);
  UpdateBar(g, index);
  ReleaseDC(mHwnd, hdc);
}

void ToolsWnd::UpdateBar(Gdiplus::Graphics &g, int index)
{
  BarInfo& info = *mBars[index];

  if (info.Opened) {
    for (int j = 0; j < info.Buttons.size(); j++) {
      Button& btn = *info.Buttons[j];
      if (btn.ImageIndex < btn.IconImages.size() && btn.ImageIndex >= 0) {
        DrawButton(g, btn.IconImages[btn.ImageIndex], btn.IconRect, btn.Header);
      }
    }

    if (info.WorkArea.width() > 0) {
      Rect rectDest(info.WorkArea.left(), info.WorkArea.top(), info.WorkArea.width(), info.WorkArea.height());
      Bitmap back(info.WorkArea.width(), info.WorkArea.height());
      Graphics gb(&back);
      RectF rect(0, 0, info.WorkArea.width(), info.WorkArea.height());
      gb.FillRectangle(mBackgroundBrush.data(), rect);
      switch (index) {
      case eClock   : DrawClock(gb, rect); break;
      case eCalendar: DrawCalendar(gb, rect); break;
      case eInform  : DrawInfo(gb, rect); break;
      case eRecords : DrawRecord(gb, rect); break;
      case eDownload: DrawDownload(gb, rect); break;
      case eFps     : DrawFps(gb, rect); break;
      }
      if (Status s = g.DrawImage(&back, rectDest)) {
        Log.Warning(QString("DrawImage fail 1 (status: %1)").arg(s));
      }
    }
  }
}

void ToolsWnd::DrawButton(Gdiplus::Graphics &g, ImageS &image, const QRect &place, const QRect &background)
{
  Bitmap back(background.width(), background.height());
  {
    Graphics gb(&back);
    Rect rect(0, 0, background.width(), background.height());
    if (Status s = gb.FillRectangle(mBackgroundBrush.data(), rect)) {
      Log.Warning(QString("FillRectangle fail 1 (status: %1, rect: (%2, %3, %4, %5))").arg(s)
                  .arg(rect.X).arg(rect.Y).arg(rect.Width).arg(rect.Height));
    }
    Rect rectIcon(place.left() - background.left(), place.top() - background.top(), place.width(), place.height());
    gb.SetInterpolationMode(InterpolationModeHighQuality);
    if (Status s = gb.DrawImage(image.data(), rectIcon)) {
      Log.Warning(QString("DrawImage fail 2 (status: %1)").arg(s));
    }
  }
  Rect rectDest(background.left(), background.top(), background.width(), background.height());
  if (Status s = g.DrawImage(&back, rectDest)) {
    Log.Warning(QString("DrawImage fail 3 (status: %1)").arg(s));
  }
}

void ToolsWnd::DrawClock(Gdiplus::Graphics &g, const Gdiplus::RectF &rect)
{
  QDateTime drawTime = (mSeekInfo.Hand)? QDateTime::fromMSecsSinceEpoch(mSeekInfo.SeekPos): mTimestamp;
  QString text = drawTime.toString("hh:mm:ss");
  g.SetTextRenderingHint(TextRenderingHintAntiAlias);
  g.DrawString((const wchar_t*)text.utf16(), text.length(), mFont.data(), rect, mTextStringFormat.data(), mTextBrush.data());
}

void ToolsWnd::DrawCalendar(Graphics &g, const RectF &rect)
{
  QDateTime drawTime = (mSeekInfo.Hand)? QDateTime::fromMSecsSinceEpoch(mSeekInfo.SeekPos): mTimestamp;
  QString text = drawTime.toString("dd/MM/yy");
  g.SetTextRenderingHint(TextRenderingHintAntiAlias);
  g.DrawString((const wchar_t*)text.utf16(), text.length(), mFont.data(), rect, mTextStringFormat.data(), mTextBrush.data());
}

void ToolsWnd::DrawInfo(Gdiplus::Graphics &g, const Gdiplus::RectF &rect)
{
  QString text = mCameraInfo;
  g.SetTextRenderingHint(TextRenderingHintAntiAlias);
  g.DrawString((const wchar_t*)text.utf16(), text.length(), mFont.data(), rect, mTextStringFormat.data(), mTextBrush.data());
}

void ToolsWnd::DrawRecord(Gdiplus::Graphics &g, const Gdiplus::RectF &rect)
{
  int width = rect.Width - 3;
  int height = rect.Height - 1;
  Rect rectLine(0, 2, width + 2, 4);
  g.DrawRectangle(mControlPen.data(), rectLine);
  if (mLeftTimestamp.isValid() && mRightTimestamp.isValid()) {
    int posL = GetRecordPosition(mLeftTimestamp.toMSecsSinceEpoch(), width);
    int posR = GetRecordPosition(mRightTimestamp.toMSecsSinceEpoch(), width);
    if (posL < width && posR > 0) {
      Rect rectLineSel(posL, 2, posR - posL, 4);
      g.DrawRectangle(mControlPen2.data(), rectLineSel);
    }
  }
  qint64 msStep = 1000 * mSeekInfo.Scale / width;
  mSeekInfo.NextMoveTime = mTimestamp.addMSecs(msStep);
  mSeekInfo.PrevMoveTime = mTimestamp.addMSecs(-msStep);

  int milestoneSec = 24*60*60;
  for (int i = 0; kScaleMilestones[i]; i++) {
    int count = mSeekInfo.Scale / kScaleMilestones[i];
    if (count * kScaleMilestonesPixel < width) {
      milestoneSec = kScaleMilestones[i];
      break;
    }
  }

  QDateTime first = QDateTime::fromMSecsSinceEpoch(mSeekInfo.PeriodStart);
  QDateTime last = QDateTime::fromMSecsSinceEpoch(mSeekInfo.PeriodFinish);
  if (milestoneSec <= 60*60) {
    int milestoneMinutes = milestoneSec / 60;
    QTime firstTime(first.time().hour(), first.time().minute() / milestoneMinutes * milestoneMinutes, 0, 0);
    first.setTime(firstTime);
  } else {
    int milestoneHours = milestoneSec / (60*60);
    QTime firstTime(first.time().hour() / milestoneHours * milestoneHours, 0, 0, 0);
    first.setTime(firstTime);
  }

  const int kSeekTextWidth = 36;
  g.SetTextRenderingHint(TextRenderingHintAntiAlias);
  for (QDateTime time = first.addSecs(milestoneSec); time < last; time = time.addSecs(milestoneSec)) {
    qint64 timeMs = time.toMSecsSinceEpoch();
    int pos = GetRecordPosition(timeMs, width);
    g.DrawLine(mControlPen.data(), pos, 2, pos, 8);
    QString text = time.toString("hh:mm");
    int textPos = (pos < width/2)? qMax(pos - kSeekTextWidth/2, 0): qMin(pos - kSeekTextWidth/2, width - kSeekTextWidth);
    RectF rectText(textPos, 6, kSeekTextWidth, height);
    g.DrawString((const wchar_t*)text.utf16(), text.length(), mSmallFont.data(), rectText, mSmallTextStringFormat.data(), mSmallTextBrush.data());
  }

  qint64 curTimeMs = (mSeekInfo.Hand)? mSeekInfo.SeekPos: mTimestamp.toMSecsSinceEpoch();
  int pos = (int)((curTimeMs - mSeekInfo.PeriodStart) * width / (mSeekInfo.PeriodFinish - mSeekInfo.PeriodStart));
  QVector<Point> points;
  points.append(Point(pos - 2, 1));
  points.append(Point(pos + 2, 1));
  points.append(Point(pos, 3));
  points.append(Point(pos - 1, 1));
  g.DrawLines(mControlPen.data(), points.data(), points.size());

  points.clear();
  points.append(Point(pos - 2, 7));
  points.append(Point(pos + 2, 7));
  points.append(Point(pos, 5));
  points.append(Point(pos - 1, 7));
  g.DrawLines(mControlPen.data(), points.data(), points.size());
}

int ToolsWnd::GetRecordPosition(const qint64 &ts, int width)
{
  return (int)((ts - mSeekInfo.PeriodStart) * width / (mSeekInfo.PeriodFinish - mSeekInfo.PeriodStart));
}

void ToolsWnd::DrawDownload(Gdiplus::Graphics&, const Gdiplus::RectF&)
{
}

void ToolsWnd::DrawFps(Gdiplus::Graphics& g, const Gdiplus::RectF& rect)
{
  QString text = mFps > 0.01? QString("%1").arg(mFps, 2, 'f', 2): QString("--.--");
  g.SetTextRenderingHint(TextRenderingHintAntiAlias);
  g.DrawString((const wchar_t*)text.utf16(), text.length(), mFont.data(), rect, mTextStringFormat.data(), mTextBrush.data());
}

void ToolsWnd::Resize(int width, int height)
{
  if (width != mWidth) {
    RecalcCaption(false);
    if (mHeight != height) {
      Log.Warning(QString("Tool wnd: wrong height (calc: %1, get: %2)").arg(mHeight).arg(height));
    }

    HDC hdc = GetDC(mHwnd);
    Graphics g(hdc);

    Draw(g);
    ReleaseDC(mHwnd, hdc);
  }
}

void ToolsWnd::Click(int x, int y)
{
  for (int i = 0; i < mBars.size(); i++) {
    BarInfo& info = *mBars[i];
    if (x < info.PosEnd) {
      if (info.Header.contains(x, y)) {
        info.Opened = !info.Opened;
        HDC hdc = GetDC(mHwnd);
        Graphics g(hdc);

        RecalcCaption(true);
        Draw(g);
        ReleaseDC(mHwnd, hdc);
      } else if (info.Opened) {
        if (info.WorkArea.contains(x, y)) {
          x -= info.WorkArea.left();
          y -= info.WorkArea.top();
          switch (i) {
          case eClock   : ClickClock(x, y); break;
          case eCalendar: ClickCalendar(x, y); break;
          case eInform  : ClickInfo(x, y); break;
          case eRecords : ClickRecord(x, y); break;
          case eDownload: ClickDownload(x, y); break;
          case eFps     : ClickFps(x, y); break;
          }
        } else {
          for (int j = 0; j < info.Buttons.size(); j++) {
            Button& btn = *info.Buttons[j];
            if (btn.Header.contains(x, y)) {
              switch (btn.Id) {
              case eMute:       ClickBtnMute(btn); break;
              case eSwitchLive: ClickBtnLive(); break;
              case eSpeed:      ClickBtnSpeed(btn); break;
              case eReverse:    ClickBtnReverse(btn); break;
              case eLeftPos:    ClickBtnLeftPos(); break;
              case eRightPos:   ClickBtnRightPos(); break;
              case eSave:       ClickBtnSave(); break;
              }

              UpdateOneBar(i);
            }
          }
        }
      }
      break;
    }
  }
}

void ToolsWnd::ClickClock(int, int)
{
}

void ToolsWnd::ClickCalendar(int, int)
{
}

void ToolsWnd::ClickInfo(int, int)
{
}

void ToolsWnd::ClickRecord(int x, int)
{
  mSeekInfo.Hand = true;
  mSeekInfo.HandPosX = x;
  mSeekInfo.SeekPos = mTimestamp.toMSecsSinceEpoch();
  SetCapture(mHwnd);
}

void ToolsWnd::ClickDownload(int, int)
{
}

void ToolsWnd::ClickFps(int, int)
{
}

void ToolsWnd::ClickUp(int, int)
{
  if (mSeekInfo.Hand) {
    SeekApply();
    mSeekInfo.Hand = false;
    ReleaseCapture();
  }
}

void ToolsWnd::ClickCancel()
{
  if (mSeekInfo.Hand) {
    mSeekInfo.Hand = false;
    ReleaseCapture();
  }
}

void ToolsWnd::MouseMove(int x, int)
{
  if (mSeekInfo.Hand) {
    SetCursor(mCursorHand);
    BarInfo& info = *mBars[eRecords];
    x -= info.WorkArea.left();
    SeekChange(x);
  } else {
    SetCursor(mCursorArrow);
  }
}

void ToolsWnd::MouseWheel(int x, int y, int wheel)
{
  POINT p = {x, y};
  ScreenToClient(mHwnd, &p);
  x = p.x;
  y = p.y;
  for (int i = 0; i < mBars.size(); i++) {
    BarInfo& info = *mBars[i];
    if (x < info.PosEnd) {
      if (info.Opened) {
        if (info.WorkArea.contains(x, y)) {
          switch (i) {
          case eRecords : MouseWheelRecord(wheel); break;
          }
        }
      }
      break;
    }
  }
}

void ToolsWnd::MouseWheelRecord(int wheel)
{
  if (wheel > 0) {
    while (wheel > 0) {
      mSeekInfo.Scale /= 2;
      wheel--;
    }
    mSeekInfo.Scale = qMax(mSeekInfo.Scale, kScaleMinimum);
  } else {
    while (wheel < 0) {
      mSeekInfo.Scale *= 2;
      wheel++;
    }
    mSeekInfo.Scale = qMin(mSeekInfo.Scale, kScaleMaximum);
  }
  UpdateSeekPeriod();

  UpdateOneBar(eRecords);
}

void ToolsWnd::ClickBtnMute(Button &btn)
{
  bool mute = btn.ImageIndex == 0;
  mute = !mute;
  btn.ImageIndex = (mute)? 0: 1;
  mRender->OnMute(mute);
}

void ToolsWnd::ClickBtnSpeed(Button& btn)
{
  btn.ImageIndex++;
  if (btn.ImageIndex >= kSpeedValues.size()) {
    btn.ImageIndex = 0;
  }
  mSpeed = kSpeedValues[btn.ImageIndex];
  mRender->OnSwitchArchive(mTimestamp, mReverse? -mSpeed: mSpeed, 1);
}

void ToolsWnd::ClickBtnReverse(Button& btn)
{
  mReverse = btn.ImageIndex == 0;
  btn.ImageIndex = (mReverse)? 1: 0;
  mRender->OnSwitchArchive(mTimestamp, mReverse? -mSpeed: mSpeed, 1);
}

void ToolsWnd::ClickBtnLive()
{
  mSpeed = 1;
  mReverse = false;
  mSpeedBtn->ImageIndex = kSpeedDefaultIndex;
  mReverseBtn->ImageIndex = 0;
  mRender->OnSwitchLive();
}

void ToolsWnd::ClickBtnLeftPos()
{
  mLeftTimestamp = mTimestamp;

  UpdateOneBar(eRecords);
}

void ToolsWnd::ClickBtnRightPos()
{
  mRightTimestamp = mTimestamp;

  UpdateOneBar(eRecords);
}

void ToolsWnd::ClickBtnSave()
{
  if (mLeftTimestamp.isValid() && mRightTimestamp.isValid()) {
    mRender->OnDownload(mLeftTimestamp, mRightTimestamp);
    mLeftTimestamp = QDateTime();
    mRightTimestamp = QDateTime();

    UpdateOneBar(eRecords);
  }
}


ToolsWnd::ToolsWnd(Render* _Render, HWND _MainHwnd, bool _PlaySound, EStyleType _Style)
  : mPlaySound(_PlaySound), mStyle(_Style), mRender(_Render), mShow(false)
  , mSpeed(1), mReverse(false)
{
  WNDCLASSEXA wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style         = CS_HREDRAW | CS_VREDRAW;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = NULL;
  wcex.hIcon         = NULL;
  wcex.hCursor       = NULL;
  wcex.hbrBackground = NULL;//(HBRUSH)(CreateSolidBrush(RGB(145,184,74)));
  wcex.lpszMenuName	 = NULL;
  wcex.hIconSm       = NULL;

  wcex.lpfnWndProc   = ToolWndProc;
  wcex.lpszClassName = gClassName;
  if (!RegisterClassExA(&wcex)) {
    Log.Error("RegisterClassExA tools fail");
  }

  mCursorArrow = LoadCursor(NULL, IDC_ARROW);
  mCursorHand = LoadCursor(NULL, IDC_HAND);

  mHwnd = CreateWindowExA(WS_EX_TOOLWINDOW, gClassName, gClassName, WS_CHILD | WS_CLIPSIBLINGS
                          , 0, 0, 800, 40
                          , _MainHwnd, nullptr, nullptr, this);

  InitBars();
}

ToolsWnd::~ToolsWnd()
{
}
