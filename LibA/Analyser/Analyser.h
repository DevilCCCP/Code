#pragma once

#include <QRect>

#include <Lib/Include/Common.h>

#include "FilterInfo.h"
#include "ByteRegion.h"


DefineClassS(Analyser);
DefineClassS(ImageStatFtr);
DefineClassS(CellStatFtr);
DefineClassS(SignalMarkFtr);
DefineClassS(SymbolMarkFtr);
DefineClassS(DigitFtr);
DefineClassS(PlateFindFtr);
DefineClassS(PlateCalcFtr);

class Analyser
{
  const bool      mDebug;

  int             mWidth;
  int             mHeight;
  ByteRegion      mSource;
  ByteRegion      mResult;
  int             mDataVersion;

  const uchar*    mLine;
  int             mLineSize;

  ImageStatFtrS   mImageStatFtr;
  CellStatFtrS    mCellStatFtr;
  SignalMarkFtrS  mSignalMarkFtr;
  SymbolMarkFtrS  mSymbolMarkFtr;
  DigitFtrS       mDigitFtr;
  PlateFindFtrS   mPlateFindFtr;
  PlateCalcFtrS   mPlateCalcFtr;

#define UseRegionFilter \
  UseRegionFilterOne(CellStat,BlockWgb)\
  UseRegionFilterOne(CellStat,BlockWb11)\
  UseRegionFilterOne(CellStat,BlockWb21)\
  UseRegionFilterOne(SignalMark,SignalRaw)\
  UseRegionFilterOne(SymbolMark,SymbolPre)\
  UseRegionFilterOne(SymbolMark,SymbolRaw)

  enum RegionFilterId {
#define UseRegionFilterOne(X1,X2) eRid##X1##X2,
    UseRegionFilter
#undef UseRegionFilterOne
    eRidFilterIllegal
  };

#define UseLineFilter \
  UseLineFilterOne(SignalMark,Extrem)\
  UseLineFilterOne(SignalMark,Move)\
  UseLineFilterOne(SignalMark,Signal)

  enum LineFilterId {
#define UseLineFilterOne(X1,X2) eLid##X1##X2,
    UseLineFilter
#undef UseLineFilterOne
    eLidFilterIllegal
  };

public:
  int Width()  { return mWidth; }
  int Height() { return mHeight; }
  const ByteRegion& Source() { return mSource; }
  const ByteRegion& Result() { return mResult; }
  int DataVersion() { return mDataVersion; }

  const uchar*    Line()     { return mLine; }
  int             LineSize() { return mLineSize; }

  const ImageStatFtrS&   GetImageStatFtr();
  const CellStatFtrS&    GetCellStatFtr();
  const SignalMarkFtrS&  GetSignalMarkFtr();
  const SymbolMarkFtrS&  GetSymbolMarkFtr();
  const DigitFtrS&       GetDigitFtr();
  const PlateFindFtrS&   GetPlateFindFtr();
  const PlateCalcFtrS&   GetPlateCalcFtr();

public:
  void RegionInit(const uchar* data, int width, int height, int stride);
  void RegionInit(const ByteRegion& region);
  void RegionInit(const ByteRegion& region, int x, int y, int width, int height);
  int RegionFilterCount();
  QString RegionFilterName(int index);
  bool RegionFilterInfo(int index, FilterInfo* filterInfo);
  bool RegionFilterTest(int index, int p1, int p2);

  void LineInit(const uchar* src, int size);
  int LineFilterCount();
  QString LineFilterName(int index);
  bool LineFilterTest(int index, QVector<uchar>& mark);

public:
  ByteRegion* PrepareResult();
  ByteRegion* PrepareResultWhite();
  ByteRegion* PrepareResultBlack();

public:
  Analyser(bool _Debug = false);
};
