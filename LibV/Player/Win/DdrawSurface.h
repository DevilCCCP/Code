#pragma once

#include <QMap>
#include <memory>
#include <ddraw.h>
#undef SendMessage


DefineClassS(DdrawSurface);

class DdrawSurface
{
  IDirectDrawSurface* mSurface;
  int                 mBpp;
  int                 mWidth;
  int                 mHeight;
  int                 mStride;
  bool                mNv12;
  DDSURFACEDESC       mDescription;
  bool                mUseColorKey;
  DDCOLORKEY          mColorKey;
  bool                mError;
  int                 mErrorRetryCounter;
  HRESULT             mErrorCode;
  bool                mValid;
  QMap<int, int>      mWarningsCount;
  QMap<int, int>      mWarningsLog;

public:
  bool IsValid() { return mValid; }
  operator bool() const { return !!mSurface; }
  bool operator !() const { return !mSurface; }
  IDirectDrawSurface* operator->() const { return mSurface; }
  operator IDirectDrawSurface*() const { return mSurface; }
  int Width() const { return mWidth; }
  int Height() const { return mHeight; }
  int Bpp() const { return mBpp; }
  bool UseColorKey() const { return mUseColorKey; }
  DDCOLORKEY ColorKey() const { return mColorKey; }

public:
  bool Blt(DdrawSurface &source, RECT* destRect = NULL, RECT* sourceRect = NULL);
  bool Nv12AlphaBlt(const DdrawSurface& source, int alpha, RECT* destRect = NULL);

  bool Fill(DWORD color);

  void DrawMemoryBitmap(char *data, size_t stride);
  void DrawLockedMemoryBitmap(char *data, size_t stride);

  void DrawLockedRectangle(int x1, int y1, int x2, int y2, DWORD color);

  DDSURFACEDESC* Lock();
  void Unlock();

  void SetColorKey(DWORD color, DWORD flags = DDCKEY_SRCBLT);
  void SetValid();

public: // Use only after lock
  void PutPixel(int x, int y, DWORD col)
  { memcpy(reinterpret_cast<char*>(mDescription.lpSurface) + x * mBpp + y * mStride, &col, mBpp); }

  DWORD GetColor(DWORD colR, DWORD colG, DWORD colB)
  {
    DWORD Col;
    DWORD MaskR = mDescription.ddpfPixelFormat.dwRBitMask;
    DWORD MaskG = mDescription.ddpfPixelFormat.dwGBitMask;
    DWORD MaskB = mDescription.ddpfPixelFormat.dwBBitMask;
    Col = (DWORD) (MaskR * (colR/255.)) & MaskR;
    Col |= (DWORD) (MaskG * (colG/255.)) & MaskG;
    Col |= (DWORD) (MaskB * (colB/255.)) & MaskB;
    return 0xFF000000 | Col;
  }

private:
  void Invalidate();
  bool ExecWithRetry(const char* functionName, HRESULT retCode, DdrawSurface* sourceSurface = nullptr);
  bool ExecResult();
  HRESULT ExecErrorCode();

public:
  DdrawSurface(IDirectDrawSurface* _Surface, int _Width, int _Height);
  DdrawSurface();
  DdrawSurface(const DdrawSurface& copy);

  ~DdrawSurface();
};
