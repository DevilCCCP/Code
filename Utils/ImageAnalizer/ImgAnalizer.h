#pragma once

#include <QRect>

#include <Lib/Include/Common.h>
#include <LibV/Include/Region.h>
#include <LibV/Include/Hyst.h>


DefineClassS(Uin);
DefineClassS(SignalMark2);
DefineClassS(ObjConnect);

class ImgAnalizer
{
  int            mWidth;
  int            mHeight;
  Region<uchar>  mSrc;
  Region<uchar>  mDst;
  Hyst           mHyst;
  int            mBlackThreshold;
  int            mWhiteThreshold;
  int            mBackWhiteThreshold;

  ObjConnectS    mObjConnect;
  QVector<QRect> mObjRects;
  UinS           mUin;
  SignalMark2S   mSignalMark2;

public:
  const Region<uchar>& Source() const { return mSrc; }
  const Region<uchar>& Result() const { return mDst; }
  int Width()  { return mWidth; }
  int Height() { return mHeight; }

public:
  void Init(const uchar* data, int width, int height, int stride);
  void Init(const Region<uchar>& region);
  void Init(const Region<uchar>& region, int x, int y, int width, int height);
  void MakeGrad();
  void Make2Color(int minPerc = 50, int maxPerc = 950);
  void MakeMedian(int len);
  void MakeLower(int len);
  void MakeHigher(int len);
  void CalcHyst(int x, int y, int width, int height);

  bool FindUinRu(const Region<uchar>& region);
  bool UinInit();
  bool UinPreInit();
  bool UinPrepareRu(const Region<uchar>& region);

  void DumpUinPre(Region<uchar>* region);
  void DumpHyst(Region<uchar>* region);
  void DumpBlackWhite(Region<uchar>* region);
  void DumpUinPrepareRu(Region<uchar>* region);
  void DumpUinPrepare(Region<uchar>* region);
  void DumpUinPrepareObj(Region<uchar>* region);

public:
  ImgAnalizer();
};
