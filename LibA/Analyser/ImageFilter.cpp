#include "ImageFilter.h"


bool ImageFilter::StartCalcPrimary()
{
  if (GetAnalyser()->DataVersion() == mPrimaryVersion) {
    return false;
  }
  mPrimaryVersion = GetAnalyser()->DataVersion();
  return true;
}

bool ImageFilter::StartCalcSecondary(int index)
{
  if (mSecondaryVersionList.size() <= index) {
    mSecondaryVersionList.resize(index + 1);
  }

  if (GetAnalyser()->DataVersion() == mSecondaryVersionList[index]) {
    return false;
  }
  mSecondaryVersionList[index] = GetAnalyser()->DataVersion();
  return true;
}

void ImageFilter::ClearPrimary()
{
  mPrimaryVersion = 0;
}

void ImageFilter::ClearSecondary(int index)
{
  if (mSecondaryVersionList.size() <= index) {
    mSecondaryVersionList.resize(index + 1);
  } else {
    mSecondaryVersionList[index] = 0;
  }
}


ImageFilter::ImageFilter(Analyser* _Analyser)
  : mAnalyser(_Analyser)
  , mPrimaryVersion(0)
{
}

ImageFilter::~ImageFilter()
{
}
