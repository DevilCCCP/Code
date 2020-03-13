#pragma once

class GdiplusEngine {
  Gdiplus::GdiplusStartupInput mGdiplusStartupInput;
  ULONG_PTR mToken;
public:
  operator bool() { return !!mToken; }
  GdiplusEngine()
  {
    ULONG_PTR token;
    if (Gdiplus::GdiplusStartup(&token, &mGdiplusStartupInput, NULL) == Gdiplus::Ok) {
    } else {
      mToken = 0;
    }
  }
  ~GdiplusEngine() { Gdiplus::GdiplusShutdown(mToken); }
};
