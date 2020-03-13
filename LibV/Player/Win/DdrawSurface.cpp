#include <qglobal.h>

#include <Lib/Log/Log.h>

#include "DdrawSurface.h"
#include "DdrawErrors.h"


bool DdrawSurface::Blt(DdrawSurface &source, RECT *destRect, RECT *sourceRect)
{
  DWORD flag;
  DDBLTFX fx;
  memset(&fx, 0, sizeof(fx));
  fx.dwSize = sizeof(fx);

  if (source.mUseColorKey) {
    fx.ddckSrcColorkey = source.mColorKey;
    flag = DDBLT_KEYSRC | DDBLT_WAIT;
  } else {
    flag = DDBLT_WAIT;
  }
  while (ExecWithRetry("Blt", mSurface->Blt(destRect, source, sourceRect, flag, &fx), &source)) {
  }
  return ExecResult();
}

bool DdrawSurface::Nv12AlphaBlt(const DdrawSurface& _source, int alpha, RECT* destRect)
{
  DdrawSurface& source = const_cast<DdrawSurface&>(_source);
  if (alpha > 100 || alpha < 0) {
    return false;
  }

  if (DDSURFACEDESC* descrDest = Lock()) {
    if (DDSURFACEDESC* descrSrc = source.Lock()) {
      int width = qMin((int)(destRect->right - destRect->left), source.Width());
      int height = qMin((int)(destRect->bottom - destRect->top), source.Height());
      for (int j = 0; j < height; j++) {
        byte* destY = (byte*)descrDest->lpSurface + descrDest->lPitch * (destRect->top + j) + destRect->left;
        const byte* srcY = (byte*)descrSrc->lpSurface + descrSrc->lPitch * (j);
        memcpy(destY, srcY, width);
      }
      for (int j = 0; j < height / 2; j++) {
        byte* destY = (byte*)descrDest->lpSurface + descrDest->lPitch * (Height() + destRect->top /2 + j) + destRect->left;
        const byte* srcY = (byte*)descrSrc->lpSurface + descrSrc->lPitch * (source.Height() + j);
        memcpy(destY, srcY, width);
      }
      //for (int j = 0; j < height; j++) {
      //  byte* destY = (byte*)descrDest->lpSurface + descrDest->lPitch * (destRect->top + j) + destRect->left;
      //  int sj = j * source.Height() / height;
      //  for (int i = 0; i < width; i++) {
      //    int si = i * source.Width() / width;
      //    const byte* srcY = (byte*)descrSrc->lpSurface + descrSrc->lPitch * (sj) + si;
      //    int ys = *srcY;
      //    if (ys) {
      //      int yd = *destY;
      //      *destY = (byte)((yd * (100 - alpha) + ys * alpha) / 100);
      //    }
      //    destY++;
      //  }
      //}
      //for (int j = 0; j < height; j += 2) {
      //  byte* destUV = (byte*)descrDest->lpSurface + descrDest->lPitch * (Height() + (destRect->top + j)/2) + (destRect->left & 0xfffffffe);
      //  int sj = j * source.Height() / height;
      //  for (int i = 0; i < width; i += 2) {
      //    int si = i * source.Width() / width;
      //    const byte* srcY = (byte*)descrSrc->lpSurface + descrSrc->lPitch * (sj) + si;
      //    const byte* srcUV = (byte*)descrSrc->lpSurface + descrSrc->lPitch * (source.Height() + sj/2) + (si & 0xfffffffe);
      //    int ys = *srcY;
      //    if (ys) {
      //      int xd = *destUV;
      //      int xs = *srcUV;
      //      *destUV = (byte)((xd * (100 - alpha) + xs * alpha) / 100);
      //      srcUV++;
      //      destUV++;
      //      xd = *destUV;
      //      xs = *srcUV;
      //      *destUV = (byte)((xd * (100 - alpha) + xs * alpha) / 100);
      //      destUV++;
      //    }
      //  }
      //}

      source.Unlock();
    } else {
      return false;
    }
    Unlock();
  } else {
    return false;
  }
  return true;
}

bool DdrawSurface::Fill(DWORD color)
{
  DDSURFACEDESC* ddsd = Lock();
  if (!ddsd) {
    return false;
  }

  BYTE *hMem = (BYTE *)ddsd->lpSurface;
  for (int j = 0; j < Height(); j++) {
    memset(&hMem[j * ddsd->lPitch], color, ddsd->lPitch);
  }
  for (int j = 0; j < Height() / 2; j++) {
    memset(&hMem[Height() * ddsd->lPitch + j * ddsd->lPitch], 0x80, ddsd->lPitch);
  }
  Unlock();
  return true;
}

void DdrawSurface::DrawMemoryBitmap(char *data, size_t stride)
{
  if (!Lock()) {
    return;
  }

  DrawLockedMemoryBitmap(data, stride);
  Unlock();
}

void DdrawSurface::DrawLockedMemoryBitmap(char* data, size_t stride)
{
  BYTE *hMem = (BYTE *)mDescription.lpSurface;
  if ((size_t)mDescription.lPitch >= stride) {
    for (int j = 0; j < Height(); j++) {
      memcpy(&hMem[j * mDescription.lPitch], data, stride);
      data += stride;
    }
    for (int j = 0; j < Height() / 2; j++) {
      memcpy(&hMem[Height() * mDescription.lPitch + j * mDescription.lPitch], data, stride);
      data += stride;
    }
  }
}

void DdrawSurface::DrawLockedRectangle(int x1, int y1, int x2, int y2, DWORD color)
{
  if (mNv12) {
    uchar y = color & 0xff;
    quint16 uv = (color >> 8) & 0xffff;

    uchar* yData = (uchar*)mDescription.lpSurface;
    uchar* uvData = (uchar*)mDescription.lpSurface + mHeight * mStride;
    memset(yData + x1 + y1 * mStride, y, x2 - x1 + 1);
    memset(yData + x1 + y2 * mStride, y, x2 - x1 + 1);
    for (int j = y1 + 1; j <= y2-1; j++) {
      yData[x1 + j * mStride] = y;
      yData[x2 + j * mStride] = y;
    }

    quint16* lineuv1 = (quint16*)(uvData + y1/2 * mStride) + x1/2;
    quint16* lineuv2 = (quint16*)(uvData + y2/2 * mStride) + x1/2;
    for (int i = x1/2; i <= x2/2; i++) {
      *lineuv1 = uv;
      *lineuv2 = uv;
      lineuv1++;
      lineuv2++;
    }
    for (int j = y1/2 + 1; j <= y2/2 - 1; j++) {
      quint16* lineuv = (quint16*)(uvData + j * mStride);
      lineuv[x1/2] = uv;
      lineuv[x2/2] = uv;
    }
  } else {
    for (int i = x1; i <= x2; i++) {
      PutPixel(i, y1, color);
      PutPixel(i, y2, color);
    }
    for (int j = y1 + 1; j <= y2-1; j++) {
      PutPixel(x1, j, color);
      PutPixel(x2, j, color);
    }
  }
}

DDSURFACEDESC* DdrawSurface::Lock()
{
  RECT region = {0, 0, mWidth, mHeight};
  memset(&mDescription, 0, sizeof(mDescription));
  mDescription.dwSize = sizeof(mDescription);

  while (ExecWithRetry("Lock", mSurface->Lock(&region, &mDescription, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY, NULL))) {
    if (!ExecResult()) {
      return nullptr;
    }
  }

  mNv12 = false;
  mBpp = mDescription.ddpfPixelFormat.dwRGBBitCount / 8;
  if (!mBpp) {
    if (mDescription.ddpfPixelFormat.dwFlags == DDPF_FOURCC && mDescription.ddpfPixelFormat.dwFourCC == 0x3231564e) {/*NV12*/
      mNv12 = true;
      mBpp = 1;
    }
  }
  mStride = mDescription.lPitch;
  return &mDescription;
}

void DdrawSurface::Unlock()
{
  while (ExecWithRetry("Unlock", mSurface->Unlock(mDescription.lpSurface))) {
  }
}

void DdrawSurface::SetColorKey(DWORD color, DWORD flags)
{
  mColorKey.dwColorSpaceLowValue = color;
  mColorKey.dwColorSpaceHighValue = color;
  while (ExecWithRetry("SetColorKey", mSurface->SetColorKey(flags, &mColorKey))) {
  }
  if (ExecResult()) {
    mUseColorKey = true;
  }
}

void DdrawSurface::SetValid()
{
  mValid = true;
}

void DdrawSurface::Invalidate()
{
  mValid = false;
}

bool DdrawSurface::ExecWithRetry(const char *functionName, HRESULT retCode, DdrawSurface *sourceSurface)
{
  bool retry = false;
  if (retCode == DD_OK) {
    mError = false;
    return false;
  } else if (retCode == DDERR_SURFACELOST) {
    mError = true;
    if (mSurface->IsLost() == DDERR_SURFACELOST) {
      if (mSurface->Restore() == DD_OK) {
        Invalidate();
        Log.Trace("Surface restored");
        retry = true;
      }
    }
    if (sourceSurface) {
      DdrawSurface& source = *sourceSurface;
      if (source->IsLost() == DDERR_SURFACELOST) {
        if (source->Restore() == DD_OK) {
          source.Invalidate();
          Log.Trace("Surface source restored");
          retry = true;
        }
      }
    }
  } else {
    if (!mError) {
      int count = ++mWarningsCount[retCode];
      if (count > mWarningsLog[retCode]) {
        mWarningsLog[retCode] = 2 * count + 20;
        Log.Warning(QString("Surface %1 fail (code: %2, cnt: %3)").arg(functionName).arg(DdErrorString(retCode)).arg(count));
      }
      mError = true;
      mErrorRetryCounter = 0;
    }
    if (++mErrorRetryCounter >= 13) {
      mErrorRetryCounter = 0;
      return false;
    }
    mErrorCode = retCode;
  }
  return retry;
}

bool DdrawSurface::ExecResult()
{
  return !mError;
}

HRESULT DdrawSurface::ExecErrorCode()
{
  return mError? mErrorCode: 0;
}


DdrawSurface::DdrawSurface(IDirectDrawSurface *_Surface, int _Width, int _Height)
  : mSurface(_Surface), mWidth(_Width), mHeight(_Height), mUseColorKey(false), mError(false), mValid(false)
{
  if (mSurface) {
    mSurface->AddRef();
  }
}

DdrawSurface::DdrawSurface()
  : mSurface(nullptr), mWidth(0), mHeight(0), mUseColorKey(false), mError(false), mValid(false)
{
}

DdrawSurface::DdrawSurface(const DdrawSurface &copy)
  : mSurface(copy.mSurface), mWidth(copy.mWidth), mHeight(copy.mHeight), mUseColorKey(copy.mUseColorKey)
  , mColorKey(copy.mColorKey), mError(false), mValid(copy.mValid)
{
  if (mSurface) {
    mSurface->AddRef();
  }
}

DdrawSurface::~DdrawSurface()
{
  if (mSurface) {
    mSurface->Release();
    mSurface = nullptr;
  }
}

