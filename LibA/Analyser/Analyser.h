#pragma once

#include <QRect>

#include <Lib/Include/Common.h>
#include <LibV/Include/Region.h>
#include <LibV/Include/Hyst.h>


DefineClassS(Uin);
DefineClassS(UinAreaStat);
DefineClassS(SignalMark3);
DefineClassS(UinPlate);
DefineClassS(ObjConnect);

class Analyser
{
  const bool     mDump;

  int            mWidth;
  int            mHeight;
  Region<uchar>  mSrc;
  Region<uchar>  mDst;
  Hyst           mHyst;
  int            mBlackThreshold;
  int            mWhiteThreshold;
  int            mBackWhiteThreshold;

  ObjConnectS    mObjConnect;
  QVector<QRect> mConnectRects;
  QVector<QRect> mSelectedRects;

  QRect          mCurrentPlate;
  QList<QRect>   mObjRects;
  QList<QRect>   mObjRectsFixed;
  UinS           mUin;
  UinAreaStatS   mUinAreaStat;
  SignalMark3S   mSignalMark3;
  UinPlateS      mUinPlate;

public:
  const Region<uchar>& Source() const { return mSrc; }
  const Region<uchar>& Result() const { return mDst; }
  int Width()  { return mWidth; }
  int Height() { return mHeight; }
  const UinS& GetUin();
  void TakeUin(UinS& uin);

public:
  void Init(const uchar* data, int width, int height, int stride);
  void Init(const Region<uchar>& region);
  void Init(const Region<uchar>& region, int x, int y, int width, int height);
  void MakeGrad();
  void Make2Color(int minPerc = 50, int maxPerc = 950);
  void MakeMedian(int len);
  void MakeLower(int len);
  void MakeHigher(int len);
  void MakeWhiteBallance(int white = 950, int black = 50);
  void CalcHyst(int x, int y, int width, int height);

  bool FindUinRu(const Region<uchar>& region, int width);
  QRect AreaToPlate(const QRect& rect);
  bool ExtractPlate(const QRect& rect, Region<uchar>& region);
  bool UinInit();
  bool CalcUinRu(const Region<uchar>& region, const QRect& center, const QRect& plate);
  bool CalcPlateRu(const Region<uchar>& region);

private:
  bool FindUinRuDigits(const QRect& digitRect, const QVector<QRect>& otherRects);
  bool FindUinNeighbourRight(const QRect& digitRect, const QVector<QRect>& otherRects, QRect& neighbourRect);
  bool FindUinNeighbourLeft(const QRect& digitRect, const QVector<QRect>& otherRects, QRect& neighbourRect);

public:
  void DumpUinStatRaw(Region<uchar>* region, int minDiff, int);
  void DumpUinStatRaw2(Region<uchar>* region, int minDiff, int);
  void DumpUinStatRaw23(Region<uchar>* region, int minDiff, int);
  void DumpUinStatSignal(Region<uchar>* region, int, int);
  void DumpUinStatSignalLevel(Region<uchar>* region, int level, int);
  void DumpUinStatThickness2(Region<uchar>* region, int level, int threshold);
  void DumpUinStatThickness23(Region<uchar>* region, int level, int threshold);
  void DumpUinStatEdge(Region<uchar>* region, int, int);
  void DumpUinStatEdgeFiltered(Region<uchar>* region, int, int);
  void DumpUinStatBlack(Region<uchar>* region, int, int);
  void DumpUinStatWhite(Region<uchar>* region, int, int);
  void DumpUinStatCountBlack(Region<uchar>* region, int, int);
  void DumpUinStatCountWhite(Region<uchar>* region, int, int);
  void DumpUinStatMiddle(Region<uchar>* region, int, int);
  void DumpUinStatDiff(Region<uchar>* region, int, int);
  void DumpUinStatWhiteLevel(Region<uchar>* region, int level, int diffLevel);
  void DumpUinStatBothLevel(Region<uchar>* region, int whiteLevel, int blackLevel);
  void DumpUinStatWhiteLevelCut(Region<uchar>* region, int level, int);
  void DumpUinStatPlate(Region<uchar>* region, int index, int);
  void DumpUinStatPlateNormal(Region<uchar>* region, int indexPlate, int indexChar);
  void DumpUinCutLevel(Region<uchar>* region, int whiteLevel, int blackLevel);
  void DumpUinCutLevel2(Region<uchar>* region, int blackWhiteLevel, int);
  void DumpUinColorLevel(Region<uchar>* region, int level, int);
  void DumpSignalValue(Region<uchar>* region);
  void DumpSignalHeight(Region<uchar>* region, int, int);
  void DumpSignalPack(Region<uchar>* region, int, int);
  void DumpSignalArea(Region<uchar>* region, int, int);
  void DumpHyst(Region<uchar>* region);
  void DumpBlackWhite(Region<uchar>* region);
  void DumpUinSolids(Region<uchar>* region, int, int);
  void DumpUinSymbols(Region<uchar>* region, int, int);
  void DumpUinSymbolsFixed(Region<uchar>* region, int, int);
  void DumpPlate(Region<uchar>* region, int, int);
  void DumpUinPrepare(Region<uchar>* region, int, int);
  void DumpUinTest(Region<uchar>* region, int index, int);
  void DumpUinDigits(Region<uchar>* region, int index, int);

public:
  Analyser(bool _Dump = false);
};
