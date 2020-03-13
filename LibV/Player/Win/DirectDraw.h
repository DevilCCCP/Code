#pragma once

#include <memory>
#include <string>
#include <ddraw.h>

#include <Lib/Include/Common.h>

#include "DdrawSurface.h"

DefineClassS(DirectDraw);

class DirectDraw
{
private:
  IDirectDraw*        mDdraw;
  DdrawSurface        mMainSurface;
  IDirectDrawSurface* mDdMainSurface;
  IDirectDrawClipper* mMainClipper;

  int                 mBpp;
  int                 mCreateSurfaceLastFail;

public:
  DdrawSurface GetMainSurface() { return mMainSurface; }

public:
  DdrawSurface CreateSurface(int width, int height);
  DdrawSurface CreateSurfaceFromFile(const char* filename, bool autoKey = true);

  bool Blt(DdrawSurface& source, const RECT* destRect = NULL);
  bool Fill(DWORD color, const RECT* destRect = NULL);

public:
  DirectDraw(HWND hwnd, bool useClipper = true);
  ~DirectDraw();
private:
  DirectDraw(const DirectDraw& copy);
};
