#include "ImgAnalizer.h"
#include "ObjConnect.h"
#include "Uin.h"
#include "SignalMark2.h"


void ImgAnalizer::Init(const uchar* data, int width, int height, int stride)
{
  mWidth  = width;
  mHeight = height;
  mSrc.SetSource(const_cast<uchar*>(data), mWidth, mHeight, stride);
  mDst.SetSize(mWidth, mHeight);
}

void ImgAnalizer::Init(const Region<uchar>& region)
{
  mWidth  = region.Width();
  mHeight = region.Height();
  mSrc.SetSource(region);
  mDst.SetSize(mWidth, mHeight);
}

void ImgAnalizer::Init(const Region<uchar>& region, int x, int y, int width, int height)
{
  mWidth  = width;
  mHeight = height;
  mSrc.SetSource(region, x, y, mWidth, mHeight);
  mDst.SetSize(mWidth, mHeight);
}

void ImgAnalizer::MakeGrad()
{
  mHyst.Clear();
  for (int j = 0; j < Height() - 1; j++) {
    const uchar* srcnn = mSrc.Line(j);
    const uchar* srcnp = mSrc.Line(j + 1);
    const uchar* srcpn = mSrc.Line(j) + 1;
    uchar* img = mDst.Line(j);
    for (int i = 0; i < Width() - 1; i++) {
      int h = qAbs((int)*srcnn - (int)*srcpn);
      int v = qAbs((int)*srcnn - (int)*srcnp);
      int d = qMax(h, v);
      d = (d > 7)? qMin(255, 127 + d): 0;
      mHyst.Inc(d);
      *img = (uchar)(uint)d;

      srcnn++;
      srcnp++;
      srcpn++;
      img++;
    }
    *img = 0;
  }
  memset(mDst.Line(Height() - 1), 0, Width());
}

void ImgAnalizer::Make2Color(int minPerc, int maxPerc)
{
  int med = mHyst.GetMidValue(minPerc, maxPerc);

  for (int j = 0; j < Height(); j++) {
    uchar* dst = mDst.Line(j);
    for (int i = 0; i < Width(); i++) {
      if (*dst < med) {
        *dst = 0;
      } else {
        *dst = 255;
      }

      dst++;
    }
  }
}

void ImgAnalizer::MakeMedian(int len)
{
  QVector<int> values;
  for (int j = 0; j < Height(); j++) {
    uchar* dst = mDst.Line(j);
    for (int i = 0; i < Width(); i++) {
      values.clear();
      for (int jj = qMax(0, j - len); jj <= qMin(Height() - 1, j + len); jj++) {
        for (int ii = qMax(0, i - len); ii <= qMin(Width() - 1, i + len); ii++) {
          values.append((int)*mSrc.Data(ii, jj));
        }
      }
      std::sort(values.begin(), values.end());
      *dst = values.at(values.size()/2);

      dst++;
    }
  }
}

void ImgAnalizer::MakeLower(int len)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dst = mDst.Line(j);
    for (int i = 0; i < Width(); i++) {
      uchar minValue = 255;
      for (int jj = qMax(0, j - len); jj <= qMin(Height() - 1, j + len); jj++) {
        for (int ii = qMax(0, i - len); ii <= qMin(Width() - 1, i + len); ii++) {
          minValue = qMin(minValue, *mSrc.Data(ii, jj));
        }
      }
      *dst = minValue;

      dst++;
    }
  }
}

void ImgAnalizer::MakeHigher(int len)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dst = mDst.Line(j);
    for (int i = 0; i < Width(); i++) {
      uchar maxValue = 0;
      for (int jj = qMax(0, j - len); jj <= qMin(Height() - 1, j + len); jj++) {
        for (int ii = qMax(0, i - len); ii <= qMin(Width() - 1, i + len); ii++) {
          maxValue = qMax(maxValue, *mSrc.Data(ii, jj));
        }
      }
      *dst = maxValue;

      dst++;
    }
  }
}

void ImgAnalizer::CalcHyst(int x, int y, int width, int height)
{
  mHyst.Clear();
  for (int j = 0; j < height; j++) {
    const uchar* src = mSrc.Data(x, y + j);
    for (int i = 0; i < width; i++) {
      mHyst.Inc(*src);
      src++;
    }
  }
}

bool ImgAnalizer::FindUinRu(const Region<uchar>& region)
{
  if (!mSignalMark2 && !UinPreInit()) {
    return false;
  }

  mSignalMark2->Calc(&region);
  return true;
}

bool ImgAnalizer::UinInit()
{
  mUin.reset(new Uin(this));
  const char kSymbols[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
                            , 'A', 'B', 'E','K', 'M', 'H', 'O', 'P', 'C', 'T', 'Y', 'X', '\0' };
  for (int i = 0; kSymbols[i]; i++) {
    QImage img(QString(":/UinFont/Uin %1.png").arg(kSymbols[i]));
    if (!img.isNull()) {
      mUin->AddChar(QChar(kSymbols[i]), img);
    }
  }
  return true;
}

bool ImgAnalizer::UinPreInit()
{
  mSignalMark2.reset(new SignalMark2());
  return true;
}

bool ImgAnalizer::UinPrepareRu(const Region<uchar>& region)
{
  if (region.Width() < 64 || region.Height() < 16) {
    return false;
  }

  if (!mUin && !UinInit()) {
    return false;
  }

  Init(region);
  CalcHyst(16, 8, mWidth - 32, mHeight - 16);
  int minValue  = mHyst.GetValue(300);
  int maxValue  = mHyst.GetValue(400);
  int minExpect = mHyst.GetLocalMax(0, minValue);
  int maxExpect = mHyst.GetLocalMax(maxValue, 255);

  mBlackThreshold      = (2*minExpect + maxExpect)/3;
  mWhiteThreshold      = (minExpect + 2*maxExpect)/3;
  mBackWhiteThreshold  = (minExpect + maxExpect)/2;

  if (!mObjConnect) {
    mObjConnect.reset(new ObjConnect(this));
  }
  mObjConnect->ConnectBlack(region, mBlackThreshold);
  mObjRects.clear();
  mUin->BeginTestChar();
  const QVector<QRect>& rects = mObjConnect->GetObjList();
  foreach (const QRect& rect, rects) {
    if (rect.width() < Width()/4 && rect.height() > 4) {
      mObjRects.append(rect);
      mUin->TestChar(Region<uchar>(mSrc, rect.x(), rect.y(), rect.width(), rect.height()), mWhiteThreshold, mBlackThreshold);
    }
  }

  return true;
}

void ImgAnalizer::DumpUinPre(Region<uchar>* region)
{
  mSignalMark2->FillRegionMark3(region);
}

void ImgAnalizer::DumpHyst(Region<uchar>* region)
{
  mDst.SetSize(255, 255);
  mDst.ZeroData();

  int totalCount = mHyst.TotalCount() / 4;
  if (totalCount <= 0) {
    return;
  }
  for (int j = 0; j < 255; j++) {
    int v = mHyst.GetHyst(j) * 255 / totalCount;
    uchar* dst = mDst.Line(j);
    for (int i = 0; i < v; i++) {
      *dst = 127;
      dst++;
    }
  }

  region->SetSource(mDst);
}

void ImgAnalizer::DumpBlackWhite(Region<uchar>* region)
{
  for (int j = 0; j < Height(); j++) {
    const uchar* src = mSrc.Line(j);
    uchar* dst = mDst.Line(j);
    for (int i = 0; i < Width(); i++) {
      if (*src <= mBlackThreshold) {
        *dst = 0;
      } else if (*src >= mWhiteThreshold) {
        *dst = 255;
      } else {
        *dst = 127;
      }

      src++;
      dst++;
    }
  }

  region->SetSource(mDst);
}

void ImgAnalizer::DumpUinPrepareRu(Region<uchar>* region)
{
  for (int j = 0; j < Height(); j++) {
    const uchar* src = mSrc.Line(j);
    uchar* dst = mDst.Line(j);
    memcpy(dst, src, Width());
  }

  foreach (const QRect& rect, mObjRects) {
    int l = qMax(0, rect.left() - 1);
    int t = qMax(0, rect.top() - 1);
    int r = qMin(mDst.Width() - 1, rect.right() + 1);
    int b = qMin(mDst.Height() - 1, rect.bottom() + 1);
    QRect boundingRect(l, t, r - l + 1, b - t + 1);
    mDst.FillRectBorder(boundingRect, 255);
  }

  region->SetSource(mDst);
}

void ImgAnalizer::DumpUinPrepare(Region<uchar>* region)
{
  if (!mUin) {
    return;
  }

  mUin->DumpBase(region);
}

void ImgAnalizer::DumpUinPrepareObj(Region<uchar>* region)
{
  mUin->DumpTest(region);
}


ImgAnalizer::ImgAnalizer()
  : mWidth(0), mHeight(0)
{
}

