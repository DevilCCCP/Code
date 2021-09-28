#pragma once

#include <QVector>
#include <list>

#include <Lib/Include/Common.h>
#include <LibV/Include/Hyst.h>
#include <LibV/Include/Rect.h>
#include <LibV/Include/Region.h>


DefineClassS(SignalMark);

class SignalMark
{
  const Region<uchar>* mSrc;
  int                  mLengthMax;
  int                  mContextLength;

  enum EDirection {
    eNeytral,
    eUp,
    eTop,
    eDown,
    eBottom
  };
  uchar*           mLineSignal;
  EDirection       mDirection;
  int              mLeftTop;
  int              mRightTop;
  int              mLeftBottomValue;
  int              mLeftTopValue;
  int              mRightTopValue;
  int              mRightBottomValue;
  int              mSignalPower;

  struct Signal {
    int Left;
    int Right;
    int Power;
  };
  typedef std::list<Signal> LineSig;

  LineSig*         mCurrentSig;
  LineSig          mLineSig;
  QVector<LineSig> mRegionSig;

public:
  void Calc(const Region<uchar>* region, int contextFilter);
  void CalcLine(const uchar* src, int size, int contextFilter);
  void FillLineMark(uchar* mark, int size);
  void FillRegionMark(Region<uchar>* debug);

private:
  void MarkLine(const uchar* src, int size);
  void EndSignal();

  void ContextFilterLine();

public:
  SignalMark(int _LengthMax);
};
