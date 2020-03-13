#include "UinPre.h"


const int kMinUinHeight = 10;
const int kMinHeight  = 5;
const int kMinValue   = 2;
const int kNearLength = 20;

int UinPre::MinHeight()
{
  return kMinUinHeight;
}

void UinPre::Calc()
{
  mDetectResult.SetSize(mDetectRegion.Width(), mDetectRegion.Height());
  mDetectResult.ZeroData();

  for (int j = 0; j < mDetectRegion.Height(); j++) {
    const uchar* srcl = mDetectRegion.Line(j);
    const uchar* src = srcl;
    mResultData = mDetectResult.Line(j);
    src++;

    mDirection = eDescent;
    mDown = 0;
    mUp   = 0;
    mLeftTop  = 0;
    mRightTop = 0;
    mLastTop = -kNearLength;
    mLastValue = 0;
    for (int i = 1; i < mDetectRegion.Width(); i++) {
      if (*srcl >= *src) { // down
        uchar h = *srcl - *src;
        switch (mDirection) {
        case eDescent:
          if (mDown < h) {
            mDown = h;
            mLeftTop = i;
          } else if (h + kMinHeight < mDown / 2) {
            mDirection = eTop;
          }
          break;

        case eTop:
          if (h > mDown / 2 + kMinHeight) {
            mDirection = eDescent;
            mDown = h;
          }
          break;

        case eAscent:
          ApplyTop();
          mDirection = eDescent;
          mDown = h;
          break;
        }
      } else { // up
        uchar h = *src - *srcl;
        switch (mDirection) {
        case eDescent:
          mLeftTop = i;
          if (h > mDown / 4 + kMinHeight) {
            mDirection = eAscent;
            mRightTop = i;
            mUp = h;
          } else if (mDown > kMinHeight) {
            mDirection = eTop;
          }
          break;

        case eTop:
          if (h > mDown / 4 + kMinHeight) {
            mDirection = eAscent;
            mRightTop = i;
            mUp = h;
          }
          break;

        case eAscent:
          if (mUp < h) {
            mUp = h;
            mRightTop = i;
          } else if (h + kMinHeight < mUp / 2) {
            ApplyTop();
            mDirection = eDescent;
            mUp = 0;
          }
          break;
        }
      }

      srcl++;
      src++;
    }
  }
}

void UinPre::ApplyTop()
{
  int newValue = qMin(mDown, mUp) / (mRightTop - mLeftTop + 1) / (mRightTop - mLeftTop + 1);
  //if (newValue < kMinValue) {
  //  return;
  //}
  if (mRightTop < mLastTop + kNearLength && mLastValue >= kMinValue && newValue >= kMinValue) {
    for (int i = mLastTop + 1; i < mRightTop; i++) {
      mResultData[i] = 1;
    }
  }
  mLastValue = mResultData[mLeftTop] = mResultData[mRightTop] = newValue;
  mLastTop = mLeftTop;
}


UinPre::UinPre()
{
}

