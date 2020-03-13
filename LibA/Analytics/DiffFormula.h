#pragma once

#include <QString>

#include <Lib/Include/Common.h>
#include <LibV/Include/Hyst.h>
#include <Lib/Log/Log.h>

#include "BlockScene.h"


DefineClassS(DiffFormula);

class DiffFormula
{
  HystFast* mDiffHyst;
  int       mBlockSize;

  Hyst      mLowLevelHyst;
  Hyst      mHighLevelHyst;
  int       mBigDiff;
  int       mNormalDiff;

  int       mNormalDiffl;
  int       mNormalDiffh;
  int       mNormalDiffh2;
  int       mBigDiffl;
  int       mBigDiffh;
  int       mBigDiffh2;

  int       mLastResult;

public:
  bool IsValid() { return mBigDiff > mNormalDiff; }

  int Calc(const uchar& bf, const uchar& bb) { return IncRelivant(CalcFormula(bf, bb)); }
  void Merge(const uchar& bf, uchar& bb) { return NoMergeFormula(bf, bb); }

  int CalcBlock(int diffCount) { return (diffCount > mBlockSize)? ((diffCount > 2*mBlockSize)? 2: 1): 0; }

  int IncRelivant(int result)
  {
    int lastResult = mLastResult;
    mLastResult = result;
    if (lastResult) {
      return result + 2;
    }
    return result;
  }

  int CalcFormula(const uchar& bf, const uchar& bb)
  {
    if (bf >= bb) {
      if ((bf>>1) > bb + 5) {
        return 2;
        if ((bf>>2) > bb + 5) {
          return 3;
        }
      }
    } else {
      if ((bb>>1) > bf + 5) {
        return 1;
      }
    }
    return 0;
  }

  int CalcFormula2(const uchar& bf, const uchar& bb)
  {
    if (bb >= mNormalDiff) {
      if (bf >= bb) {
        if (bf - bb >= mBigDiffh) {
          if (bf - bb >= mBigDiffh2) {
            return 3;
          }
          return 2;
        }
      } else {
        if (bb - bf >= mBigDiffl) {
          return 2;
        }
      }
    } else {
      if (bf >= bb) {
        if (bf - bb >= mNormalDiffh) {
          if (bf - bb >= mNormalDiffh2) {
            return 3;
          }
          return 2;
        }
      } else {
        if (bb - bf >= mNormalDiffl) {
          return 1;
        }
      }
    }
    return 0;
  }

private:
  void NoMergeFormula(const uchar&, uchar&)
  {
  }

  void MergeFormula(const uchar& bf, uchar& bb)
  {
    if (bf >= bb) {
      if ((bf>>1) > bb + 5) {
        return;
        if ((bf>>2) > bb + 5) {
          return;
        }
      }
    } else {
      if ((bb>>1) > bf + 5) {
        return;
      }
    }
    bb = bf;
  }

  void MergeFormula2(const uchar& bf, uchar& bb)
  {
    if (bb >= mNormalDiff) {
      if (bf >= bb) {
        if (bf - bb >= mBigDiffh) {
          if (bf - bb >= mBigDiffh2) {
            return;
          }
          return;
        }
      } else {
        if (bb - bf >= mBigDiffl) {
          return;
        }
      }
    } else {
      if (bf >= bb) {
        if (bf - bb >= mNormalDiffh) {
          if (bf - bb >= mNormalDiffh2) {
            return;
          }
          return;
        }
      } else {
        if (bb - bf >= mNormalDiffl) {
          return;
        }
      }
    }
    bb = bf;
  }

public:
  void Init()
  {
    int lowLevel = mDiffHyst->GetValue(500);
    int highLevel = mDiffHyst->GetValue(990);
    mLowLevelHyst.Inc(lowLevel);
    mHighLevelHyst.Inc(highLevel);
    if (mLowLevelHyst.TotalCount() > 20) {
      lowLevel = mLowLevelHyst.GetMidValue(400, 600);
      highLevel = mHighLevelHyst.GetMidValue(400, 600);
    }

    mBigDiff = highLevel;
    mNormalDiff = (highLevel + lowLevel)/2;

    mNormalDiffl = qMax(mNormalDiff / 2, 4);
    mNormalDiffh = qMax(mNormalDiff / 2, 4);
    mNormalDiffh2 = qMax(mNormalDiff, 6);
    mBigDiffl = qMax(mNormalDiff, 6);
    mBigDiffh = qMax(mNormalDiff, 6);
    mBigDiffh2 = qMax(mBigDiff, 10);

    mLastResult = 0;
    //Log.Debug(QString("%1 %2 (n: (%3, %4) h: (%5, %6))")
    //          .arg(mNormalDiff).arg(mBigDiff).arg(mNormalDiffl).arg(mNormalDiffh2).arg(mBigDiffl).arg(mBigDiffh2));
  }

public:
  DiffFormula(HystFast* _DiffHyst, int _BlockSize)
    : mDiffHyst(_DiffHyst), mBlockSize(_BlockSize)
    , mLastResult(0)
  { }
};
