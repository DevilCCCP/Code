#include <SDL.h>

#include <Lib/Log/Log.h>
#include <LibV/Va/Va.h>

#include "SdlDrawer.h"
#include "WndProcL.h"


void QtDrawer::SetFrame(FrameS &frame)
{
//  const char* data = frame->VideoData();
//  int     dataSize = frame->VideoDataSize();
//  int     width    = frame->GetHeader()->Width;
//  int     height   = frame->GetHeader()->Height;
//  int     stride   = (dataSize / 3 * 2) / height;

//  QImage image(width, height, QImage::Format_Grayscale8);
//  for (int j = 0; j < height; j++) {
//    char* line = (char*)image.scanLine(j);
//    const char* dataLine = data + j * stride;
//    memcpy(line, dataLine, width * 1);
//  }

  const char* data = frame->VideoData();
  int     dataSize = frame->VideoDataSize();
  int     width    = frame->GetHeader()->Width;
  int     height   = frame->GetHeader()->Height;
  int     stride   = (dataSize / 3 * 2) / height;

  char* rgbData = new char[width * height * 4];
  for (int j = 0; j < height; j++) {
    const char* d = data + j*stride;
    char* rd = rgbData + j * 4*width;
    for (int i = 0; i < width; i++) {
      *rd++ = *d;
      *rd++ = *d;
      *rd++ = *d;
      *rd++ = 0;
      d++;
    }
  }
  if (SDL_Window* window = static_cast<WndProcL*>(Windows())->VideoWindow()) {
    if (!mFrameSurface) {
      mFrameSurface = SDL_CreateRGBSurfaceFrom((void*)rgbData, width, height, 32, 4*width, 0x0000ff, 0x0000ff00, 0x00ff0000, 0);
      if (!mFrameSurface) {
        Log.Error(QString("Create SDL surface fail"));
        return;
      }
    }

    SDL_Surface* screen = SDL_GetWindowSurface(window);
    SDL_BlitSurface(mFrameSurface, nullptr, screen, nullptr);
    mFrameSurface = nullptr;
    SDL_UpdateWindowSurface(window);
  }
  delete[] rgbData;
}

void QtDrawer::SetStatusFrame(FrameS& frame)
{
  Q_UNUSED(frame);

}

void QtDrawer::SetStatus(EDrawStatus status)
{
}

void QtDrawer::Redraw()
{
}

void QtDrawer::Clear()
{
}

void QtDrawer::SetZoom(int scale)
{
}

void QtDrawer::MoveZoom(const QPointF& pos)
{
}

bool QtDrawer::Draw()
{
  return true;
}

bool QtDrawer::PrepareDraw()
{
  return true;
}

bool QtDrawer::PrepareScene(const QRect& destRect)
{
  return true;
}


QtDrawer::QtDrawer(WndProcAS& _MainWindow, const char** _StatusIcons, bool _ScaleBest)
  : DeviceDrawer(_MainWindow, _StatusIcons, _ScaleBest)
  , mCurrentStatus(eDrawStatusIllegal)
  , mZoomScale(1), mZoomPos(0, 0), mError(false), mHasStatusFrame(false)
  , mFrameSurface(nullptr)
{
}
