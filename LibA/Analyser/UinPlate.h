#pragma once

#include <LibV/Include/Region.h>

#include "UinMetrics.h"


class UinPlate
{
  UinMetrics    mUinMetrics;
  Region<uchar> mSrc;
  Region<uchar> mDst;
  QList<QRect>  mResults;

public:
  void CalcRegion(const Region<uchar>& region);
  void DumpResults(Region<uchar>* debug);

private:
  bool CalcSubRegion(const Region<uchar>& srcRegion, Region<uchar>& dstRegion);

public:
  UinPlate();
};
