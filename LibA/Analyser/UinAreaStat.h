#pragma once

#include <QVector>
#include <QMap>
#include <QList>
#include <QImage>

#include <Lib/Include/Common.h>

#include "AnalyserOld.h"
#include "UinMetrics.h"
#include "Hyst.h"
#include "Region.h"
#include "ByteRegion.h"


DefineClassS(UinAreaStat);
DefineClassS(ObjConnect);
DefineClassS(Uin);

class UinAreaStat
{
  AnalyserOld*              mAnalyser;
  UinMetrics                mUinMetrics;

  int                       mCellWidth;
  int                       mCellHeight;
  int                       mSignalLengthMax;
  int                       mSignalLengthGood;
  int                       mSignalHeightMin;
  int                       mSignalHeightMax;
  int                       mPackLength;
  int                       mPackMaxSpace;

  struct Cell {
    Hyst      CellHyst;
    int       BlackValue;
    int       WhiteValue;
    int       CountBlack;
    int       CountWhite;
//    int       NeighborHorz;
//    int       NeighborVert;
    Cell(): BlackValue(0), WhiteValue(255) {}
  };

  struct Signal {
    int  Left;
    int  Right;
    int  Good;
    int  Height;
  };
  typedef QVector<Signal> SignalLine;

  struct Pack {
    int  Left;
    int  Right;
  };
  typedef QList<Pack> PackLine;

  struct Plate {
    struct Line {
      int Left;
      int Right;
    };
    int         Top;
    int         Bottom;
    int         WhiteIndex;
    int         BlackIndex;
    QList<Line> LineList;
  };

  Region<Cell>              mCellHyst;
  Region<Cell>              mCell4Hyst;
  Region<Cell>              mCell2Hyst;
  QVector<SignalLine>       mSignalRegion;
  QVector<SignalLine>       mSignalRegion2;
  QVector<PackLine>         mPackRegion;
  QVector<int>              mWhiteBlackCount;
  int                       mWhiteIndex;
  int                       mBlackIndex;
  QList<Plate>              mPlateList;
  ByteRegion                mThicknessRegion;
  ByteRegion                mThicknessRegion2;

  ByteRegion                mThicknessEdge;
  ObjConnectS               mObjConnect;
  QVector<QRect>            mObjList;

  typedef QVector<const QRect*> ObjList;
  struct ObjPack {
    QRect  Area;
    QPoint Left;
    QPoint Right;
    QPoint BottomLeft;
    QPoint BottomRight;
    int    Height;
  };
  Region<ObjList>           mObjRegion;
  QVector<ObjList>          mObjLineList;
  QVector<ObjPack>          mObjPackList;

  ObjPack*                  mObjPack;
  QPoint                    mObjPlateBottomRight;

  typedef QVector<QRect> RectPack;
  typedef QVector<RectPack> RectPackList;
  struct ObjPlate {
    ByteRegion     SourceRegion;
    Hyst           PlateHyst;
    ByteRegion     SourceNormalRegion;
    ByteRegion     NormalRegion;
    QVector<int>   VertHyst;
    UinMetrics     NormalMetrics;
    QVector<int>   SpaceList;
    UinS           UinTest;
    QVector<QRect> PossibleList;
    QVector<QRect> UinList;
    QVector<QChar> UinText;
  };
  QList<ObjPlate>           mObjPlateList;
  ObjPlate*                 mObjPlate;
  ByteRegion*               mObjPlateRegion;

  typedef QVector<QRect> UinList;
  struct CharInfo {
    int  Place;
    char Char;
  };
  UinS                      mUin;
  QMap<int, UinList>        mPlateUinMap;
  int                       mCurrentUinShift;
  UinList*                  mCurrentUinList;
  QList<ByteRegion>         mUinRegionStore;

private:
  const ByteRegion& Source() const { return mAnalyser->Source(); }
  int Width() const { return mAnalyser->Width(); }
  int Height() const { return mAnalyser->Height(); }

public:
  void Calc(const UinMetrics& _UinMetrics);
  void Calc();

  void DumpRaw(ByteRegion* debug, int minDiff);
  void DumpRaw2(ByteRegion* debug, int minDiff);
  void DumpRaw23(ByteRegion* debug, int minDiff);
  void DumpSignal(ByteRegion* debug);
  void DumpSignalLevel(ByteRegion* debug, int level);
  void DumpThickness2(ByteRegion* debug, int level, int threshold);
  void DumpThickness23(ByteRegion* debug, int level, int threshold);
  void DumpThicknessEdge(ByteRegion* debug);
  void DumpThicknessEdgeFiltered(ByteRegion* debug);
  void DumpBlack(ByteRegion* debug);
  void DumpWhite(ByteRegion* debug);
  void DumpCountBlack(ByteRegion* debug);
  void DumpCountWhite(ByteRegion* debug);
  void DumpWhiteLevel(ByteRegion* debug, int level, int diffLevel);
  void DumpBothLevel(ByteRegion* debug, int whiteLevel, int blackLevel);
  void DumpWhiteLevelCut(ByteRegion* debug, int level);
  void DumpCutLevel(ByteRegion* debug, int whiteLevel, int blackLevel);
  void DumpCutLevel2(ByteRegion* debug, int blackWhiteLevel);
  void DumpColorLevel(ByteRegion* debug, int level);
  void DumpMiddle(ByteRegion* debug);
  void DumpDiff(ByteRegion* debug);
  void DumpPlate(ByteRegion* debug, int index);
  void DumpPlateUinTest(ByteRegion* debug, int index);
  void DumpPlateUinDigits(ByteRegion* debug, int index);
  void DumpPlateNormal(ByteRegion* debug, int indexPlate, int indexChar);

private:
  void CalcCell();
  void CalcCell2();
  void CalcCell4Hyst();
//  void CalcNeighbor();
  void CalcSignal();
  void CalcSignal2();
  void CalcSignalPack();
  void CalcPlate();
  void CalcPlateOne(int whiteIndex, int blackIndex);
  void CalcThickness(const QVector<SignalLine>& signalRegion, ByteRegion& thicknessRegion);
  void CalcThicknessEdge(const ByteRegion& thicknessRegion);
  void CalcObjPack();
  void CalcObjPlates();
  bool CalcObjPlate();
  bool CalcObjPlateBottom();
  bool CalcObjPlateConstruct();
  bool CalcObjPlateTop();
  bool CalcObjPlateNormal();
  bool CalcObjPlateSpace();
  void CalcObjPlateUin();
  void CalcObjPlateUinFindPossible();
  void CalcObjPlateUinTestPossible();
  void CalcObjPlateUinTestPossibleOne(const QRect& rect, QRect* trimmedRect = nullptr);
  void CalcObjPlateUinEasy();
  void CalcObjPlateUinBase();
  void CalcObjPlateUinConstruct();
  void CalcObjPlateUinConstructTest(int height);
  void TryPlate(const QPoint& p1, const QPoint& p2, const QRect& seekRect);
  bool AddPlateLine(ObjList& line, const QRect& rect);
  void AnalyzePlate();
  bool FilterPlateOne(Plate* plate);
  void PrepareDump(ByteRegion* debug);
  void PrepareDumpWhite(ByteRegion* debug);
  void PrepareDumpBlack(ByteRegion* debug);

public:
  UinAreaStat(AnalyserOld* _Analyser);
  ~UinAreaStat();
};
