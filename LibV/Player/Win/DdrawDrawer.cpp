#include <Lib/Log/Log.h>
#include <LibV/Va/Va.h>

#include "DdrawDrawer.h"
#include "WndProc.h"


void DdrawDrawer::SetFrame(FrameS &frame)
{
  mHasStatusFrame = false;

  if (PrepareDraw()) {
    if (!mBackFrame || mBackFrame.Width() != frame->GetHeader()->Width || mBackFrame.Height() != frame->GetHeader()->Height) {
      mBackFrame = mDirectDraw->CreateSurface(frame->GetHeader()->Width, frame->GetHeader()->Height);
    }
    if (mBackFrame) {
      Object* objects = reinterpret_cast<Object*>(frame->ObjectData());
      int objSize = frame->ObjectDataSize() / sizeof(Object);
      if (mBackFrame.Lock()) {
        mBackFrame.DrawLockedMemoryBitmap(frame->VideoData(), frame->GetHeader()->Width);
        for (int i = 0; i < objSize; i++) {
          const Object& obj = objects[i];
          Rectangle rect = obj.Dimention;
          int colInd = obj.Color / 1000;
          int colValue = qMin(100, obj.Color % 1000) * 255 / 100;
          int r, g, b;
          switch (colInd) {
          case 1:  r = 0; g = colValue; b = 0; break;
          case 2:  r = colValue; g = 0; b = 0; break;
          case 3:  r = 0; g = 0; b = colValue; break;
          default: r = colValue; g = colValue; b = colValue; break;
          }
          int y = (299*r + 587*g + 114*b) / 1000;
          int u = (-169*r - 331*g + 500*b) / 1000 + 128;
          int v = (500*r - 419*g - 81*b) / 1000 + 128;
          DWORD col = (((uint)v & 0xff) << 16) | (((uint)u & 0xff) << 8) | ((uint)y & 0xff);

          rect.Left = qMax(0, rect.Left);
          rect.Top = qMax(0, rect.Top);
          rect.Right = qBound(rect.Left, rect.Right, mBackFrame.Width() - 1);
          rect.Bottom = qBound(rect.Top, rect.Bottom, mBackFrame.Height() - 1);
          mBackFrame.DrawLockedRectangle(rect.Left, rect.Top, rect.Right, rect.Bottom, col);
        }
        mBackFrame.Unlock();
      }
    }

    if (!Draw()) {
      if (!mError) {
        Log.Warning("Drawing lost");
      }
      mError = true;
    } else if (mError) {
      Log.Info("Drawing restored");
      mError = false;
    }
  }
}

void DdrawDrawer::SetStatusFrame(FrameS& frame)
{
  Q_UNUSED(frame);

  if (mHasStatusFrame && !mError) {
    return;
  }

  mHasStatusFrame = true;

  if (PrepareDraw()) {
    if (!Draw()) {
      if (!mError) {
        Log.Warning("Drawing lost");
      }
      mError = true;
    } else if (mError) {
      Log.Info("Drawing restored");
      mError = false;
    }
  }
}

void DdrawDrawer::SetStatus(EDrawStatus status)
{
  mCurrentStatus = status;
  if (PrepareDraw() && mStatusIcons[mCurrentStatus]) {
    Draw();
  }
}

void DdrawDrawer::Redraw()
{
  if (PrepareDraw()) {
    Draw();
  }
}

void DdrawDrawer::Clear()
{
  if (PrepareDraw()) {
    if (mBackFrame) {
      mBackFrame.Fill(0);
      Draw();
    }
  }
}

void DdrawDrawer::SetZoom(int scale)
{
  mZoomScale = qMax(scale, 1);
}

void DdrawDrawer::MoveZoom(const QPointF& pos)
{
  if (mZoomScale > 1) {
    qreal zMax = (mZoomScale - 1) * 0.5;
    mZoomPos.rx() = qBound(-zMax, pos.x(), zMax);
    mZoomPos.ry() = qBound(-zMax, pos.y(), zMax);
  } else {
    mZoomPos = QPointF(0, 0);
  }
}

bool DdrawDrawer::Draw()
{
  RECT rect;
  if (!GetWindowRect(mMainWindow, &rect)) {
    return false;
  }
  RECT rect2;
  if (!GetClientRect(mMainWindow, &rect2)) {
    return false;
  }
  int sceneWidth = rect2.right - rect2.left;
  int sceneHeight = rect2.bottom - rect2.top;
  if (!mBackScene || mBackScene.Width() != sceneWidth || mBackScene.Height() != sceneHeight) {
    mLastDestRect = QRect(0, 0, 0, 0);
    mBackScene = mDirectDraw->CreateSurface(sceneWidth, sceneHeight);

    if (mBackScene) {
      Log.Info(QString("Create back surface (w: %1, h: %2)").arg(sceneWidth).arg(sceneHeight));
    } else {
      return false;
    }
  }

  if (mBackFrame) {
    float kx = (float)mBackScene.Width() / (float)mBackFrame.Width() * mZoomScale;
    float ky = (float)mBackScene.Height() / (float)mBackFrame.Height() * mZoomScale;
    if (ScaleBest()) {
      kx = ky = qMin(kx, ky);
    }
    int w = (int)(kx * mBackFrame.Width() + 0.5f);
    int h = (int)(ky * mBackFrame.Height() + 0.5f);

    RECT destRect;
    destRect.left   = (int)(mBackScene.Width()/2 - w * (mZoomPos.x() + 0.5) + 0.5);
    destRect.top    = (int)(mBackScene.Height()/2 - h * (mZoomPos.x() + 0.5) + 0.5);
    destRect.right  = destRect.left + w;
    destRect.bottom = destRect.top + h;
    if (mZoomScale <= 1) {
      PrepareScene(destRect);
      mBackScene.Blt(mBackFrame, &destRect);
    } else {
      destRect.left   = (int)(mBackScene.Width() * (mZoomPos.x() + 0.5) - w/2 + 0.5);
      destRect.right  = (int)(mBackScene.Width() * (mZoomPos.x() + 0.5) + (w+1)/2 + 0.5);
      destRect.top    = (int)(mBackScene.Height() * (mZoomPos.y() + 0.5) - h/2 + 0.5);
      destRect.bottom = (int)(mBackScene.Height() * (mZoomPos.y() + 0.5) + (h+1)/2 + 0.5);

      RECT srcRect;
      srcRect.left   = (destRect.left < 0)? (int)(-destRect.left / kx): 0;
      srcRect.right  = (destRect.right > mBackScene.Width())? mBackFrame.Width() - (int)((destRect.right - mBackScene.Width()) / kx): mBackFrame.Width();
      srcRect.top    = (destRect.top < 0)? (int)(-destRect.top / ky): 0;
      srcRect.bottom = (destRect.bottom > mBackScene.Height())? mBackFrame.Height() - (int)((destRect.bottom - mBackScene.Height()) / ky): mBackFrame.Height();

      destRect.left   = qMax(0, (int)destRect.left);
      destRect.right  = qMin(mBackScene.Width(), (int)destRect.right);
      destRect.top    = qMax(0, (int)destRect.top);
      destRect.bottom = qMin(mBackScene.Height(), (int)destRect.bottom);
      PrepareScene(destRect);
      mBackScene.Blt(mBackFrame, &destRect, &srcRect);
    }
  } else {
    RECT destRect;
    destRect.left   = -1;
    destRect.top    = -1;
    destRect.right  = 1;
    destRect.bottom = 1;
    PrepareScene(destRect);
  }

  if (mHasStatusFrame && mStatusFrame.IsValid()) {
    RECT destRect;
    int w = mBackScene.Width();
    int h = mBackScene.Height();
    int width = qMin(w, h) * 1/2;
    destRect.left = mBackScene.Width()/2 - width/2;
    destRect.top = mBackScene.Height()/2 - width/2;
    destRect.right = destRect.left + width;
    destRect.bottom = destRect.top + width;
    //mBackScene.Nv12AlphaBlt(mStatusIcons[mCurrentStatus], 50, &destRect);
    mBackScene.Blt(mStatusFrame, &destRect);
  }

  if (DdrawSurface* iconSurface = GetIcon(mCurrentStatus)) {
    RECT destRect;
    int w = mBackScene.Width();
    int h = mBackScene.Height();
    int width = qMin(qMin(w, h) * 1/20, 40);
    destRect.left = mBackScene.Width() - width - 10;
    destRect.top = 10;
    destRect.right = destRect.left + width;
    destRect.bottom = destRect.top + width;
    //mBackScene.Nv12AlphaBlt(mStatusIcons[mCurrentStatus], 50, &destRect);
    mBackScene.Blt(*iconSurface, &destRect);
  }
  return mDirectDraw->Blt(mBackScene, &rect);
}

bool DdrawDrawer::PrepareDraw()
{
  if (HWND window = static_cast<WndProc*>(Windows())->GetDrawWnd()) {
    if (window != mMainWindow) {
      mMainWindow = window;
      mBackScene = DdrawSurface();
      mDirectDraw = DirectDrawS(new DirectDraw(mMainWindow));
      Log.Info("Create DirectDraw device");

      LoadIcons();
      SurfaceFromImage(":/Icons/No Video.png", mStatusFrame);
    }
    return mDirectDraw;
  }
  return false;
}

bool DdrawDrawer::PrepareScene(const RECT& destRect)
{
  QRect currentRect(QPoint(destRect.left, destRect.top), QPoint(destRect.bottom, destRect.right));
  if (currentRect != mLastDestRect) {
    if (!mBackScene.Fill(0)) {
      return false;
    }
    mLastDestRect = currentRect;
  }
  return true;
}

DdrawSurface *DdrawDrawer::GetIcon(int ind)
{
  if (ind >= eDrawStatusIllegal) {
    return nullptr;
  }
  if (ind >= mStatusIcons.size() || !mStatusIcons[ind] || !mStatusIcons[ind].IsValid()) {
    if (!LoadIcons()) {
      return nullptr;
    } else {
      Log.Info("DirectDraw: icons reloaded");
    }
  }
  return &mStatusIcons[ind];
}

bool DdrawDrawer::LoadIcons()
{
  mStatusIcons.clear();
  for (int ind = 0; StatusIcon(ind); ind++) {
    DdrawSurface surface;
    if (!SurfaceFromImage(StatusIcon(ind), surface)) {
      return false;
    }
    mStatusIcons.append(surface);
  }
  return true;
}

bool DdrawDrawer::SurfaceFromImage(const QString& path, DdrawSurface& surface)
{
  QImage image(path);
  if (image.isNull()) {
    Log.Warning(QString("Load status icon fail (path: %1)").arg(path));
    return false;
  }

  surface = mDirectDraw->CreateSurface(image.width(), image.height());
  if (!surface) {
    return false;
  }
  DWORD colorKey = qRgb(0, 0, 0);
  surface.SetColorKey(colorKey);
  if (DDSURFACEDESC* descr = surface.Lock()) {
    for (int j = 0; j < image.height(); j++) {
      const QRgb* data = reinterpret_cast<const QRgb*>(image.constScanLine(j));
      char* dest = (char*)descr->lpSurface + descr->lPitch * j;
      for (int i = 0; i < image.width(); i++) {
        if (qAlpha(*data) < 50) {
          *dest = 0;
        } else {
          int r = qRed(*data);
          int g = qGreen(*data);
          int b = qBlue(*data);
          *dest = (byte)((0.257f * r) + (0.504f * g) + (0.098f * b) + 16);
        }
        data++;
        dest++;
      }
    }
    for (int j = 0; j < image.height(); j += 2) {
      const QRgb* data = reinterpret_cast<const QRgb*>(image.constScanLine(j));
      char* dest = (char*)descr->lpSurface + descr->lPitch * (j/2 + image.height());
      for (int i = 0; i < image.width(); i += 2) {
        if (qAlpha(*data) < 50) {
          *dest++ = 127;
          *dest++ = 127;
        } else {
          int r = qRed(*data);
          int g = qGreen(*data);
          int b = qBlue(*data);
          *dest++ = (byte)(-(0.148 * r) - (0.291 * g) + (0.439 * b) + 128);
          *dest++ = (byte)((0.439f * r) - (0.368f * g) - (0.071f * b) + 128);
        }
        data += 2;
      }
    }
    surface.Unlock();
  }

  surface.SetValid();
  return true;
}


DdrawDrawer::DdrawDrawer(WndProcAS& _MainWindow, const char** _StatusIcons, bool _ScaleBest)
  : DeviceDrawer(_MainWindow, _StatusIcons, _ScaleBest)
  , mMainWindow(nullptr), mCurrentStatus(eDrawStatusIllegal)
  , mZoomScale(1), mZoomPos(0, 0), mError(false), mHasStatusFrame(false)
{
}
