#pragma once

#include <QVector>

#include <Lib/Include/Common.h>
#include <LibV/Include/Hyst.h>
#include <LibV/Include/Rect.h>
#include <LibV/Include/Region.h>


DefineClassS(SignalMark2);

class SignalMark2
{
  const Region<uchar>* mSrc;
  int                  mLengthMax;
  int                  mContextLength;

  enum EState {
    eL0,
    eL1,
    eL2,
    eL12,
    eR2,
    eR1
  };

  EState           mState;
  int              mL1;
  int              mL2;
  int              mL12;
  int              mR2;
  int              mR1;
  int              mL1Value;
  int              mL2Value;
  int              mL12Value;
  int              mR2Value;
  int              mR1Value;
  int              mTopValue;
  int              mPowerThreshold;

  struct Signal {
    int  Left;
    int  Right;
    int  Width;
    int  Height;

    int  Power;
    int  Level;
    int  Continue;
    int  Break;
    int  Connect;
    bool Ok;
  };
  typedef QVector<Signal> LineSig;

  struct SignalConnect {
    int  Left;
    int  Right;
  };
  typedef QVector<SignalConnect> LineCon;

  struct SignalArea {
    int  Left;
    int  Right;
    int  Top;
    int  Bottom;
  };

  LineSig*            mCurrentSig;
  LineSig*            mCurrentSigBad;
  LineSig             mLineSig;
  LineCon*            mCurrentCon;
  QVector<SignalArea> mCurrentArea;
  QVector<LineSig>    mRegionSig;
  QVector<LineSig>    mRegionSigBad;
  QVector<LineCon>    mRegionCon;

  int                 mSignalBreakThreshold;
  Region<int>         mSignalContext;
  QVector<SignalArea> mAllArea;

public:
  void Calc(const Region<uchar>* region);
  void CalcLine(const uchar* src, int size);

  void FillLineMark(uchar* mark, int size);
  void FillRegionMark(Region<uchar>* debug);
  void FillRegionMark2(Region<uchar>* debug);
  void FillRegionMark3(Region<uchar>* debug);
  void FillRegionLevel(Region<uchar>* debug);

private:
  void MarkLine(const uchar* src, int size);
  void MergeLine(LineSig* line1, LineSig* line2);
  void MergeLine2(LineSig* line1, LineSig* line2);
  void EqualLine(LineSig* line1, LineSig* line2);
  void EqualLineContinue(LineSig* line1, LineSig* line2);
  void FilterSignal();
  void FilterSignalWidth(int minWidth, int maxWidth);
  void FilterSignalContinue(int minValue, int maxValue);
  void ConnectLine();
  void ConnectAreaLine(int j);
  void ContextFilter(int width, int height, int threshold);
  bool EndSignal();
  bool FinalSignalFilter(const Signal& signal);
  int FinalSignalMark(const Signal& signal);
  int FinalSignalGood(const Signal& signal);

public:
  SignalMark2(int _LengthMax = 80);
};
