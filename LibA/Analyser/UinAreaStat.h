#pragma once

#include <QVector>
#include <QMap>
#include <QList>
#include <QImage>

#include <Lib/Include/Common.h>
#include <LibV/Include/Hyst.h>
#include <LibV/Include/Region.h>

#include "Analyser.h"
#include "UinMetrics.h"


DefineClassS(UinAreaStat);
DefineClassS(ObjConnect);
DefineClassS(Uin);

class UinAreaStat
{
  Analyser*                 mAnalyser;
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
  Region<uchar>             mThicknessRegion;
  Region<uchar>             mThicknessRegion2;

  Region<uchar>             mThicknessEdge;
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
    Region<uchar>  SourceRegion;
    Hyst           PlateHyst;
    Region<uchar>  SourceNormalRegion;
    Region<uchar>  NormalRegion;
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
  Region<uchar>*            mObjPlateRegion;

  typedef QVector<QRect> UinList;
  struct CharInfo {
    int  Place;
    char Char;
  };
  UinS                      mUin;
  QMap<int, UinList>        mPlateUinMap;
  int                       mCurrentUinShift;
  UinList*                  mCurrentUinList;
  QList<Region<uchar> >     mUinRegionStore;

private:
  const Region<uchar>& Source() const { return mAnalyser->Source(); }
  int Width() const { return mAnalyser->Width(); }
  int Height() const { return mAnalyser->Height(); }

public:
  void Calc(const UinMetrics& _UinMetrics);
  void Calc();

  void DumpRaw(Region<uchar>* debug, int minDiff);
  void DumpRaw2(Region<uchar>* debug, int minDiff);
  void DumpRaw23(Region<uchar>* debug, int minDiff);
  void DumpSignal(Region<uchar>* debug);
  void DumpSignalLevel(Region<uchar>* debug, int level);
  void DumpThickness2(Region<uchar>* debug, int level, int threshold);
  void DumpThickness23(Region<uchar>* debug, int level, int threshold);
  void DumpThicknessEdge(Region<uchar>* debug);
  void DumpThicknessEdgeFiltered(Region<uchar>* debug);
  void DumpBlack(Region<uchar>* debug);
  void DumpWhite(Region<uchar>* debug);
  void DumpCountBlack(Region<uchar>* debug);
  void DumpCountWhite(Region<uchar>* debug);
  void DumpWhiteLevel(Region<uchar>* debug, int level, int diffLevel);
  void DumpBothLevel(Region<uchar>* debug, int whiteLevel, int blackLevel);
  void DumpWhiteLevelCut(Region<uchar>* debug, int level);
  void DumpCutLevel(Region<uchar>* debug, int whiteLevel, int blackLevel);
  void DumpCutLevel2(Region<uchar>* debug, int blackWhiteLevel);
  void DumpColorLevel(Region<uchar>* debug, int level);
  void DumpMiddle(Region<uchar>* debug);
  void DumpDiff(Region<uchar>* debug);
  void DumpPlate(Region<uchar>* debug, int index);
  void DumpPlateUinTest(Region<uchar>* debug, int index);
  void DumpPlateUinDigits(Region<uchar>* debug, int index);
  void DumpPlateNormal(Region<uchar>* debug, int indexPlate, int indexChar);

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
  void CalcThickness(const QVector<SignalLine>& signalRegion, Region<uchar>& thicknessRegion);
  void CalcThicknessEdge(const Region<uchar>& thicknessRegion);
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
  void PrepareDump(Region<uchar>* debug);
  void PrepareDumpWhite(Region<uchar>* debug);
  void PrepareDumpBlack(Region<uchar>* debug);

public:
  UinAreaStat(Analyser* _Analyser);
  ~UinAreaStat();
};
