#pragma once

#include <Lib/Include/Common.h>

#include <LibV/Va/Region.h>


class UinPre
{
  PROPERTY_GET_SET(int, RegionI)
  PROPERTY_GET_SET(int, RegionJ)
  PROPERTY_GET_SET(Region<uchar>, DetectRegion)
  PROPERTY_GET(Region<uchar>,     DetectResult)
  ;

  enum ELpTopDirection {
    eDescent,
    eTop,
    eAscent
  };
  ELpTopDirection mDirection;
  int             mDown;
  int             mUp;
  int             mLeftTop;
  int             mRightTop;
  int             mLastTop;
  int             mLastValue;
  uchar*          mResultData;

public:
  static int MinHeight();
  void Calc();

private:
  void inline ApplyTop();

public:
  UinPre();

  friend class BlockObj;
};
