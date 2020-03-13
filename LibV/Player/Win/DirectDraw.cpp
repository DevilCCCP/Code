#include <Lib/Log/Log.h>

#include "DirectDraw.h"
#include "DdrawErrors.h"


DdrawSurface DirectDraw::CreateSurface(int width, int height)
{
  DDSURFACEDESC ddsd;
  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
  ddsd.dwWidth = width;
  ddsd.dwHeight = height;

  {
    //ddsd.dwFlags |= DDSD_PIXELFORMAT;
    //ddsd.ddpfPixelFormat.dwSize = sizeof(ddsd.ddpfPixelFormat);
    ////ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
    ////ddsd.ddpfPixelFormat.dwFourCC = format;
    //ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
    //ddsd.ddpfPixelFormat.dwRGBBitCount = 24;
    //ddsd.ddpfPixelFormat.dwRBitMask	= 0xFF0000;
    //ddsd.ddpfPixelFormat.dwGBitMask	= 0xFF00;
    //ddsd.ddpfPixelFormat.dwBBitMask	= 0xFF;

    ddsd.dwFlags |= DDSD_PIXELFORMAT;
    ddsd.ddpfPixelFormat.dwSize = sizeof(ddsd.ddpfPixelFormat);
    ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
    ddsd.ddpfPixelFormat.dwFourCC = 0x3231564E;
  }

  ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
  LPDIRECTDRAWSURFACE dds;
  HRESULT err = mDdraw->CreateSurface(&ddsd, &dds, NULL);
  if (err != DD_OK) {
    if (!mCreateSurfaceLastFail) {
      Log.Warning(QString("CreateSurface fail (w: %1, h: %2, code: %3").arg(width).arg(height).arg(DdErrorString(err)));
      mCreateSurfaceLastFail = true;
    }
    return DdrawSurface();
  }

  return DdrawSurface(dds, width, height);
}

DdrawSurface DirectDraw::CreateSurfaceFromFile(const char *filename, bool autoKey)
{
  FILE* file;
  fopen_s(&file, filename, "rb");
  if (!file) {
    Log.Warning(QString("DirectDraw: file open fail (%1)").arg(filename));
    return DdrawSurface();
  }

  BITMAPFILEHEADER bfh;
  BITMAPINFOHEADER bih;
  std::vector<RGBQUAD> rgb;
  std::vector<char> bitmap;
  bool fail = true;
  do
  {
    if (!fread(&bfh, sizeof(bfh), 1, file)) {
      break;
    }
    if (bfh.bfType !=	*(WORD*)"BM") {
      break;
    }
    if (!fread(&bih, sizeof(bih), 1, file)) {
      break;
    }
    if (bih.biPlanes != 1) {
      break;
    }

    if (bih.biBitCount < 16) {
      if (bih.biClrUsed == 0) {
        bih.biClrUsed = 1 << bih.biBitCount;
      }
      rgb.resize(bih.biClrUsed + 1);
      if (fread(rgb.data(), sizeof(RGBQUAD), rgb.size(), file) != rgb.size()) {
        break;
      }
    }

    switch (bih.biBitCount) { // align 4 bytes
    case 24 : bih.biSizeImage = (bih.biWidth*3 + (4-((bih.biWidth*3)%4))%4) * bih.biHeight; break;
    case 16 : bih.biSizeImage = (bih.biWidth*2 + (4-((bih.biWidth*2)%4))%4) * bih.biHeight; break;
    case  8 : bih.biSizeImage = (bih.biWidth   + (4-((bih.biWidth  )%4))%4) * bih.biHeight; break;
    case  4 : bih.biSizeImage = (bih.biWidth/2 + (4-((bih.biWidth/2)%4))%4) * bih.biHeight; break;
    }

    bitmap.resize(bih.biSizeImage);
    if (fread(bitmap.data(), 1, bitmap.size(), file) != bitmap.size()) {
      break;
    }
    fail = false;
  } while (fail);

  fclose(file);
  if (fail) {
    Log.Warning(QString("DirectDraw: file format fail (%1)").arg(filename));
    return DdrawSurface();
  }

  auto surface = CreateSurface(bih.biWidth, bih.biHeight);
  if (!surface) {
    return surface;
  }
  if (!surface.Lock()) {
    return surface;
  }

  DWORD colorKey = 0xFF000000;
  size_t n = 0;
  for (int j = bih.biHeight - 1; j >= 0; j--)
  {
    for (int i = 0; i < bih.biWidth; i++)
    {
      DWORD colR, colG, colB;
      switch (bih.biBitCount)
      {
      case 24:
        colR = bitmap[n+2];
        colG = bitmap[n+1];
        colB = bitmap[n];
        n += 3;
        break;

      case 16:
        n += 2;
        break;

      case 8:
        colR = rgb[bitmap[n]].rgbRed;
        colG = rgb[bitmap[n]].rgbGreen;
        colB = rgb[bitmap[n]].rgbBlue;
        n ++;
        break;

      case 4:
        size_t col;
        if (n % 2) {
          col = bitmap[n/2] & 0x0f;
        } else {
          col = (bitmap[n/2] & 0x0f) >> 4;
        }
        colR = rgb[col].rgbRed;
        colG = rgb[col].rgbGreen;
        colB = rgb[col].rgbBlue;
        n ++;
        break;

      default:
        break;
      }

      DWORD color = surface.GetColor(colR, colG, colB);
      if (i == 0 && j == bih.biHeight - 1) colorKey = color;
      if (autoKey && color == colorKey) surface.PutPixel(i, j, 0xFF000000);
      else surface.PutPixel(i, j, color);

    } //for i

    n += (4 - ((bih.biWidth * bih.biBitCount / 8) % 4)) % 4; // skip align

  } //for j

  surface.Unlock();
  if (autoKey) {
    surface.SetColorKey(0xFF000000);
  }
  return surface;
}

bool DirectDraw::Blt(DdrawSurface &source, const RECT *destRect)
{
  return mMainSurface.Blt(source, const_cast<LPRECT>(destRect));
}

bool DirectDraw::Fill(DWORD color, const RECT *destRect)
{
  DDBLTFX fx;
  memset(&fx, 0, sizeof(fx));
  fx.dwSize = sizeof(fx);
  fx.dwFillColor = color;

  int count = 0;
  do {
    auto err = mMainSurface->Blt(const_cast<LPRECT>(destRect), NULL, NULL, DDBLT_COLORFILL, &fx);
    if (err == DD_OK) {
      return true;
    } else if (err == DDERR_SURFACELOST) {
      mMainSurface->Restore();
    } else if (err == DDERR_SURFACEBUSY) {
    } else {
      Log.Warning(QString("DirectDraw: blt error: %1").arg(err));
      return false;
    }
  } while (count < 10);
  Log.Warning("DirectDraw: blt all tries fail");
  return false;
}

DirectDraw::DirectDraw(HWND hwnd, bool useClipper)
  : mDdraw(nullptr), mDdMainSurface(nullptr), mMainClipper(nullptr)
  , mCreateSurfaceLastFail(false)
{
  if (DirectDrawCreate(NULL, &mDdraw, NULL) != DD_OK) {
    Log.Fatal("DirectDraw: create fail", true);
  }

  DDSURFACEDESC ddsd;
  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_CAPS;
  ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

  if (HRESULT err = mDdraw->SetCooperativeLevel(hwnd, DDSCL_NORMAL) != DD_OK) {
    Log.Fatal(QString("DirectDraw: SetCooperativeLevel fail (code: %1)").arg(DdErrorString(err)), true);
  }
  if (HRESULT err = mDdraw->CreateSurface(&ddsd, &mDdMainSurface, NULL) != DD_OK) {
    Log.Fatal(QString("DirectDraw: CreateSurface fail (code: %1)").arg(DdErrorString(err)), true);
  }

  if (!useClipper
      || mDdraw->CreateClipper(0, &mMainClipper, NULL) != DD_OK
      || mMainClipper->SetHWnd(0, hwnd) != DD_OK
      || mDdMainSurface->SetClipper(mMainClipper) != DD_OK) {
    mMainClipper = nullptr;
  }

  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_CAPS;
  ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

  if (HRESULT err = mDdMainSurface->GetSurfaceDesc(&ddsd) != DD_OK) {
    Log.Fatal(QString("DirectDraw: create main surface fail (code: %1)").arg(DdErrorString(err)), true);
  }
  mMainSurface = DdrawSurface(mDdMainSurface, 0, 0);

  mBpp = 0;
  if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) {
    mBpp = 8;
  } else if (ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB) {
    mBpp = ddsd.ddpfPixelFormat.dwRGBBitCount;
  }
  mBpp = (mBpp + 7) / 8;
}

DirectDraw::~DirectDraw()
{
  if (mMainClipper) mMainClipper->Release();
  mMainSurface = DdrawSurface();
  if (mDdraw) mDdraw->Release();
}
