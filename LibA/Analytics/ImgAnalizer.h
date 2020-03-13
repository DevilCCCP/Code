#pragma once

#include <QRect>

#include <Lib/Include/Common.h>
#include <LibV/Va/Region.h>

#include "Hyst.h"


DefineClassS(UinPre);
DefineClassS(Uin);

class ImgAnalizer
{
  UinPreS       mUinPre;

  int           mWidth;
  int           mHeight;
  Region<uchar> mSrc;
  Region<uchar> mDst;
  Hyst          mHyst;

public:
  const Region<uchar>& Result() const { return mDst; }
protected:
  int Width()  { return mWidth; }
  int Height() { return mHeight; }

public:
  void Init(const uchar* data, int width, int height, int stride);
  void MakeGrad();
  void Make2Color();

  void MakeUinPre();
  void ResetUinPreRect();
  bool NextUinPreRect(QRect& rect);
  void GetUinPre(Region<uchar>& debug);

public:
  ImgAnalizer();
};
