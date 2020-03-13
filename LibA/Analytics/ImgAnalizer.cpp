#include "ImgAnalizer.h"
#include "Uin/UinPre.h"


void ImgAnalizer::Init(const uchar* data, int width, int height, int stride)
{
  mWidth  = width;
  mHeight = height;
  mSrc.SetSource(const_cast<uchar*>(data), width, height, stride);
  mDst.SetSize(width, height);
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
      int h = (int)*srcnn - (int)*srcpn;
      int v = (int)*srcnn - (int)*srcnp;
      int d = qAbs(qMax(h, v));
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

void ImgAnalizer::Make2Color()
{
  int med = mHyst.GetMidValue(50, 950);

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

void ImgAnalizer::MakeUinPre()
{
  if (!mUinPre) {
    mUinPre.reset(new UinPre());
  }

  mUinPre->Clear();
  mUinPre->CalcStage1(&mSrc);
}

void ImgAnalizer::ResetUinPreRect()
{
  mUinPre->ResetRect();
}

bool ImgAnalizer::NextUinPreRect(QRect& rect)
{
  Rectangle r;
  if (mUinPre->NextRect(r)) {
    rect.setCoords(r.Left, r.Top, r.Right, r.Bottom);
    return true;
  }
  return false;
}

void ImgAnalizer::GetUinPre(Region<uchar>& debug)
{
  mUinPre->DebugValue(debug);
}


ImgAnalizer::ImgAnalizer()
  : mWidth(0), mHeight(0)
{
}

