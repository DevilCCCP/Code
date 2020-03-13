#pragma once

#include <QVector>

#include <Lib/Include/Common.h>
#include <LibV/Include/Hyst.h>
#include <LibV/Include/Rect.h>
#include <LibV/Include/Region.h>


DefineClassS(SignalMark3);

class SignalMark3
{
  const Region<uchar>* mSrc;

  PROPERTY_GET_SET(int, SignalValueMin)
  PROPERTY_GET_SET(int, SignalTopMin)
  PROPERTY_GET_SET(int, SignalWidthMax)
  PROPERTY_GET_SET(int, SignalHeightMin)
  PROPERTY_GET_SET(int, SignalHeightMax)
  PROPERTY_GET_SET(int, PackWidth)
  PROPERTY_GET_SET(int, PackMinSpace)
  PROPERTY_GET_SET(int, PackMinCount)
  PROPERTY_GET_SET(int, AreaMinHeight)

  enum EExmState {
    eNoExm,
    eMini,
    eMaxi,
  };

  struct Extrem {
    int  Location;
    int  Value;
    bool IsMaxi;
  };
  typedef QVector<Extrem> LineExm;

  struct Move {
    int  Left;
    int  LeftValue;
    int  Right;
    int  RightValue;
    int  Rise;
  };
  typedef QVector<Move> LineMov;

  struct Backgroung {
    int  Left;
    int  Right;
    int  Value;
    int  Cut;
  };

  enum ESigState {
    eNoSig,
    eLb,
    eLt,
    eRt,
    eRb,
    eRb2,
  };

  struct Signal {
    int  Left;
    int  Top;
    int  Right;
    int  LeftValue;
    int  TopValue;
    int  RightValue;

    int  MinValue;
    int  Height;
  };
  typedef QVector<Signal> LineSig;

  struct SignalPack {
    int  Left;
    int  Right;
  };
  typedef QVector<SignalPack> LinePack;

  const uchar*        mSrcLine;
  int                 mSrcLineSize;
  EExmState           mExmState;
  LineExm             mLineExm;
  LineMov             mLineMov;
  int                 mMinValue;
  int                 mMaxValue;
  ESigState           mSigState;
  Signal*             mLostSignal;

  QVector<LineSig>    mRegionSig;
  LineSig*            mCurrentLineSig;
  QVector<LineSig>    mRegionSig2;

  QVector<LinePack>   mRegionPack;
  LinePack*           mCurrentLinePack;

  QVector<QRect>      mRegionArea;

public:
  void RegionCalc1(const Region<uchar>* region);
  void RegionCalc2();
  const QVector<QRect>& ResultAreas();

  void CalcLine(const uchar* src, int size);

  void DumpLineExtrem(QVector<uchar>& mark);
  void DumpLinePhases(QVector<uchar>& mark);
  void DumpLineMove(QVector<uchar>& mark);
  void DumpLineSignal(QVector<uchar>& mark);
  void DumpLinePack(QVector<uchar>& mark);
  void DumpRegionValue(Region<uchar>* debug);
  void DumpRegionHeight(Region<uchar>* debug);
  void DumpRegionPack(Region<uchar>* debug);
  void DumpRegionArea(Region<uchar>* debug);

private:
  void CalcInit();
  void MarkLine();
  void MarkLineExtrem();
  void MarkLineBackground();
  void MarkLineMove();
  void MarkLineSignal();
  void MarkLineSignalEnd();
  void MarkLineSignalOptimize();
  void MarkLinePack();
  void MarkLinePackOptimize();
  void FilterLine(int j);
  void MergeLine(int j);
  void ApplyLine(int j);

  void CollectArea();
  void FilterArea();

public:
  SignalMark3();
};
