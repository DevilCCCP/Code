#include <LibV/Include/Hyst.h>

#include "UinPlate.h"


const int kMinWhiteBlackDiff = 20;

void UinPlate::CalcRegion(const Region<uchar>& region)
{
  mSrc.SetSource(region);
  mResults.clear();

  mUinMetrics.SetBaseWidth(150);
  if (mSrc.Width() < mUinMetrics.Width() / 2 || mSrc.Height() < mUinMetrics.Height() / 2) {
//    mResults.append(QRect(0, 0, mSrc.Width(), mSrc.Height()));
    mDst.SetSource(mSrc);
    return;
  }

  mDst.SetSize(mSrc.Width(), mSrc.Height());
  mDst.FillData(255);
  int  blockWidth = mUinMetrics.DigitWidth();
  int blockHeight = mUinMetrics.DigitHeight()/3;
  for (int j = 0; ; j++) {
    int jFrom = j*blockHeight;
    int jTo   = jFrom + blockHeight;
    if (jTo > mSrc.Height()) {
      break;
    }
    for (int i = 0; ; i++) {
      int iFrom = i*blockWidth;
      int iTo   = iFrom + blockWidth;
      if (iTo > mSrc.Width()) {
        break;
      }
      Region<uchar> subSrc(mSrc, iFrom, jFrom, blockWidth, blockHeight);
      Region<uchar> subDst(mDst, iFrom, jFrom, blockWidth, blockHeight);
      /*bool good = */CalcSubRegion(subSrc, subDst);
//      if (good) {
//        mResults.append(QRect(iFrom, jFrom, blockWidth, blockHeight));
//      }
    }
  }
}

void UinPlate::DumpResults(Region<uchar>* debug)
{
  debug->SetSource(mDst);
//  if (debug->Width() != mSrc.Width() || debug->Height() != mSrc.Height()) {
//    debug->SetSize(mSrc.Width(), mSrc.Height());
//  }
//  debug->FillData(255);

//  foreach (const QRect& rect, mResults) {
//    debug->FillRectBorder(rect, 0);
//  }
}

bool UinPlate::CalcSubRegion(const Region<uchar>& srcRegion, Region<uchar>& dstRegion)
{
  Hyst hyst;
  for (int j = 0; j < srcRegion.Height(); j++) {
    const uchar* src = srcRegion.Line(j);
    for (int i = 0; i < srcRegion.Width(); i++) {
      hyst.Inc(*src);
    }
  }

  int minValue  = hyst.GetValue(300);
  int maxValue  = hyst.GetValue(400);
  int minExpect = hyst.GetLocalMax(0, minValue);
  int maxExpect = hyst.GetLocalMax(maxValue, 255);

//  if (maxExpect - minExpect < kMinWhiteBlackDiff) {
//    return false;
//  }

  int blackThreshold      = (2*minExpect + maxExpect)/3;
//  int whiteThreshold      = (minExpect + 2*maxExpect)/3;
//  int backWhiteThreshold  = (minExpect + maxExpect)/2;

  int thick = mUinMetrics.DigitThick();
  int count = 0;
  for (int j = 0; j < srcRegion.Height(); j++) {
    const uchar* src = srcRegion.Line(j);
    int thickc = 0;
    for (int i = 0; i < srcRegion.Width(); i++) {
      if (*src <= blackThreshold) {
        thickc++;
      } else {
        if (thickc > 1 && thickc < thick) {
          count++;
        }
        thickc = 0;
      }

      src++;
    }
    if (thickc > 1 && thickc < thick) {
      count++;
    }
  }
  if (count >= srcRegion.Height()) {
    dstRegion.FillData(127);
  }

  for (int j = 0; j < srcRegion.Height(); j++) {
    const uchar* src = srcRegion.Line(j);
    uchar* dst = dstRegion.Line(j);
    for (int i = 0; i < srcRegion.Width(); i++) {
      if (*src <= blackThreshold) {
        *dst = 0;
      }

      src++;
      dst++;
    }
  }
  return true;
}


UinPlate::UinPlate()
{
}

