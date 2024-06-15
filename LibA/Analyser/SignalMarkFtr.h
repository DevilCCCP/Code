#pragma once

#include <QVector>

#include "ImageFilter.h"
#include "Signal.h"
#include "Region.h"


class SignalMarkFtr: public ImageFilter
{
  PROPERTY_GET_SET(int, SignalValueMin)
  PROPERTY_GET_SET(int, SignalWidthMax)

  const uchar*          mSrcLine;
  int                   mSrcLineSize;
  SignalCtor::LineExm   mLineExm;
  SignalCtor::LineMov   mLineMov;
  int                   mMinValue;
  int                   mMaxValue;
  Signal*               mLostSignal;

  SignalLine*           mCurrentSignalLine;
  SignalArea            mSignalArea;

public:
  QString LineNameExtrem();
  QString LineNameMove();
  QString LineNameSignal();

  bool LineTestExtrem(QVector<uchar>& mark);
  bool LineTestMove(QVector<uchar>& mark);
  bool LineTestSignal(QVector<uchar>& mark);

  void RegionInfoSignalRaw(FilterInfo* filterInfo);
  void RegionInfoSignalLevel(FilterInfo* filterInfo);

  bool RegionTestSignalRaw(int minV, int minD);
  bool RegionTestSignalLevel(int level, int levelD);

  void LineCalc();
  SignalArea* CalcSignal();

  const SignalArea& GetSignalArea() const;

private:
  void MarkLine();
  void MarkLineExtrem();
  void MarkLineMove();
  void MarkLineSignal();
  void MarkLineSignalEnd();
  void MarkLineSignalOptimizeAndFilter();

public:
  SignalMarkFtr(Analyser* _Analyser);
};

