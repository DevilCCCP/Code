#include "Analyser.h"
#include "ImageStatFtr.h"
#include "CellStatFtr.h"
#include "SignalMarkFtr.h"
#include "SymbolMarkFtr.h"
#include "DigitFtr.h"
#include "PlateFindFtr.h"
#include "PlateCalcFtr.h"


const ImageStatFtrS& Analyser::GetImageStatFtr()
{
  if (!mImageStatFtr) {
    mImageStatFtr.reset(new ImageStatFtr(this));
  }
  return mImageStatFtr;
}

const CellStatFtrS& Analyser::GetCellStatFtr()
{
  if (!mCellStatFtr) {
    mCellStatFtr.reset(new CellStatFtr(this));
  }
  return mCellStatFtr;
}

const SignalMarkFtrS& Analyser::GetSignalMarkFtr()
{
  if (!mSignalMarkFtr) {
    mSignalMarkFtr.reset(new SignalMarkFtr(this));
  }
  return mSignalMarkFtr;
}

const SymbolMarkFtrS& Analyser::GetSymbolMarkFtr()
{
  if (!mSymbolMarkFtr) {
    mSymbolMarkFtr.reset(new SymbolMarkFtr(this));
  }
  return mSymbolMarkFtr;
}

const DigitFtrS& Analyser::GetDigitFtr()
{
  if (!mDigitFtr) {
    mDigitFtr.reset(new DigitFtr(this));
  }
  return mDigitFtr;
}

const PlateFindFtrS& Analyser::GetPlateFindFtr()
{
  if (!mPlateFindFtr) {
    mPlateFindFtr.reset(new PlateFindFtr(this));
  }
  return mPlateFindFtr;
}

const PlateCalcFtrS& Analyser::GetPlateCalcFtr()
{
  if (!mPlateCalcFtr) {
    mPlateCalcFtr.reset(new PlateCalcFtr(this));
  }
  return mPlateCalcFtr;
}

void Analyser::RegionInit(const uchar* data, int width, int height, int stride)
{
  mWidth  = width;
  mHeight = height;
  mSource.SetSource(const_cast<uchar*>(data), mWidth, mHeight, stride);
  mResult.SetSize(mWidth, mHeight);
  mDataVersion++;
}

void Analyser::RegionInit(const ByteRegion& region)
{
  mWidth  = region.Width();
  mHeight = region.Height();
  mSource.SetSource(region);
  mResult.SetSize(mWidth, mHeight);
  mDataVersion++;
}

void Analyser::RegionInit(const ByteRegion& region, int x, int y, int width, int height)
{
  mWidth  = width;
  mHeight = height;
  mSource.SetSource(region, x, y, mWidth, mHeight);
  mResult.SetSize(mWidth, mHeight);
  mDataVersion++;
}

int Analyser::RegionFilterCount()
{
  return eRidFilterIllegal;
}

QString Analyser::RegionFilterName(int index)
{
  FilterInfo filterInfo;
  switch ((RegionFilterId)index) {
  case eRidFilterIllegal: break;
#define UseRegionFilterOne(X1,X2) case eRid##X1##X2: Get##X1##Ftr()->RegionInfo##X2(&filterInfo); return filterInfo.Name;
    UseRegionFilter
#undef UseRegionFilterOne
  }
  return "<error>";
}

bool Analyser::RegionFilterInfo(int index, FilterInfo* filterInfo)
{
  switch ((RegionFilterId)index) {
  case eRidFilterIllegal: break;
#define UseRegionFilterOne(X1,X2) case eRid##X1##X2: Get##X1##Ftr()->RegionInfo##X2(filterInfo); return true;
    UseRegionFilter
#undef UseRegionFilterOne
  }
  return false;
}

bool Analyser::RegionFilterTest(int index, int p1, int p2)
{
  switch ((RegionFilterId)index) {
  case eRidFilterIllegal: break;
#define UseRegionFilterOne(X1,X2) case eRid##X1##X2: return Get##X1##Ftr()->RegionTest##X2(p1,p2);
    UseRegionFilter
#undef UseRegionFilterOne
  }
  return false;
}

void Analyser::LineInit(const uchar* src, int size)
{
  mLine = src;
  mLineSize = size;
}

int Analyser::LineFilterCount()
{
  return eLidFilterIllegal;
}

QString Analyser::LineFilterName(int index)
{
  switch ((LineFilterId)index) {
  case eLidFilterIllegal: break;
#define UseLineFilterOne(X1,X2) case eLid##X1##X2: return Get##X1##Ftr()->LineName##X2();
    UseLineFilter
#undef UseLineFilterOne
  }
  return "<error>";
}

bool Analyser::LineFilterTest(int index, QVector<uchar>& mark)
{
  switch ((LineFilterId)index) {
  case eLidFilterIllegal: break;
#define UseLineFilterOne(X1,X2) case eLid##X1##X2: return Get##X1##Ftr()->LineTest##X2(mark);
    UseLineFilter
#undef UseLineFilterOne
  }
  return false;
}

ByteRegion* Analyser::PrepareResult()
{
  if (mResult.Width() != Width() || mResult.Height() != Height()) {
    mResult.SetSize(Width(), Height());
  }

  return &mResult;
}

ByteRegion* Analyser::PrepareResultWhite()
{
  PrepareResult();
  mResult.FillData(255);

  return &mResult;
}

ByteRegion* Analyser::PrepareResultBlack()
{
  PrepareResult();
  mResult.FillData(0);

  return &mResult;
}


Analyser::Analyser(bool _Debug)
  : mDebug(_Debug)
  , mWidth(0), mHeight(0), mDataVersion(0)
  , mLine(nullptr), mLineSize(0)
{
}

