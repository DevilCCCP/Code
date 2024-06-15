#include <QDebug>

#include "UinAreaStat.h"
#include "ObjConnect.h"
#include "Uin.h"


const int   kCell2Size = 2;
const int   kBlackWhiteDiffMin = 16;
const int   kWhiteThreshold = 1;
const int   kBlackThreshold = 2;
const int   kPlateSquareMin = 8;
const int   kPackMinSignals = 3;
const int   kPlateObjectMin = 6;
const int   kPlateObjectLossMax = 2;
const int   kPlateHeightMin = 18;
const int   kPlateRectPossibleMin = 6;
const int   kPlateRectPossibleMax = 20;
const qreal kPlateEasyDigitMatchMin = 1.5;
const int   kPlateConstructDigitsMin = 3;
const int   kPlateTemplateSize = 6;
const char  kPlateTemplate[kPlateTemplateSize] = { 'c', 'd', 'd', 'd', 'c', 'c'};

template <typename T>
Q_DECL_CONSTEXPR inline const T &qMax3(const T &a, const T &b, const T &c) { return (a < b) ? ((b < c) ? c : b) : ((a < c) ? c : a); }

void UinAreaStat::Calc(const UinMetrics& _UinMetrics)
{
  mUinMetrics = _UinMetrics;

  Calc();
}

void UinAreaStat::Calc()
{
  if (!mUinMetrics.IsValid()) {
    return;
  }

  mCellWidth        = mUinMetrics.DigitWidth() * 3/2 + 1;
  mCellHeight       = mUinMetrics.DigitHeight()/4 + 1;
  mSignalLengthMax  = mUinMetrics.DigitWidth()*5/2 + 1;
  mSignalLengthGood = mUinMetrics.DigitThick() + 3;
  mSignalHeightMin  = mUinMetrics.DigitHeight() / 2 - 2;
  mSignalHeightMax  = mUinMetrics.DigitHeight()*3/2 + 2;
  mPackLength       = mUinMetrics.Width();
  mPackMaxSpace     = mUinMetrics.DigitWidth() * 5/2;

  CalcCell();
//  CalcNeighbor();
//  CalcSignal();
  CalcSignal2();
//  CalcSignalPack();

//  CalcThickness(mSignalRegion, mThicknessRegion);
  CalcThickness(mSignalRegion2, mThicknessRegion2);
  CalcThicknessEdge(mThicknessRegion2);

//  CalcCell4Hyst();

//  CalcPlate();
//  AnalyzePlate();
  CalcObjPack();
  CalcObjPlates();
}

void UinAreaStat::DumpRaw(ByteRegion* debug, int minDiff)
{
  PrepareDumpWhite(debug);

  if (mCellHyst.IsNull()) {
    return;
  }

  for (int j = 0; j < Height(); j++) {
    const uchar* src = Source().Line(j);
    uchar* dst = debug->Line(j);
    Cell* cell = mCellHyst.Line(j / mCellHeight);
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      bool good = cell->WhiteValue - cell->BlackValue >= minDiff;
      if (good) {
        int blackThreshold = (2*cell->BlackValue + cell->WhiteValue)/3;
        int whiteThreshold = (cell->BlackValue + 2*cell->WhiteValue)/3;
        for (int i = 0; i < mCellWidth; i++) {
          if (*src < blackThreshold) {
            *dst = 0;
          } else if (*src > whiteThreshold) {
            *dst = 255;
          } else {
            *dst = 127;
          }
          src++;
          dst++;
        }
      } else {
        for (int i = 0; i < mCellWidth; i++) {
          *dst = 255;
          src++;
          dst++;
        }
      }
      cell++;
    }
  }
}

void UinAreaStat::DumpRaw2(ByteRegion* debug, int minDiff)
{
  PrepareDump(debug);

  if (mCellHyst.IsNull()) {
    return;
  }

  for (int j = 0; j < Height(); j++) {
    int jCell = j / mCellHeight;
    const uchar* src = Source().Line(j);
    uchar* dst = debug->Line(j);
    Cell* cell  = mCellHyst.Line(jCell);
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      int width = qMin(mCellWidth, Width() - iCell*mCellWidth);
      bool good = cell->WhiteValue - cell->BlackValue >= minDiff;
      if (good) {
        int blackWhiteThreshold = (cell->BlackValue + cell->WhiteValue)/2;
        for (int i = 0; i < width; i++) {
          if (*src < blackWhiteThreshold) {
            *dst = 0;
          } else {
            *dst = 255;
          }
          src++;
          dst++;
        }
      } else {
        memset(dst, 255, width);
        src += width;
        dst += width;
      }
      cell++;
    }
  }
}

void UinAreaStat::DumpRaw23(ByteRegion* debug, int minDiff)
{
  PrepareDump(debug);

  if (mCellHyst.IsNull()) {
    return;
  }

  for (int j = 0; j < Height(); j++) {
    int jCell = j / mCellHeight;
    const uchar* src = Source().Line(j);
    uchar* dst = debug->Line(j);
    Cell* cell  = mCellHyst.Line(jCell);
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      int width = qMin(mCellWidth, Width() - iCell*mCellWidth);
      bool good = cell->WhiteValue - cell->BlackValue >= minDiff;
      if (good) {
        int blackWhiteThreshold = (cell->BlackValue + 2*cell->WhiteValue)/3;
        for (int i = 0; i < width; i++) {
          if (*src < blackWhiteThreshold) {
            *dst = 0;
          } else {
            *dst = 255;
          }
          src++;
          dst++;
        }
      } else {
        memset(dst, 255, width);
        src += width;
        dst += width;
      }
      cell++;
    }
  }
}

void UinAreaStat::DumpSignal(ByteRegion* debug)
{
  PrepareDumpWhite(debug);

  if (mPackRegion.isEmpty() || mSignalRegion.isEmpty()) {
    return;
  }

  for (int j = 0; j < Height(); j++) {
    uchar* dst = debug->Line(j);
    PackLine* packLine = &mPackRegion[j];
    foreach (const Pack& pack, *packLine) {
      for (int i = pack.Left; i < pack.Right; i++) {
        dst[i] = 210;
      }
    }
  }
  for (int j = 0; j < Height(); j++) {
    uchar* dst = debug->Line(j);
    SignalLine* signalLine = &mSignalRegion[j];
    foreach (const Signal& signal, *signalLine) {
      int color = qMax(0, 128 - signal.Height * 8);
      if (!(signal.Height >= mSignalHeightMin && signal.Height <= mSignalHeightMax)) {
        color = 170;
      } else if (!signal.Good) {
        color = 128;
      }
      for (int i = signal.Left; i < signal.Right; i++) {
        dst[i] = color;
      }
    }
  }
}

void UinAreaStat::DumpSignalLevel(ByteRegion* debug, int level)
{
  PrepareDumpWhite(debug);

  if (mSignalRegion.isEmpty()) {
    return;
  }

  int minWidth = level - 1;
  int maxWidth = 2*level + 1;
  for (int j = 0; j < Height(); j++) {
    uchar* dst = debug->Line(j);
    SignalLine* signalLine = &mSignalRegion[j];
    foreach (const Signal& signal, *signalLine) {
      int width = signal.Right - signal.Left;
      if (width >= minWidth && width <= maxWidth) {
        for (int i = signal.Left; i < signal.Right; i++) {
          dst[i] = 50;
        }
      }
    }
  }
}

void UinAreaStat::DumpThickness2(ByteRegion* debug, int level, int threshold)
{
  PrepareDumpWhite(debug);

  if (mThicknessRegion2.IsNull()) {
    return;
  }

  for (int j = 0; j < Height(); j++) {
    const uchar* thick  = mThicknessRegion2.Data(1, j);
    const uchar* thickm = j > 0? mThicknessRegion2.Data(1, j - 1): mThicknessRegion2.Data(1, j);
    const uchar* thickp = j < Height() - 1? mThicknessRegion2.Data(1, j + 1): mThicknessRegion2.Data(1, j);
    uchar* dst = debug->Data(1, j);
    uchar* dstm = j > 0? debug->Data(1, j - 1): debug->Data(1, j);
    uchar* dstp = j < Height() - 1? debug->Data(1, j + 1): debug->Data(1, j);
    for (int i = 1; i < Width() - 1; i++) {
      if (*thick && qAbs(*thick - level) < threshold
          && *thick >= thick[-1] && *thick >= thick[1]
          && *thick >= *thickm && *thick >= thickm[-1] && *thick >= thickm[1]
          && *thick >= *thickp && *thick >= thickp[-1] && *thick >= thickp[1]
          ) {
        dst[0] = dst[-1] = dst[1] = dstm[0] = dstm[-1] = dstm[1] = dstp[0] = dstp[-1] = dstp[1] = 0;
      }
      thick++;
      thickm++;
      thickp++;
      dst++;
      dstm++;
      dstp++;
    }
  }
}

void UinAreaStat::DumpThickness23(ByteRegion* debug, int level, int threshold)
{
  PrepareDumpWhite(debug);

  if (mThicknessRegion.IsNull()) {
    return;
  }

  for (int j = 0; j < Height(); j++) {
    const uchar* thick  = mThicknessRegion.Data(1, j);
    const uchar* thickm = j > 0? mThicknessRegion.Data(1, j - 1): mThicknessRegion.Data(1, j);
    const uchar* thickp = j < Height() - 1? mThicknessRegion.Data(1, j + 1): mThicknessRegion.Data(1, j);
    uchar* dst = debug->Data(1, j);
    uchar* dstm = j > 0? debug->Data(1, j - 1): debug->Data(1, j);
    uchar* dstp = j < Height() - 1? debug->Data(1, j + 1): debug->Data(1, j);
    for (int i = 1; i < Width() - 1; i++) {
      if (*thick && qAbs(*thick - level) < threshold
          && *thick >= thick[-1] && *thick >= thick[1]
          && *thick >= *thickm && *thick >= thickm[-1] && *thick >= thickm[1]
          && *thick >= *thickp && *thick >= thickp[-1] && *thick >= thickp[1]
          ) {
        dst[0] = dst[-1] = dst[1] = dstm[0] = dstm[-1] = dstm[1] = dstp[0] = dstp[-1] = dstp[1] = 0;
      }
      thick++;
      thickm++;
      thickp++;
      dst++;
      dstm++;
      dstp++;
    }
  }
}

void UinAreaStat::DumpThicknessEdge(ByteRegion* debug)
{
  PrepareDumpWhite(debug);

  if (mThicknessEdge.IsNull()) {
    return;
  }

  uchar color = 0;
  for (int j = 0; j < Height(); j++) {
    const uchar* src = mThicknessEdge.Line(j);
    uchar* dst = debug->Line(j);
    for (int i = 0; i < Width(); i++) {
      if (*src) {
        *dst = color;
      }
      src++;
      dst++;
    }
    color += 0x40;
    if (color > 0xd0) {
      color = 0;
    }
  }
}

void UinAreaStat::DumpThicknessEdgeFiltered(ByteRegion* debug)
{
  PrepareDumpWhite(debug);

  if (mThicknessEdge.IsNull()) {
    return;
  }

  uchar color = 0;
  foreach (const QRect& obj, mObjList) {
    for (int j = obj.top(); j <= obj.bottom(); j++) {
      const uchar* src = mThicknessEdge.Data(obj.left(), j);
      uchar* dst = debug->Data(obj.left(), j);
      for (int i = obj.left(); i <= obj.right(); i++) {
        if (*src) {
          *dst = color;
        }
        src++;
        dst++;
      }
    }
    color += 0x40;
    if (color > 0xd0) {
      color = 0;
    }
  }
}

void UinAreaStat::DumpBlack(ByteRegion *debug)
{
  PrepareDumpWhite(debug);

  if (mCellHyst.IsNull()) {
    return;
  }

  for (int j = 0; j < Height(); j++) {
    uchar* dst = debug->Line(j);
    Cell* cell = mCellHyst.Line(j / mCellHeight);
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      int value = cell->BlackValue;
      for (int i = 0; i < mCellWidth; i++) {
        *dst = value;
        dst++;
      }
      cell++;
    }
  }
}

void UinAreaStat::DumpWhite(ByteRegion *debug)
{
  PrepareDumpWhite(debug);

  if (mCellHyst.IsNull()) {
    return;
  }

  for (int j = 0; j < Height(); j++) {
    uchar* dst = debug->Line(j);
    Cell* cell = mCellHyst.Line(j / mCellHeight);
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      int value = cell->WhiteValue;
      for (int i = 0; i < mCellWidth; i++) {
        *dst = value;
        dst++;
      }
      cell++;
    }
  }

  for (int jCell = 0; jCell < mCellHyst.Height() - 1; jCell++) {
    for (int iCell = 0; iCell < mCellHyst.Width() - 1; iCell++) {
//      if (mCellHyst.Data(iCell, jCell)->NeighborHorz >= 3 && mCellHyst.Data(iCell, jCell)->NeighborVert >= 3) {
        *debug->Data(iCell * mCellWidth + mCellWidth/2 - 1, jCell * mCellHeight + mCellHeight/2 - 1) = mCellHyst.Data(iCell, jCell)->BlackValue;
        *debug->Data(iCell * mCellWidth + mCellWidth/2, jCell * mCellHeight + mCellHeight/2 - 1) = mCellHyst.Data(iCell, jCell)->BlackValue;
        *debug->Data(iCell * mCellWidth + mCellWidth/2 - 1, jCell * mCellHeight + mCellHeight/2) = mCellHyst.Data(iCell, jCell)->BlackValue;
        *debug->Data(iCell * mCellWidth + mCellWidth/2, jCell * mCellHeight + mCellHeight/2) = mCellHyst.Data(iCell, jCell)->BlackValue;
//      }
    }
  }

//  int jCell = 36;
//  int iCell = 14;
//  for (int j = 0; j < mCellHeight; j++) {
//    uchar* dst = debug->Line(jCell * mCellHeight + j);
//    int value = (j % 2)? 0: 255;
//    for (int i = 0; i < mCellWidth; i++) {
//      dst[iCell * mCellWidth + i] = value;
//    }
//  }
}

void UinAreaStat::DumpCountBlack(ByteRegion* debug)
{
  PrepareDumpWhite(debug);

  if (mCellHyst.IsNull() || mCell4Hyst.IsNull()) {
    return;
  }

  for (int j = 0; j < Height(); j++) {
    uchar* dst = debug->Line(j);
    int jCell = j / mCellHeight;
    Cell* cell4[2] = { jCell > 0? mCell4Hyst.Line(jCell - 1): nullptr
                       , jCell < mCell4Hyst.Height()? mCell4Hyst.Line(jCell): nullptr };
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      bool fit = false;
      int count = mCellWidth * mCellHeight * 4;
      for (int j = 0; j < 2; j++) if (cell4[j]) {
        Cell* jcell4[2] = { (iCell > 0)? &cell4[j][iCell - 1]: nullptr
                            , (iCell < mCell4Hyst.Width())? &cell4[j][iCell]: nullptr };
        for (int i = 0; i < 2; i++) {
          Cell* cell = jcell4[i];
          if (cell) {
            bool fit_ = cell->WhiteValue - cell->BlackValue >= (2 << kHystFastShift);
            if (fit_) {
              count = qMin(count, cell->CountBlack);
              fit = true;
            }
          }
        }
      }
      int value = fit? count * 255 / (mCellWidth * mCellHeight * 4): 0;
      for (int i = 0; i < mCellWidth; i++) {
        *dst = value;
        dst++;
      }
    }
  }
}

void UinAreaStat::DumpCountWhite(ByteRegion* debug)
{
  PrepareDumpWhite(debug);

  if (mCellHyst.IsNull() || mCell4Hyst.IsNull()) {
    return;
  }

  for (int j = 0; j < Height(); j++) {
    uchar* dst = debug->Line(j);
    int jCell = j / mCellHeight;
    Cell* cell4[2] = { jCell > 0? mCell4Hyst.Line(jCell - 1): nullptr
                       , jCell < mCell4Hyst.Height()? mCell4Hyst.Line(jCell): nullptr };
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      bool fit = false;
      int count = 0;
      for (int j = 0; j < 2; j++) if (cell4[j]) {
        Cell* jcell4[2] = { (iCell > 0)? &cell4[j][iCell - 1]: nullptr
                            , (iCell < mCell4Hyst.Width())? &cell4[j][iCell]: nullptr };
        for (int i = 0; i < 2; i++) {
          Cell* cell = jcell4[i];
          if (cell) {
            bool fit_ = cell->WhiteValue - cell->BlackValue >= (2 << kHystFastShift);
            if (fit_) {
              count = qMax(count, cell->CountWhite);
              fit = true;
            }
          }
        }
      }
      int value = fit? count * 255 / (mCellWidth * mCellHeight * 4): 0;
      for (int i = 0; i < mCellWidth; i++) {
        *dst = value;
        dst++;
      }
    }
  }
}

void UinAreaStat::DumpWhiteLevel(ByteRegion* debug, int level, int diffLevel)
{
  PrepareDumpWhite(debug);

  if (mCellHyst.IsNull() || mCell4Hyst.IsNull()) {
    return;
  }

  for (int j = 0; j < Height(); j++) {
    uchar* dst = debug->Line(j);
    int jCell = j / mCellHeight;
    Cell* cell4[2] = { jCell > 0? mCell4Hyst.Line(jCell - 1): nullptr
                       , jCell < mCell4Hyst.Height()? mCell4Hyst.Line(jCell): nullptr };
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      bool fit = false;
      int whiteValue = 0;
      int blackValue = 0;
      for (int j = 0; j < 2; j++) if (cell4[j]) {
        Cell* jcell4[2] = { (iCell > 0)? &cell4[j][iCell - 1]: nullptr
                            , (iCell < mCell4Hyst.Width())? &cell4[j][iCell]: nullptr };
        for (int i = 0; i < 2; i++) {
          Cell* cell = jcell4[i];
          if (cell) {
            bool fit_ = qAbs(cell->WhiteValue - (level << kHystFastShift)) <= (diffLevel << kHystFastShift)
                && cell->WhiteValue - cell->BlackValue >= (2 << kHystFastShift)
                /*&& cell->CountWhite > mCellWidth * mCellHeight * 4/2*/;
            if (fit_) {
              if (qAbs(cell->WhiteValue - (level << kHystFastShift)) < qAbs(cell->WhiteValue - whiteValue)) {
                whiteValue = cell->WhiteValue;
                blackValue = cell->BlackValue;
              }
              fit = true;
            }
          }
        }
      }
      int value = fit? whiteValue: 0;
      for (int i = 0; i < mCellWidth; i++) {
        *dst = value;
        dst++;
      }
      if (j % mCellHeight == mCellHeight/2) {
        dst[-mCellWidth/2] = blackValue;
        dst[-mCellWidth/2 + 1] = blackValue;
      }
    }
  }
}

void UinAreaStat::DumpBothLevel(ByteRegion* debug, int whiteLevel, int blackLevel)
{
  PrepareDumpWhite(debug);

  if (mCellHyst.IsNull() || mCell4Hyst.IsNull()) {
    return;
  }

  for (int j = 0; j < Height(); j++) {
    uchar* dst = debug->Line(j);
    int jCell = j / mCellHeight;
    Cell* cell4[2] = { jCell > 0? mCell4Hyst.Line(jCell - 1): nullptr
                       , jCell < mCell4Hyst.Height()? mCell4Hyst.Line(jCell): nullptr };
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      bool fit = false;
      int whiteValue = 0;
      int blackValue = 0;
      for (int j = 0; j < 2; j++) if (cell4[j]) {
        Cell* jcell4[2] = { (iCell > 0)? &cell4[j][iCell - 1]: nullptr
                            , (iCell < mCell4Hyst.Width())? &cell4[j][iCell]: nullptr };
        for (int i = 0; i < 2; i++) {
          Cell* cell = jcell4[i];
          if (cell) {
            bool fit_ = qAbs(cell->WhiteValue - (whiteLevel << kHystFastShift)) <= (1 << kHystFastShift)
                && qAbs(cell->BlackValue - (blackLevel << kHystFastShift)) <= (2 << kHystFastShift);
            if (fit_) {
              if (qAbs(cell->WhiteValue - (whiteLevel << kHystFastShift)) < qAbs(cell->WhiteValue - whiteValue)) {
                whiteValue = cell->WhiteValue;
                blackValue = cell->BlackValue;
              }
              fit = true;
            }
          }
        }
      }
      int value = fit? whiteValue: 0;
      for (int i = 0; i < mCellWidth; i++) {
        *dst = value;
        dst++;
      }
      if (j % mCellHeight == mCellHeight/2) {
        dst[-mCellWidth/2] = blackValue;
        dst[-mCellWidth/2 + 1] = blackValue;
      }
    }
  }
}

void UinAreaStat::DumpWhiteLevelCut(ByteRegion* debug, int level)
{
  PrepareDumpBlack(debug);

  if (mCellHyst.IsNull() || mCell4Hyst.IsNull()) {
    return;
  }

  int cutValue = ((level << kHystFastShift) - kBlackWhiteDiffMin);

  for (int j = 0; j < Height(); j++) {
    const uchar* src = Source().Line(j);
    uchar* dst = debug->Line(j);
    int jCell = j / mCellHeight;
    Cell* cell4[2] = { jCell > 0? mCell4Hyst.Line(jCell - 1): nullptr
                       , jCell < mCell4Hyst.Height()? mCell4Hyst.Line(jCell): nullptr };
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      bool fit = false;
      int whiteValue = 0;
//      int blackValue = 0;
      for (int j = 0; j < 2; j++) if (cell4[j]) {
        Cell* jcell4[2] = { (iCell > 0)? &cell4[j][iCell - 1]: nullptr
                            , (iCell < mCell4Hyst.Width())? &cell4[j][iCell]: nullptr };
        for (int i = 0; i < 2; i++) {
          Cell* cell = jcell4[i];
          if (cell) {
            bool fit_ = qAbs(cell->WhiteValue - (level << kHystFastShift)) <= kBlackWhiteDiffMin
                && cell->WhiteValue - cell->BlackValue >= (2 << kHystFastShift)
                && cell->CountWhite > mCellWidth * mCellHeight * 4/2;
            if (fit_) {
              if (qAbs(cell->WhiteValue - (level << kHystFastShift)) < qAbs(cell->WhiteValue - whiteValue)) {
                whiteValue = cell->WhiteValue;
//                blackValue = cell->BlackValue;
              }
              fit = true;
            }
          }
        }
      }
      if (fit) {
        for (int i = 0; i < mCellWidth; i++) {
          if (*src > cutValue) {
            *dst = 255;
          }
          src++;
          dst++;
        }
      } else {
        src += mCellWidth;
        dst += mCellWidth;
      }
    }
  }
}

void UinAreaStat::DumpCutLevel(ByteRegion* debug, int whiteLevel, int blackLevel)
{
  PrepareDumpWhite(debug);

  int blackThreshold = blackLevel << kHystFastShift;
  int whiteThreshold = whiteLevel << kHystFastShift;

  for (int j = 0; j < Height(); j++) {
    const uchar* src = mAnalyser->Source().Line(j);
    uchar* dst = debug->Line(j);
    for (int i = 0; i < Width(); i++) {
      if (*src < blackThreshold) {
        *dst = 0;
      } else if (*src > whiteThreshold) {
        *dst = 255;
      } else {
        *dst = 127;
      }

      dst++;
      src++;
    }
  }
}

void UinAreaStat::DumpCutLevel2(ByteRegion* debug, int blackWhiteLevel)
{
  PrepareDumpWhite(debug);

  int blackWhiteThreshold = blackWhiteLevel << kHystFastShift;

  for (int j = 0; j < Height(); j++) {
    const uchar* src = mAnalyser->Source().Line(j);
    uchar* dst = debug->Line(j);
    for (int i = 0; i < Width(); i++) {
      if (*src < blackWhiteThreshold) {
        *dst = 0;
      }

      dst++;
      src++;
    }
  }
}

void UinAreaStat::DumpColorLevel(ByteRegion* debug, int level)
{
  PrepareDumpWhite(debug);

  int blackThreshold = (level) << kHystFastShift;
  int whiteThreshold = (level + 1) << kHystFastShift;

  for (int j = 0; j < Height(); j++) {
    const uchar* src = mAnalyser->Source().Line(j);
    uchar* dst = debug->Line(j);
    for (int i = 0; i < Width(); i++) {
      if (*src >= blackThreshold && *src < whiteThreshold) {
        *dst = 0;
      } else {
        *dst = 255;
      }

      dst++;
      src++;
    }
  }
}

void UinAreaStat::DumpMiddle(ByteRegion* debug)
{
  PrepareDumpWhite(debug);

  if (mCellHyst.IsNull()) {
    return;
  }

  for (int j = 0; j < Height(); j++) {
    uchar* dst = debug->Line(j);
    Cell* cell = mCellHyst.Line(j / mCellHeight);
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      int value = (cell->BlackValue + cell->WhiteValue)/2;
      for (int i = 0; i < mCellWidth; i++) {
        *dst = value;
        dst++;
      }
      cell++;
    }
  }
}

void UinAreaStat::DumpDiff(ByteRegion* debug)
{
  PrepareDumpWhite(debug);

  if (mCellHyst.IsNull()) {
    return;
  }

  for (int j = 0; j < Height(); j++) {
    uchar* dst = debug->Line(j);
    Cell* cell = mCellHyst.Line(j / mCellHeight);
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      for (int i = 0; i < mCellWidth; i++) {
        *dst = cell->WhiteValue - cell->BlackValue;
        dst++;
      }
      cell++;
    }
  }
}

void UinAreaStat::DumpPlate(ByteRegion* debug, int index)
{
  if (index < 0 || index >= mObjLineList.size()) {
    PrepareDumpBlack(debug);
    return;
  }

  PrepareDumpWhite(debug);
  const ObjPack* objPack = &mObjPackList.at(index);
  for (int j = objPack->Area.top(); j < objPack->Area.bottom(); j++) {
    const uchar* src = Source().Data(objPack->Area.left(), j);
    uchar* dst = debug->Data(objPack->Area.left(), j);
    memcpy(dst, src, objPack->Area.right() - objPack->Area.left());
  }
  debug->FillLine(QPoint(objPack->Left.x(), qMax(0, objPack->Left.y() - objPack->Height)), QPoint(objPack->Right.x(), qMax(0, objPack->Right.y() - objPack->Height)), 127);
  debug->FillLine(objPack->Left, objPack->Right, 127);
  debug->FillLine(objPack->BottomLeft, objPack->BottomRight, 40);
  debug->FillRectBorder(objPack->Area, 127);
}

void UinAreaStat::DumpPlateUinTest(ByteRegion* debug, int index)
{
  if (index < 0 || index >= mObjPlateList.size()) {
    PrepareDumpBlack(debug);
    return;
  }

  const ObjPlate* objPlate = &mObjPlateList.at(index);
  objPlate->UinTest->DumpTest(debug);
}

void UinAreaStat::DumpPlateUinDigits(ByteRegion* debug, int index)
{
  if (index < 0 || index >= mObjPlateList.size()) {
    PrepareDumpBlack(debug);
    return;
  }

  const ObjPlate* objPlate = &mObjPlateList.at(index);
  objPlate->UinTest->DumpDigits(debug);
}

void UinAreaStat::DumpPlateNormal(ByteRegion* debug, int indexPlate, int indexChar)
{
  if (indexPlate < 0 || indexPlate >= mObjPlateList.size()) {
    PrepareDumpBlack(debug);
    return;
  }
  PrepareDumpWhite(debug);

  mObjPlate = &mObjPlateList[indexPlate];
  mObjPlateRegion = &mObjPlate->NormalRegion;

  debug->Copy(*mObjPlateRegion, QRect(0, 0, mObjPlateRegion->Width(), mObjPlateRegion->Height()));
//  auto hystFrom = mObjPlate->VertHyst.begin();
//  auto hystTo   = mObjPlate->VertHyst.begin() + qMin(mObjPlate->NormalMetrics.DigitWidth(), mObjPlate->VertHyst.size());
//  for (int i = 0; i < mObjPlate->VertHyst.size(); i++) {
//    int value = *std::max_element(hystFrom, hystTo)/4;
//    for (int j = 0; j < value; j++) {
//      *debug->Data(i, mObjPlateRegion->Height() + j) = 127;
//    }
//    if (hystTo != mObjPlate->VertHyst.end()) {
//      hystFrom++;
//      hystTo++;
//    }
//  }
//  int vmax = std::max(*std::max_element(mObjPlate->VertHyst.begin(), mObjPlate->VertHyst.end()), 1);
  foreach (int index, mObjPlate->SpaceList) {
    for (int j = 0; j < mObjPlateRegion->Height(); j++) {
      *debug->Data(index, mObjPlateRegion->Height() + j) = 80;
    }
  }
  for (int i = 0; i < mObjPlate->VertHyst.size(); i++) {
    int value = mObjPlate->VertHyst.at(i)/4;
    for (int j = 0; j < value; j++) {
      *debug->Data(i, mObjPlateRegion->Height() + j) = 0;
    }
  }
  debug->FillLine(QPoint(0, 4*mObjPlateRegion->Height()), QPoint(mObjPlate->NormalMetrics.DigitWidth(), 4*mObjPlateRegion->Height()), 80);

  if (indexChar == 0) {
    foreach (const QRect& rect, mObjPlate->UinList) {
      debug->FillRectBorder(rect, 40);
    }
  } else if (indexChar <= mObjPlate->UinList.size()) {
    debug->FillRectBorder(mObjPlate->UinList.at(indexChar - 1), 40);
  }
}

void UinAreaStat::CalcCell()
{
  mCellHyst.SetSize((Width() + mCellWidth - 1) / mCellWidth, (Height() + mCellHeight - 1) / mCellHeight);
  for (int j = 0; j < Height(); j++) {
    const uchar* src = Source().Line(j);
    Cell* cell = mCellHyst.Line(j / mCellHeight);
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      for (int i = 0; i < mCellWidth; i++) {
        cell->CellHyst.Inc(*src);
        src++;
      }
      cell++;
    }
  }

  for (int jCell = 0; jCell < mCellHyst.Height(); jCell++) {
    Cell* cell = mCellHyst.Line(jCell);
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      int minValue  = cell->CellHyst.GetValue(50);
      int maxValue  = cell->CellHyst.GetValue(950);
      cell->BlackValue      = minValue/* + kHystFastLength/2*/;
      cell->WhiteValue      = maxValue/* + kHystFastLength/2*/;

      int blackThreshold = (2*cell->BlackValue + cell->WhiteValue)/3;
      int whiteThreshold = (cell->BlackValue + 2*cell->WhiteValue)/3;
      cell->CountBlack      = cell->CellHyst.LessCount(blackThreshold);
      cell->CountWhite      = cell->CellHyst.GreaterCount(whiteThreshold);

      cell++;
    }
  }
}

void UinAreaStat::CalcCell2()
{
  mCell2Hyst.SetSize((mCellHyst.Width() + kCell2Size - 1) / kCell2Size, (mCellHyst.Height() + kCell2Size - 1) / kCell2Size);
  for (int jCell = 0; jCell < mCellHyst.Height(); jCell++) {
    Cell* cell = mCellHyst.Line(jCell);
    Cell* cell2 = mCell2Hyst.Line(jCell / kCell2Size);
    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
      int minValue  = cell->CellHyst.GetValue(50);
      int maxValue  = cell->CellHyst.GetValue(950);
      cell->BlackValue      = minValue;
      cell->WhiteValue      = maxValue;

      int blackThreshold = (2*cell->BlackValue + cell->WhiteValue)/3;
      int whiteThreshold = (cell->BlackValue + 2*cell->WhiteValue)/3;
      cell->CountBlack      = cell->CellHyst.LessCount(blackThreshold);
      cell->CountWhite      = cell->CellHyst.GreaterCount(whiteThreshold);

      cell2[iCell / kCell2Size].CellHyst.Add(cell->CellHyst);

      cell++;
    }
  }
}

void UinAreaStat::CalcCell4Hyst()
{
  mCell4Hyst.SetSize(mCellHyst.Width() - 1, mCellHyst.Height() - 1);
  for (int jCell = 0; jCell < mCell4Hyst.Height(); jCell++) {
    Cell* cell4 = mCell4Hyst.Line(jCell);
    for (int iCell = 0; iCell < mCell4Hyst.Width(); iCell++) {
      for (int j = 0; j < 2; j++) {
        Cell* jcell = mCellHyst.Data(iCell, jCell + j);
        for (int i = 0; i < 2; i++) {
          cell4->CellHyst.Add(jcell->CellHyst);
          jcell++;
        }
      }
      cell4++;
    }
  }

  for (int jCell = 0; jCell < mCell4Hyst.Height(); jCell++) {
    Cell* cell4 = mCell4Hyst.Line(jCell);
    for (int iCell = 0; iCell < mCell4Hyst.Width(); iCell++) {
      int minValue  = cell4->CellHyst.GetValue(50);
      int maxValue  = cell4->CellHyst.GetValue(950);
      cell4->BlackValue      = minValue;
      cell4->WhiteValue      = maxValue;

      int blackThreshold = (2*cell4->BlackValue + cell4->WhiteValue)/3;
      int whiteThreshold = (cell4->BlackValue + 2*cell4->WhiteValue)/3;
      cell4->CountBlack      = cell4->CellHyst.LessCount(blackThreshold);
      cell4->CountWhite      = cell4->CellHyst.GreaterCount(whiteThreshold);

      cell4++;
    }
  }
}

//class Neighbor
//{
//  int mMaxDiff;

//  int mCount;
//  int mValueMin;
//  int mValueMax;

//public:
//  int Count() const { return mCount; }

//public:
//  void Start(int value)
//  {
//    mCount = 1;
//    mValueMin = mValueMax = value;
//  }

//  bool Add(int value)
//  {
//    if (value < mValueMin) {
//      if (mValueMax - value <= mMaxDiff) {
//        mValueMin = value;
//        mCount++;
//        return true;
//      }
//    } else if (value > mValueMax) {
//      if (value - mValueMin <= mMaxDiff) {
//        mValueMax = value;
//        mCount++;
//        return true;
//      }
//    } else {
//      mCount++;
//      return true;
//    }
//    return false;
//  }

//public:
//  Neighbor()
//    : mMaxDiff((2 << kHystFastShift) - 2)
//  { }
//};

//void UinAreaStat::CalcNeighbor()
//{
//  for (int jCell = 0; jCell < mCellHyst.Height(); jCell++) {
//    Cell* cell = mCellHyst.Line(jCell);
//    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
//      cell->NeighborHorz = 0;
//      cell->NeighborVert = 0;
//      cell++;
//    }
//  }
//  Neighbor neighbor;
//  for (int jCell = 0; jCell < mCellHyst.Height(); jCell++) {
//    for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
//      neighbor.Start(mCellHyst.Data(iCell, jCell)->WhiteValue);
//      int iMaxCell = iCell + 1;
//      for (; iMaxCell < mCellHyst.Width(); iMaxCell++) {
//        if (!neighbor.Add(mCellHyst.Data(iMaxCell, jCell)->WhiteValue)) {
//          break;
//        }
//      }
//      for (int i = iCell; i < iMaxCell; i++) {
//        mCellHyst.Data(i, jCell)->NeighborHorz = qMax(mCellHyst.Data(i, jCell)->NeighborHorz, neighbor.Count());
//      }
//    }
//  }
//  for (int iCell = 0; iCell < mCellHyst.Width(); iCell++) {
//    for (int jCell = 0; jCell < mCellHyst.Height(); jCell++) {
//      neighbor.Start(mCellHyst.Data(iCell, jCell)->WhiteValue);
//      int jMaxCell = jCell + 1;
//      for (; jMaxCell < mCellHyst.Height(); jMaxCell++) {
//        if (!neighbor.Add(mCellHyst.Data(iCell, jMaxCell)->WhiteValue)) {
//          break;
//        }
//      }
//      for (int j = jCell; j < jMaxCell; j++) {
//        mCellHyst.Data(iCell, j)->NeighborVert = qMax(mCellHyst.Data(iCell, j)->NeighborVert, neighbor.Count());
//      }
//    }
//  }
//}

void UinAreaStat::CalcSignal()
{
  mSignalRegion.clear();
  mSignalRegion.resize(Height());

  for (int j = 0; j < Height(); j++) {
    int jCell = j / mCellHeight;
    const uchar* src = Source().Line(j);
    Cell* cell  = mCellHyst.Line(jCell);
    SignalLine* signalLine = &mSignalRegion[j];
    SignalLine* signalLineTop = j > 0? &mSignalRegion[j - 1]: nullptr;
    Signal* signalTopLast = signalLineTop && !signalLineTop->isEmpty()? &signalLineTop->first(): nullptr;
    int signalTopLastIndex = 0;
    signalLine->reserve(Width()/2);
    int signalLength = 0;
    for (int iCell = 0; iCell < mCellHyst.Width() - 1; iCell++) {
      bool good = cell->WhiteValue - cell->BlackValue >= kBlackWhiteDiffMin;
      int blackWhiteThreshold = good? (cell->BlackValue + 2*cell->WhiteValue)/3: 0;
      for (int i = 0; i < mCellWidth; i++) {
        bool hasSignal = *src < blackWhiteThreshold;
        if (hasSignal) {
          signalLength++;
        } else if (signalLength > 0) {
          if (signalLength <= mSignalLengthMax) {
            signalLine->append(Signal());
            Signal* signal = &signalLine->last();
            signal->Right  = iCell*mCellWidth + i;
            signal->Left   = signal->Right - signalLength;
            signal->Good   = signalLength <= mSignalLengthGood? 1: 0;
            signal->Height = 1;
            while (signalTopLast && signalTopLast->Right < signal->Left) {
              signalTopLast = (++signalTopLastIndex < signalLineTop->size())? signalTopLast + 1: nullptr;
            }
            while (signalTopLast) {
              if (signalTopLast->Left <= signal->Right) {
                signal->Good  += signalTopLast->Good;
                signal->Height = signalTopLast->Height + 1;
              }
              if (signalTopLast->Right > signal->Right) {
                break;
              }
              signalTopLast = (++signalTopLastIndex < signalLineTop->size())? signalTopLast + 1: nullptr;
            }
          }
          signalLength = 0;
        }
        src++;
      }
      cell++;
    }
  }

  for (int j = Height() - 1; j > 0; j--) {
    SignalLine* signalLine = &mSignalRegion[j];
    SignalLine* signalLineTop = &mSignalRegion[j-1];
    Signal* signalLast = signalLine && !signalLine->isEmpty()? &signalLine->first(): nullptr;
    Signal* signalTopLast = signalLineTop && !signalLineTop->isEmpty()? &signalLineTop->first(): nullptr;
    int signalLastIndex = 0;
    int signalTopLastIndex = 0;
    while (signalLast && signalTopLast) {
      if (signalTopLast->Right >= signalLast->Left && signalTopLast->Left <= signalLast->Right) {
        if (signalTopLast->Height < signalLast->Height) {
          signalTopLast->Good   = signalLast->Good;
          signalTopLast->Height = signalLast->Height;
        }
      }
      if (signalTopLast->Right <= signalLast->Right) {
        signalTopLast = (++signalTopLastIndex < signalLineTop->size())? signalTopLast + 1: nullptr;
      } else {
        signalLast = (++signalLastIndex < signalLine->size())? signalLast + 1: nullptr;
      }
    }
  }
}

void UinAreaStat::CalcSignal2()
{
  mSignalRegion2.clear();
  mSignalRegion2.resize(Height());

  for (int j = 0; j < Height(); j++) {
    int jCell = j / mCellHeight;
    const uchar* src = Source().Line(j);
    Cell* cell  = mCellHyst.Line(jCell);
    SignalLine* signalLine = &mSignalRegion2[j];
    SignalLine* signalLineTop = j > 0? &mSignalRegion2[j - 1]: nullptr;
    Signal* signalTopLast = signalLineTop && !signalLineTop->isEmpty()? &signalLineTop->first(): nullptr;
    int signalTopLastIndex = 0;
    signalLine->reserve(Width()/2);
    int signalLength = 0;
    for (int iCell = 0; iCell < mCellHyst.Width() - 1; iCell++) {
      bool good = cell->WhiteValue - cell->BlackValue >= kBlackWhiteDiffMin;
      int blackWhiteThreshold = good? (cell->BlackValue + cell->WhiteValue)/2: 0;
      for (int i = 0; i < mCellWidth; i++) {
        bool hasSignal = *src < blackWhiteThreshold;
        if (hasSignal) {
          signalLength++;
        } else if (signalLength > 0) {
          if (signalLength <= mSignalLengthMax) {
            signalLine->append(Signal());
            Signal* signal = &signalLine->last();
            signal->Right  = iCell*mCellWidth + i;
            signal->Left   = signal->Right - signalLength;
            signal->Good   = signalLength <= mSignalLengthGood? 1: 0;
            signal->Height = 1;
            while (signalTopLast && signalTopLast->Right < signal->Left) {
              signalTopLast = (++signalTopLastIndex < signalLineTop->size())? signalTopLast + 1: nullptr;
            }
            while (signalTopLast) {
              if (signalTopLast->Left <= signal->Right) {
                signal->Good  += signalTopLast->Good;
                signal->Height = signalTopLast->Height + 1;
              }
              if (signalTopLast->Right > signal->Right) {
                break;
              }
              signalTopLast = (++signalTopLastIndex < signalLineTop->size())? signalTopLast + 1: nullptr;
            }
          }
          signalLength = 0;
        }
        src++;
      }
      cell++;
    }
  }

  for (int j = Height() - 1; j > 0; j--) {
    SignalLine* signalLine = &mSignalRegion2[j];
    SignalLine* signalLineTop = &mSignalRegion2[j-1];
    Signal* signalLast = signalLine && !signalLine->isEmpty()? &signalLine->first(): nullptr;
    Signal* signalTopLast = signalLineTop && !signalLineTop->isEmpty()? &signalLineTop->first(): nullptr;
    int signalLastIndex = 0;
    int signalTopLastIndex = 0;
    while (signalLast && signalTopLast) {
      if (signalTopLast->Right >= signalLast->Left && signalTopLast->Left <= signalLast->Right) {
        if (signalTopLast->Height < signalLast->Height) {
          signalTopLast->Good   = signalLast->Good;
          signalTopLast->Height = signalLast->Height;
        }
      }
      if (signalTopLast->Right <= signalLast->Right) {
        signalTopLast = (++signalTopLastIndex < signalLineTop->size())? signalTopLast + 1: nullptr;
      } else {
        signalLast = (++signalLastIndex < signalLine->size())? signalLast + 1: nullptr;
      }
    }
  }
}

void UinAreaStat::CalcSignalPack()
{
  mPackRegion.clear();
  mPackRegion.resize(Height());

  for (int j = 0; j < Height(); j++) {
    SignalLine* signalLine = &mSignalRegion[j];
    if (signalLine->isEmpty()) {
      continue;
    }
    PackLine* packLine = &mPackRegion[j];

    auto itr = signalLine->begin();
    auto firstSignal = itr;
    auto lastSignal  = itr;
    int count = 0;
    Pack* currentPack = nullptr;
    for (++itr; itr != signalLine->end(); itr++) {
      Signal* signal = itr;
      if (!(signal->Height >= mSignalHeightMin && signal->Height <= mSignalHeightMax)
          || signal->Good < signal->Height*2/3) {
        signal->Good = 0;
        continue;
      }
      if (signal->Left - lastSignal->Right > mPackMaxSpace) {
        firstSignal = itr;
        count = 1;
      } else {
        count++;
      }
      lastSignal = itr;
      if (signal->Right - firstSignal->Left > mPackLength) {
        while (signal->Right - firstSignal->Left > mPackLength) {
          Q_ASSERT(firstSignal != lastSignal);
          ++firstSignal;
        }
      }
      if (count >= kPackMinSignals) {
        if (!currentPack) {
          packLine->append(Pack());
          currentPack = &packLine->last();
          currentPack->Left = firstSignal->Left;
        }
        currentPack->Right = lastSignal->Right;
      } else {
        if (currentPack) {
          currentPack = nullptr;
        }
      }
    }
  }
}

void UinAreaStat::CalcPlate()
{
  mPlateList.clear();
  mWhiteBlackCount.fill(0, kHystFastLength * kHystFastLength);
  for (int jCell = 0; jCell < mCell4Hyst.Height(); jCell++) {
    Cell* cell4 = mCell4Hyst.Line(jCell);
    for (int iCell = 0; iCell < mCell4Hyst.Width(); iCell++) {
      if (cell4->WhiteValue - cell4->BlackValue >= kBlackWhiteDiffMin) {
        for (int j = qMax(0, (cell4->WhiteValue >> kHystFastShift) - kWhiteThreshold); j <= qMin(kHystFastLength - 1, (cell4->WhiteValue >> kHystFastShift) + kWhiteThreshold); j++) {
          int* whiteBlackCount = &mWhiteBlackCount[j * kHystFastLength];
          for (int i = qMax(0, (cell4->BlackValue >> kHystFastShift) - kBlackThreshold); i <= qMin(kHystFastLength - 1, (cell4->BlackValue >> kHystFastShift) + kBlackThreshold); i++) {
            whiteBlackCount[i]++;
          }
        }
      }
      cell4++;
    }
  }

  for (int i = 0; i < mWhiteBlackCount.size(); i++) {
    if (mWhiteBlackCount.at(i) >= kPlateSquareMin) {
      int whiteIndex = i / kHystFastLength;
      int blackIndex = i % kHystFastLength;
      if (((whiteIndex - blackIndex) << kHystFastShift) >= kBlackWhiteDiffMin) {
        CalcPlateOne(whiteIndex, blackIndex);
      }
    }
  }
}

void UinAreaStat::CalcPlateOne(int whiteIndex, int blackIndex)
{
  QVector<QList<Plate::Line> > newLines;
  newLines.resize(mCell4Hyst.Height());
  for (int jCell = 0; jCell < mCell4Hyst.Height(); jCell++) {
    Cell* cell4 = mCell4Hyst.Line(jCell);
    QList<Plate::Line>* jLine = &newLines[jCell];
    Plate::Line* line = nullptr;

    for (int iCell = 0; iCell < mCell4Hyst.Width(); iCell++) {
      int blackIndex_ = cell4->BlackValue >> kHystFastShift;
      int whiteIndex_ = cell4->WhiteValue >> kHystFastShift;
      if (qAbs(whiteIndex - whiteIndex_) <= kWhiteThreshold && qAbs(blackIndex - blackIndex_) <= kBlackThreshold
          && ((whiteIndex_ - blackIndex_) << kHystFastShift) >= kBlackWhiteDiffMin) {
        if (!line) {
          jLine->append(Plate::Line());
          line = &jLine->last();
          line->Left = iCell;
        }
        line->Right = iCell + 1;
      } else {
        line = nullptr;
      }
      cell4++;
    }
  }

  for (int jCell = 0; jCell < newLines.size(); jCell++) {
    QList<Plate::Line>* jLine = &newLines[jCell];
    while (!jLine->isEmpty()) {
      Plate newPlate;
      newPlate.Top = jCell;
      newPlate.Bottom = jCell + 1;
      newPlate.LineList.append(jLine->takeFirst());
      Plate::Line* line = &newPlate.LineList.last();
      int count = line->Right - line->Left;
      for (int jCellDown = jCell + 1; jCellDown < newLines.size(); jCellDown++) {
        QList<Plate::Line>* jLineDown = &newLines[jCellDown];
        for (auto itr = jLineDown->begin(); itr != jLineDown->end(); itr++) {
          Plate::Line* lineDown = &*itr;
          if (lineDown->Left <= line->Right && lineDown->Right >= line->Left) {
            newPlate.LineList.append(*lineDown);
            line = &newPlate.LineList.last();
            count += line->Right - line->Left;
            newPlate.Bottom = jCellDown + 1;
            jLineDown->erase(itr);
            break;
          }
        }
        if (newPlate.Bottom < jCellDown + 1) {
          break;
        }
      }
      if (count >= kPlateSquareMin) {
        newPlate.WhiteIndex = whiteIndex;
        newPlate.BlackIndex = blackIndex;
        newPlate.LineList.prepend(newPlate.LineList.first());
        for (int i = 1; i < newPlate.LineList.size() - 1; i++) {
          newPlate.LineList[i].Left = qMin(newPlate.LineList[i].Left, newPlate.LineList[i + 1].Left);
          newPlate.LineList[i].Right = qMax(newPlate.LineList[i].Right, newPlate.LineList[i + 1].Right);
        }
        mPlateList.append(newPlate);
      }
    }
  }
}

void UinAreaStat::CalcThickness(const QVector<SignalLine>& signalRegion, ByteRegion& thicknessRegion)
{
  thicknessRegion.SetSize(Source().Width(), Source().Height());
  thicknessRegion.ZeroData();

  for (int j = 0; j < Height(); j += Height() - 1) {
    const SignalLine& signalLine = signalRegion.at(j);
    uchar* thick = thicknessRegion.Line(j);
    foreach (const Signal& signal, signalLine) {
      for (int i = signal.Left; i < signal.Right; i++) {
        thick[i] = 1;
      }
    }
  }
  for (int j = 1; j < Height() - 1; j++) {
    const SignalLine& signalLine = signalRegion.at(j);
    const uchar* thickPrev = thicknessRegion.Line(j - 1);
    uchar* thick = thicknessRegion.Line(j);
    foreach (const Signal& signal, signalLine) {
      int half = (signal.Left + signal.Right - 1)/2;
      int length = 1;
      for (int i = signal.Left; i <= half; i++) {
        thick[i] = qMin(qMin((int)thickPrev[i] + 1, length), 255);
        length++;
      }
      length = signal.Right - (half + 1);
      for (int i = half + 1; i < signal.Right; i++) {
        thick[i] = qMin(qMin((int)thickPrev[i] + 1, length), 255);
        length--;
      }
    }
  }
  for (int j = Height() - 2; j >= 1; j--) {
    const SignalLine& signalLine = signalRegion.at(j);
    const uchar* thickPrev = thicknessRegion.Line(j + 1);
    uchar* thick = thicknessRegion.Line(j);
    foreach (const Signal& signal, signalLine) {
      for (int i = signal.Left; i < signal.Right; i++) {
        thick[i] = qMin(qMin((int)thickPrev[i] + 1, (int)thick[i]), 255);
      }
    }
  }

//  for (int j = 1; j < Height(); j++) {
//    const SignalLine& signalLine = mSignalRegion2.at(j);
//    const uchar* thickPrev = mThicknessRegion2.Line(j - 1);
//    uchar* thick = mThicknessRegion2.Line(j);
//    foreach (const Signal& signal, signalLine) {
//      int maxLength = 1;
//      for (int i = signal.Left; i < signal.Right; i++) {
//        maxLength = qMax(maxLength, qMax((int)thick[i], (int)thickPrev[i]));
//      }
//      for (int i = signal.Left; i < signal.Right; i++) {
//        thick[i] = maxLength;
//      }
//    }
//  }
//  for (int j = Height() - 2; j >= 0; j--) {
//    const SignalLine& signalLine = mSignalRegion2.at(j);
//    const uchar* thickPrev = mThicknessRegion2.Line(j + 1);
//    uchar* thick = mThicknessRegion2.Line(j);
//    foreach (const Signal& signal, signalLine) {
//      int maxLength = 1;
//      for (int i = signal.Left; i < signal.Right; i++) {
//        maxLength = qMax(maxLength, qMax((int)thick[i], (int)thickPrev[i]));
//      }
//      for (int i = signal.Left; i < signal.Right; i++) {
//        thick[i] = maxLength;
//      }
//    }
  //  }
}

void UinAreaStat::CalcThicknessEdge(const ByteRegion& thicknessRegion)
{
  int minThickness = (mUinMetrics.CharThick() - 1)/2;
  int maxThickness = (mUinMetrics.DigitThick() + 1)/2 + 2;

  mThicknessEdge.SetSize(thicknessRegion.Width(), thicknessRegion.Height());
  mThicknessEdge.ZeroData();
  for (int j = 1; j < thicknessRegion.Height() - 1; j++) {
    const uchar* thickn = thicknessRegion.Data(1, j);
    const uchar* thickm = thicknessRegion.Data(1, j - 1);
    const uchar* thickp = thicknessRegion.Data(1, j + 1);
    uchar* markn = mThicknessEdge.Data(1, j);
    uchar* markm = mThicknessEdge.Data(1, j - 1);
    uchar* markp = mThicknessEdge.Data(1, j + 1);
    for (int i = 1; i < thicknessRegion.Width() - 1; i++) {
      if (*thickn && *thickn >= minThickness && *thickn <= maxThickness &&
          *thickn >= thickn[1] && *thickn >= thickn[-1] &&
          *thickn >= thickm[0] && *thickn >= thickm[1] && *thickn >= thickm[-1] &&
          *thickn >= thickp[0] && *thickn >= thickp[1] && *thickn >= thickp[-1]) {
        markn[0] = markn[1] = markn[-1] = 1;
        markm[0] = markm[1] = markm[-1] = 1;
        markp[0] = markp[1] = markp[-1] = 1;
      }

      thickn++;
      thickm++;
      thickp++;
      markn++;
      markm++;
      markp++;
    }
  }

  if (!mObjConnect) {
    mObjConnect.reset(new ObjConnect());
  }
  mObjConnect->ConnectAny(mThicknessEdge);

  int maxWidth = mUinMetrics.DigitWidth() * 4 / 3;
  int minHeight = mUinMetrics.PrefixHeight() * 2 / 4 - 1;
  int maxHeight = mUinMetrics.DigitHeight() * 4 / 3;
  const QVector<QRect>& objList = mObjConnect->GetObjList();
  mObjList.reserve(objList.size());
  mObjRegion.SetSize(mCellHyst.Width(), mCellHyst.Height());
  foreach (const QRect& obj, objList) {
    if (obj.width() <= maxWidth && obj.height() >= minHeight && obj.height() <= maxHeight) {
      mObjList.append(obj);
      int x = (obj.left() + obj.right()) / (2*mCellWidth);
      int y = (obj.top() + obj.bottom()) / (2*mCellHeight);
      mObjRegion.Data(x, y)->append(&mObjList.last());
//      qDebug() << "symbol" << ((obj.left() + obj.right()) / 2) << ((obj.top() + obj.bottom()) / 2);
    }
  }
}

void UinAreaStat::CalcObjPack()
{
  int widthCell = (mUinMetrics.Width() / mCellWidth + 1)/2;
  int heightCell = (mUinMetrics.Width() / (2*mCellHeight) + 1)/2;
  for (int j = 0; j < mObjRegion.Height(); j++) {
    const ObjList* objList = mObjRegion.Line(j);
    for (int i = 0; i < mObjRegion.Width(); i++) {
      foreach (const QRect* obj, *objList) {
//        qDebug() << "selected" << (*obj);
        // obj near
        int l = qMax(0, i - widthCell);
        int r = qMin(mObjRegion.Width()-1, i + widthCell);
        int t = qMax(0, j - heightCell);
        int b = qMin(mObjRegion.Height()-1, j + heightCell);
        QRect seekRect(l, t, r - l + 1, b - t + 1);
        Region<ObjList> connectRegion;
        l = qMax(0, i - 2*widthCell);
        r = qMin(mObjRegion.Width()-1, i + 2*widthCell);
        t = qMax(0, j - 2*heightCell);
        b = qMin(mObjRegion.Height()-1, j + 2*heightCell);
        connectRegion.SetSource(mObjRegion, QRect(l, t, r - l + 1, b - t + 1));
        for (int j = 0; j < connectRegion.Height(); j++) {
          const ObjList* objList2 = connectRegion.Line(j);
          for (int i = 0; i < connectRegion.Width(); i++) {
            foreach (const QRect* obj2, *objList2) {
              TryPlate((obj->bottomLeft() + obj->bottomRight())/2, (obj2->bottomLeft() + obj2->bottomRight())/2, seekRect);
            }
            objList2++;
          }
        }
        //
      }
      objList++;
    }
  }
}

void UinAreaStat::CalcObjPlates()
{
  mObjPlateList.clear();
  mUinRegionStore.clear();
  for (auto itr = mObjPackList.begin(); itr != mObjPackList.end(); itr++) {
    mObjPack = itr;
    mObjPlateList.append(ObjPlate());
    mObjPlate = &mObjPlateList.last();
    mObjPlateRegion = &mObjPlate->SourceRegion;

    if (CalcObjPlate()) {
      CalcObjPlateUin();
    } else {
      mObjPlateList.removeLast();
    }
  }
}

bool UinAreaStat::CalcObjPlate()
{
  return CalcObjPlateBottom()
      && CalcObjPlateConstruct()
      && CalcObjPlateNormal()
      && CalcObjPlateTop()
      && CalcObjPlateSpace();
}

bool UinAreaStat::CalcObjPlateBottom()
{
  int minValueLeft  = Width() * 255;
  int minJ          = 0;
  QPoint l = mObjPack->Left;
  QPoint r = mObjPack->Right;
  QPoint m = (l + r)/2;
  for (int j = 0; j < mObjPack->Height; j++) {
    int  left = Source().SumLine(QPoint(l.x(), qMin(Height() - 1, l.y() + j)), QPoint(m.x(), qMin(Height() - 1, m.y() + j)));
    if (left < minValueLeft) {
      minValueLeft = left;
      minJ         = j;
    }
  }
  QPoint minLeft(mObjPack->Left.x(), qMin(mObjPack->Left.y() + minJ, Height() - 1));
  QPoint minRight(mObjPack->Right.x(), qMin(mObjPack->Right.y() + minJ, Height() - 1));
  mObjPack->BottomLeft  = mObjPack->Left;
  mObjPack->BottomRight = mObjPack->Right;
  int    maxValue = 0;
  for (int j = minJ; j >= 0; j--) {
    int v1 = Source().SumLine(QPoint(l.x(), qMin(l.y() + j, Height() - 1)), QPoint(r.x(), qMin(r.y() + j, Height() - 1)));
    if (v1 > maxValue) {
      maxValue = v1;
      mObjPack->BottomLeft  = QPoint(l.x(), l.y() + j);
      mObjPack->BottomRight = QPoint(r.x(), r.y() + j);
    }
  }

  l = mObjPack->BottomLeft;
  r = mObjPack->BottomRight;
  for (int jl = qMax(0, l.y() - mObjPack->Height/2); jl <= qMin(minLeft.y(), l.y() + mObjPack->Height/2); jl++) {
    for (int jr = qMax(0, r.y() - mObjPack->Height/2); jr <= qMin(minRight.y(), r.y() + mObjPack->Height/2); jr++) {
      int v1 = Source().SumLine(QPoint(l.x(), jl), QPoint(r.x(), jr));
      if (v1 > maxValue) {
        maxValue = v1;
        mObjPack->BottomLeft  = QPoint(l.x(), jl);
        mObjPack->BottomRight = QPoint(r.x(), jr);
      }
    }
  }
  return true;
}

bool UinAreaStat::CalcObjPlateConstruct()
{
  int k2 = mObjPack->BottomRight.y() - mObjPack->BottomLeft.y();
  int k1 = mObjPack->BottomRight.x() - mObjPack->BottomLeft.x();
  int h  = 2*mObjPack->Height;
  int w  = UinMetrics::WidthForHeight(h) * 3/2;
  if (h < kPlateHeightMin) {
    return false;
  }
  mObjPlateRegion->SetSize(w, h);
  QPoint plateLeft = (mObjPack->BottomLeft + mObjPack->BottomRight)/2 + QPoint(-w/2, -w * k2/k1/2);
  for (int j = 0; j < mObjPlateRegion->Height(); j++) {
    uchar* dst = mObjPlateRegion->Line(mObjPlateRegion->Height() - 1 - j);
    int jl = plateLeft.y() - j;
    int il = plateLeft.x() + j * k2/k1;
    for (int i = 0; i < mObjPlateRegion->Width(); i++) {
      int ic = il + i;
      int jc = jl + i * k2/k1;
      *dst = (ic >= 0 && jc >= 0 && ic < Width() && jc < Height())? *Source().Data(ic, jc): 255;
      dst++;
    }
  }

  for (int j = mObjPack->Height; j < mObjPlateRegion->Height(); j++) {
    int l = qMax(mObjPlateRegion->Width()/2 - k1/2, 0);
    int r = qMin(mObjPlateRegion->Width()/2 + k1/2, mObjPlateRegion->Width() - 1);
    const uchar* src = mObjPlateRegion->Data(l, j);
    for (int i = l; i <= r; i++) {
      mObjPlate->PlateHyst.Inc(*src);
      src++;
    }
  }

  return true;
}

bool UinAreaStat::CalcObjPlateTop()
{
  int k1 = mObjPack->BottomRight.x() - mObjPack->BottomLeft.x();
  int l = qMax(mObjPlateRegion->Width()/2 - k1/2, 0);
  int r = qMin(mObjPlateRegion->Width()/2 + k1/2, mObjPlateRegion->Width() - 1);
  int w = r - l + 1;
  int minValue = 255*Width();
  int maxValue = 0;
  for (int j = mObjPlateRegion->Height() - mUinMetrics.DigitThick() - 1; j > mObjPack->Height; j--) {
    int v = 0;
    const uchar* src = mObjPlateRegion->Data(l, j);
    for (int i = l; i <= r; i++) {
      if (*src == 0) {
        v++;
      }
      src++;
    }
    minValue = qMin(minValue, v);
    maxValue = qMax(maxValue, v);
  }

  int digitThick = UinMetrics(UinMetrics::WidthForHeight(mObjPack->Height)).DigitThick();
  int blackThreshold = w*2/3;
  int whiteThreshold = digitThick;

  int topBorder = 0;
  for (int j = mObjPack->Height; j >= 0; j--) {
    int v = 0;
    const uchar* src = mObjPlateRegion->Data(l, j);
    for (int i = l; i <= r; i++) {
      if (*src == 0) {
        v++;
      }
      src++;
    }
    if (v < whiteThreshold) {
      topBorder = qMax(j - digitThick, topBorder);
    } else if (v >= blackThreshold) {
      topBorder = qMax(j + 1, topBorder);
      break;
    }
  }

  int bottomBorder = mObjPlateRegion->Height() - 1;
  for (int j = mObjPlateRegion->Height() - 2*mUinMetrics.DigitThick(); j < mObjPlateRegion->Height(); j++) {
    int v = 0;
    const uchar* src = mObjPlateRegion->Data(l, j);
    for (int i = l; i <= r; i++) {
      if (*src == 0) {
        v++;
      }
      src++;
    }
    if (v < whiteThreshold) {
      bottomBorder = j;
      break;
    } else if (v >= blackThreshold) {
      bottomBorder = j - 1;
      break;
    }
  }

  int height = bottomBorder - topBorder + 1;
  if (height < kPlateHeightMin) {
    return false;
  }
  mObjPlate->NormalRegion.SetSource(mObjPlate->SourceNormalRegion, 0, topBorder, mObjPlate->SourceNormalRegion.Width(), height);
  mObjPlateRegion = &mObjPlate->NormalRegion;

  mObjPlate->VertHyst.fill(0, mObjPlateRegion->Width());
  for (int j = 0; j < mObjPlateRegion->Height(); j++) {
    const uchar* src = mObjPlateRegion->Line(j);
    int*        hyst = mObjPlate->VertHyst.data();

    for (int i = 0; i < mObjPlateRegion->Width(); i++) {
      if (*src <= 128) {
        if (*src == 0) {
          *hyst += 4;
        } else {
          *hyst += 1;
        }
      }
      src++;
      hyst++;
    }
  }

  return true;
}

bool UinAreaStat::CalcObjPlateNormal()
{
  mObjPlate->SourceNormalRegion.SetSize(mObjPlate->SourceRegion.Width(), mObjPlate->SourceRegion.Height());
  int low = mObjPlate->PlateHyst.GetValue(50);
  int hi  = mObjPlate->PlateHyst.GetValue(950);
  int l1 = (2*low + hi)/3;
  int l2 = (low + 2*hi)/3;
  for (int j = 0; j < mObjPlateRegion->Height(); j++) {
    const uchar* src = mObjPlate->SourceRegion.Line(j);
    uchar*       dst = mObjPlate->SourceNormalRegion.Line(j);

    for (int i = 0; i < mObjPlateRegion->Width(); i++) {
      if (*src <= l1) {
        *dst = 0;
      } else if (*src >= l2) {
        *dst = 255;
      } else {
        *dst = 127;
      }
      src++;
      dst++;
    }
  }
  mObjPlateRegion = &mObjPlate->SourceNormalRegion;
  return true;
}

bool UinAreaStat::CalcObjPlateSpace()
{
  mObjPlate->NormalMetrics.SetBaseHeight(mObjPlateRegion->Height());
  int diffTreshold = 4;
  int rightHyst    = qMin(mObjPlate->NormalMetrics.DigitWidth()*5/4, mObjPlate->VertHyst.size());
  int midHyst      = rightHyst/2;
  auto hystFrom    = mObjPlate->VertHyst.begin();
  auto hystTo      = mObjPlate->VertHyst.begin() + rightHyst;
  for (int i = 0; i < mObjPlate->VertHyst.size(); i++) {
    int minValue = *std::min_element(hystFrom, hystTo);
    if (mObjPlate->VertHyst.at(i) <= minValue + diffTreshold) {
      mObjPlate->SpaceList.append(i);
    }
    if (i >= midHyst && hystTo != mObjPlate->VertHyst.end()) {
      midHyst++;
      hystFrom++;
      hystTo++;
    }
  }
  return true;
}

void UinAreaStat::CalcObjPlateUin()
{
  CalcObjPlateUinFindPossible();
  if (mObjPlate->PossibleList.size() < kPlateRectPossibleMin) {
    return;
  }

  mAnalyser->UinInit();
  mAnalyser->TakeUin(mObjPlate->UinTest);
  mUin = mObjPlate->UinTest;

  mUin->BeginTest();
  mPlateUinMap.clear();
  CalcObjPlateUinTestPossible();
  for (auto itr = mPlateUinMap.begin(); itr != mPlateUinMap.end(); itr++) {
    mCurrentUinShift = itr.key();
    mCurrentUinList  = &itr.value();
    CalcObjPlateUinConstruct();
  }
}

void UinAreaStat::CalcObjPlateUinFindPossible()
{
  mObjPlate->PossibleList.clear();

  for (auto itrLeft = mObjPlate->SpaceList.begin(); itrLeft != mObjPlate->SpaceList.end(); itrLeft++) {
    int left = *itrLeft;
    auto itrRight = itrLeft;
    itrRight++;
    int right = *itrRight;
    if (right == left + 1) {
      continue;
    }
    int rightMin = left + mObjPlate->NormalMetrics.DigitWidth()*2/3;
    int rightMax = left + mObjPlate->NormalMetrics.DigitWidth()*5/3;
    for (; itrRight != mObjPlate->SpaceList.end(); itrRight++) {
      if (*itrRight == right + 1) {
        right++;
        continue;
      }
      right = *itrRight;
      if (right > rightMax) {
        break;
      } else if (right < rightMin) {
        continue;
      }

      QRect rect(left + 1, 0, right - left - 1, mObjPlateRegion->Height());
      mObjPlate->PossibleList.append(rect);
    }
  }

  if (mObjPlate->PossibleList.size() > kPlateRectPossibleMax) {
    int midPlace = mObjPlateRegion->Width()/2;
    int  left = 0;
    int right = mObjPlate->PossibleList.size() - 1;
    const QRect*  leftRect = &mObjPlate->PossibleList.at(left);
    const QRect* rightRect = &mObjPlate->PossibleList.at(right);
    int  leftMid = midPlace - leftRect->right();
    int rightMid = rightRect->left() - midPlace;
    while (right - left >= kPlateRectPossibleMax) {
      if (leftMid < rightMid) {
        right--;
        rightRect = &mObjPlate->PossibleList.at(right);
        rightMid = rightRect->left() - midPlace;
      } else {
        left++;
        leftRect = &mObjPlate->PossibleList.at(left);
        leftMid = midPlace - leftRect->right();
      }
    }
    mObjPlate->PossibleList = mObjPlate->PossibleList.mid(left, right - left + 1);
  }
}

void UinAreaStat::CalcObjPlateUinTestPossible()
{
  const int disp = mObjPlate->NormalMetrics.CharThick();
  for (const QRect& rect: mObjPlate->PossibleList) {
    QRect trimmedRect(rect);
    CalcObjPlateUinTestPossibleOne(rect, &trimmedRect);
    if (trimmedRect.top() < disp) {
      trimmedRect.setTop(disp);
      CalcObjPlateUinTestPossibleOne(trimmedRect);
    }
  }
}

void UinAreaStat::CalcObjPlateUinTestPossibleOne(const QRect& rect, QRect* trimmedRect)
{
  QRect bestRect(rect);
  mUin->BeginChar(ByteRegion(*mObjPlateRegion, rect));
  mUin->TrimChar(&bestRect);
  mUin->TestChar();
  qreal normalMatch = mUin->GetBestMatch();
  qreal bestMatch = normalMatch;
  int moveStep = qMax(2, mObjPlate->NormalMetrics.DigitThick()/2);
  qreal lastMatch = normalMatch;
  for (int step = moveStep; step < mObjPlate->NormalMetrics.DigitWidth()/2; step += moveStep) {
    mUinRegionStore.append(ByteRegion(rect.size()));
    ByteRegion& nextRegion = mUinRegionStore.last();
    for (int j = 0; j < nextRegion.Height(); j++) {
      int l = step - 2*step*j/(nextRegion.Height() - 1);
      l = qBound(0, rect.left() + l, mObjPlateRegion->Width() - nextRegion.Width() - 1);
      const uchar* src = mObjPlateRegion->Data(l, rect.top() + j);
      uchar* dst = nextRegion.Line(j);
      memcpy(dst, src, nextRegion.Width());
    }

    mUin->BeginChar(nextRegion);
    QRect trimmedRectLocal(rect);
    mUin->TrimChar(&trimmedRectLocal);
    mUin->TestChar();
    qreal nextMatch = mUin->GetBestMatch();
    if (nextMatch <= lastMatch) {
      break;
    }
    lastMatch = nextMatch;
    if (nextMatch > bestMatch) {
      bestMatch = nextMatch;
      bestRect = trimmedRectLocal;
    }
  }
  for (int step = moveStep; step < mObjPlate->NormalMetrics.DigitWidth()/2; step += moveStep) {
    mUinRegionStore.append(ByteRegion(rect.size()));
    ByteRegion& nextRegion = mUinRegionStore.last();
    for (int j = 0; j < nextRegion.Height(); j++) {
      int l = -step + 2*step*j/(nextRegion.Height() - 1);
      l = qBound(0, rect.left() + l, mObjPlateRegion->Width() - nextRegion.Width() - 1);
      const uchar* src = mObjPlateRegion->Data(l, rect.top() + j);
      uchar* dst = nextRegion.Line(j);
      memcpy(dst, src, nextRegion.Width());
    }

    mUin->BeginChar(nextRegion);
    QRect trimmedRectLocal(rect);
    mUin->TrimChar(&trimmedRectLocal);
    mUin->TestChar();
    qreal nextMatch = mUin->GetBestMatch();
    if (nextMatch <= lastMatch) {
      break;
    }
    lastMatch = nextMatch;
    if (nextMatch > bestMatch) {
      bestMatch = nextMatch;
      bestRect = trimmedRectLocal;
    }
  }
  if (bestMatch > kPlateEasyDigitMatchMin) {
    mObjPlate->UinList.append(bestRect);
    if (trimmedRect) {
      *trimmedRect = bestRect;
    }
  }
}

void UinAreaStat::CalcObjPlateUinEasy()
{
  const int disp = mObjPlate->NormalMetrics.CharThick();
  for (auto itrLeft = mObjPlate->SpaceList.begin(); itrLeft != mObjPlate->SpaceList.end(); itrLeft++) {
    int left = *itrLeft;
    auto itrRight = itrLeft;
    itrRight++;
    int right = *itrRight;
    if (right == left + 1) {
      continue;
    }
    int rightMin = left + mObjPlate->NormalMetrics.DigitWidth()*2/3;
    int rightMax = left + mObjPlate->NormalMetrics.DigitWidth()*5/3;
    for (; itrRight != mObjPlate->SpaceList.end(); itrRight++) {
      if (*itrRight == right + 1) {
        right++;
        continue;
      }
      right = *itrRight;
      if (right > rightMax) {
        break;
      } else if (right < rightMin) {
        continue;
      }

      QRect rect(left + 1, 0, right - left - 1, mObjPlateRegion->Height());
      mUin->BeginChar(ByteRegion(*mObjPlateRegion, rect));
      mUin->TrimChar(&rect);
//      mObjPlate->UinList.append(rect);
      mUin->TestChar();
      if (mUin->GetBestMatch() > 1.0) {
        mObjPlate->UinList.append(rect);
        mObjPlate->UinText.append(mUin->GetBestChar());
        qDebug() << mObjPlate->UinList.size() << "Uin match" << mUin->GetBestMatch() << rect << mUin->GetBestChar();
//        mObjPlate->UinList.append(rect);
//        qDebug() << "Uin match" << mUin->GetBestMatch() << mUin->GetBestChar();
//        return;
      }

      QRect rect2(left + 1, disp, right - left - 1, mObjPlateRegion->Height() - disp);
      mUin->BeginChar(ByteRegion(*mObjPlateRegion, rect2));
      mUin->TrimChar(&rect2);
      if (rect2 == rect) {
        continue;
      }
//      mObjPlate->UinList.append(rect);
      mUin->TestChar();
//      if (mUin->GetBestMatch() > 1.0) {
        mObjPlate->UinList.append(rect2);
        mObjPlate->UinText.append(mUin->GetBestChar());
        qDebug() << mObjPlate->UinList.size() << "Uin match" << mUin->GetBestMatch() << rect2 << mUin->GetBestChar();
//        mObjPlate->UinList.append(rect);
//        qDebug() << "Uin match" << mUin->GetBestMatch() << mUin->GetBestChar();
//        return;
//      }
    }
  }
}

void UinAreaStat::CalcObjPlateUinBase()
{
  qreal bestMatch = 0;
  QRect bestRect;
  QChar bestChar;
  mUin->BeginTest();
  int leftBorder  = mObjPlateRegion->Width()/2 - 3*mObjPlate->NormalMetrics.DigitWidth();
  int rightBorder = mObjPlateRegion->Width()/2 + 2*mObjPlate->NormalMetrics.DigitWidth();
  auto leftItr = std::lower_bound(mObjPlate->SpaceList.begin(), mObjPlate->SpaceList.end(), leftBorder);
  for (auto itr = leftItr; itr != mObjPlate->SpaceList.end() && *itr < rightBorder; itr++) {
    int left = *itr;
    int rightMin = left + mObjPlate->NormalMetrics.DigitWidth()*2/3;
    int rightMax = left + mObjPlate->NormalMetrics.DigitWidth()*4/3;
    auto rightItr = std::lower_bound(itr, mObjPlate->SpaceList.end(), rightMin);
    for (auto itr = rightItr; itr != mObjPlate->SpaceList.end() && *itr < rightMax; itr++) {
      int right = *itr;
      QRect rect(left + 1, 0, right - left, mObjPlateRegion->Height());
      mUin->BeginChar(ByteRegion(*mObjPlateRegion, rect));
      mUin->TrimChar(&rect);
//      mObjPlate->UinList.append(rect);
      mUin->TestChar();
      if (mUin->GetBestMatch() > 1.6) {
        bestMatch = mUin->GetBestMatch();
        bestRect  = rect;
        bestChar  = mUin->GetBestChar();
        mObjPlate->UinList.append(bestRect);
        mObjPlate->UinText.append(bestChar);
        qDebug() << mObjPlate->UinList.size() << "Uin match" << mUin->GetBestMatch() << mUin->GetBestChar();
//        mObjPlate->UinList.append(rect);
//        qDebug() << "Uin match" << mUin->GetBestMatch() << mUin->GetBestChar();
//        return;
      }

      rect = QRect(left + 1, mObjPlate->NormalMetrics.DigitThick(), right - left, mObjPlateRegion->Height() - mObjPlate->NormalMetrics.DigitThick());
      mUin->BeginChar(ByteRegion(*mObjPlateRegion, rect));
      mUin->TrimChar(&rect);
//      mObjPlate->UinList.append(rect);
      mUin->TestChar();
      if (mUin->GetBestMatch() > 1.5) {
        bestMatch = mUin->GetBestMatch();
        bestRect  = rect;
        bestChar  = mUin->GetBestChar();
        mObjPlate->UinList.append(bestRect);
        mObjPlate->UinText.append(bestChar);
        qDebug() << mObjPlate->UinList.size() << "Uin match" << mUin->GetBestMatch() << mUin->GetBestChar();
//        mObjPlate->UinList.append(rect);
//        qDebug() << "Uin match" << mUin->GetBestMatch() << mUin->GetBestChar();
//        return;
      }

      rect = QRect(left + 1, 0, right - left, mObjPlateRegion->Height() - mObjPlate->NormalMetrics.DigitThick());
      mUin->BeginChar(ByteRegion(*mObjPlateRegion, rect));
      mUin->TrimChar(&rect);
//      mObjPlate->UinList.append(rect);
      mUin->TestChar();
      if (mUin->GetBestMatch() > 1.5) {
        bestMatch = mUin->GetBestMatch();
        bestRect  = rect;
        bestChar  = mUin->GetBestChar();
        mObjPlate->UinList.append(bestRect);
        mObjPlate->UinText.append(bestChar);
        qDebug() << mObjPlate->UinList.size() << "Uin match" << mUin->GetBestMatch() << mUin->GetBestChar();
//        mObjPlate->UinList.append(rect);
//        qDebug() << "Uin match" << mUin->GetBestMatch() << mUin->GetBestChar();
//        return;
      }

    }
  }
  if (bestMatch > 1.5) {
    qDebug() << "Uin best match" << bestMatch << bestChar;
    mObjPlate->UinList.append(bestRect);
  }
}

void UinAreaStat::CalcObjPlateUinConstruct()
{
  if (mCurrentUinList->size() < kPlateConstructDigitsMin) {
    return;
  }

  for (const QRect& rect: *mCurrentUinList) {
    CalcObjPlateUinConstructTest(UinMetrics::WidthForCharHeight(rect.height()));
    CalcObjPlateUinConstructTest(UinMetrics::WidthForDigitHeight(rect.height()));
  }
}

void UinAreaStat::CalcObjPlateUinConstructTest(int height)
{
  UinMetrics testMetrics(height);

  int xThreshold = testMetrics.DigitWidth()/2;
  int widthThreshold = testMetrics.DigitWidth()/4;
  int heightThreshold = 2*testMetrics.DigitThick();

  for (const QRect& currentUnitRect: *mCurrentUinList) {
    for (int startCharIndex = 0; startCharIndex <= kPlateTemplateSize - kPlateConstructDigitsMin; startCharIndex++) {
      UinList foundChars;
      int currentX = currentUnitRect.x();
      int currentRectIndex = 0;
      for (int currentCharIndex = startCharIndex; currentCharIndex < kPlateTemplateSize; currentCharIndex++) {
        char currentChar = kPlateTemplate[currentCharIndex];
        int currentCharWidth = currentChar == 'c'? testMetrics.CharWidth(): testMetrics.DigitWidth();
        int currentCharHeight = currentChar == 'c'? testMetrics.CharHeight(): testMetrics.DigitHeight();
        const QRect* rect = &mCurrentUinList->at(currentRectIndex);
        for (; currentRectIndex < mCurrentUinList->size(); currentRectIndex++) {
          rect = &mCurrentUinList->at(currentRectIndex);
          if (rect->x() > currentX - xThreshold * (1 + currentCharIndex - startCharIndex)) {
            break;
          }
        }
        if (currentRectIndex >= mCurrentUinList->size()) {
          break;
        }

        if (qAbs(rect->x() - currentX) < xThreshold * (1 + currentCharIndex - startCharIndex)
            && qAbs(rect->width() - currentCharWidth) < widthThreshold
            && qAbs(rect->height() - currentCharHeight) < heightThreshold) {
          currentX = rect->left() + currentCharWidth;
          foundChars.append(*rect);
        } else {
          currentX += currentCharWidth;
        }
      }

      if (foundChars.size() >= kPlateConstructDigitsMin) {
        qDebug() << "foundChars" << foundChars.size();
      }
    }
  }
}

bool ObjRefXLess(const QRect* obj, const QRect* obj2)
{
  return obj->x() < obj2->x();
}

void UinAreaStat::TryPlate(const QPoint& p1, const QPoint& p2, const QRect& seekRect)
{
//  qDebug() << "try" << (*obj) << (*obj2);
  if (qAbs(p1.x() - p2.x()) < 2) {
    return;
  }
  qreal k = (qreal)(p1.y() - p2.y()) / (p1.x() - p2.x());
  if (k < -0.5 || k > 0.5) {
    return;
  }
  qreal c = p1.y() - k*p1.x();

  Region<ObjList> seekRegion;
  seekRegion.SetSource(mObjRegion, seekRect);

  int yDelta = (int)(mUinMetrics.DigitHeight()/4/* + qAbs(k)*mUinMetrics.DigitWidth()*/ + 1);
  ObjList objLine;
  for (int j = 0; j < seekRegion.Height(); j++) {
    const ObjList* objList3 = seekRegion.Line(j);
    for (int i = 0; i < seekRegion.Width(); i++) {
      foreach (const QRect* obj3, *objList3) {
        int x3 = (obj3->left() + obj3->right())/2;
        int y3 = obj3->bottom();
        int yi = k*x3 + c;
        if (qAbs(yi - y3) < yDelta) {
          objLine.append(obj3);
//          qDebug() << "connect" << count << (*obj3);
        }
      }
      objList3++;
    }
  }

  if (objLine.size() >= kPlateObjectMin) {
    int lossDelta = mUinMetrics.DigitWidth() * kPlateObjectLossMax * 4/3;
    int lossDelta2 = lossDelta*lossDelta;

    std::sort(objLine.begin(), objLine.end(), ObjRefXLess);
//    int x1 = objLine.first()->x();
//    int y1 = k*x1 + c;
//    int x2 = objLine.last()->x();
//    int y2 = k*x2 + c;
//    QPoint left(x1, y1);
//    QPoint right(x2, y2);
    for (int i = 0; i < objLine.size() - 1; i++) {
      const QRect* o1 = objLine.at(i);
      const QRect* o2 = objLine.at(i + 1);
      int delta2 = ((o1->x() - o2->x())*(o1->x() - o2->x()) + (o1->y() - o2->y())*(o1->y() - o2->y()));
      if (delta2 > lossDelta2) {
        if (i + 1 >= kPlateObjectMin) {
          ObjList leftList = objLine.mid(0, i + 1);
          AddPlateLine(leftList, seekRect);
        }
        objLine = objLine.mid(i + 1);
        i = -1;
      }
    }
    if (objLine.size() >= kPlateObjectMin) {
      AddPlateLine(objLine, seekRect);
    }
  }
}

bool UinAreaStat::AddPlateLine(UinAreaStat::ObjList& line, const QRect& rect)
{
  // filter height
  QVector<int> heightList;
  heightList.reserve(line.size());
  foreach (const QRect* obj, line) {
    heightList.append(obj->height());
  }
  std::sort(heightList.begin(), heightList.end());
  int normalHeight = heightList.at(heightList.size()/2);
  if (heightList.first() < normalHeight*3/5 || heightList.last() > normalHeight*7/5) {
    for (int i = 0; i < line.size(); i++) {
      const QRect* obj = line.at(i);
      if (obj->height() < normalHeight*3/5 || obj->height() > normalHeight*7/5) {
        line.removeAt(i);
        i--;
      }
    }
    if (line.size() < kPlateObjectMin) {
      return false;
    }
  }

  ObjPack* pack = nullptr;
  for (int i = 0; i < mObjLineList.size(); i++) {
    const ObjList& line1 = mObjLineList.at(i);
    if (line1.contains(line.first()) && line1.contains(line.last())) {
      return false;
    } else if (line.contains(line1.first()) && line.contains(line1.last())) {
      if (!pack) {
        mObjLineList.replace(i, line);
        pack = &mObjPackList[i];
      } else {
        qSwap(mObjLineList[i], mObjLineList[mObjLineList.size() - 1]);
        mObjLineList.removeLast();
        qSwap(mObjPackList[i], mObjPackList[mObjPackList.size() - 1]);
        mObjPackList.removeLast();
        i--;
      }
    }
  }
  if (!pack) {
    mObjLineList.append(line);
    mObjPackList.append(ObjPack());
    pack = &mObjPackList.last();
  }

  pack->Area   = QRect(QPoint(qBound(0, (rect.left() - rect.width()/2) * mCellWidth, Width()-1), qBound(0, (rect.top() - rect.height()/2) * mCellHeight, Height()-1))
                       , QPoint(qBound(0, (rect.right() + rect.width()/2) * mCellWidth, Width()-1), qBound(0, (rect.bottom() + rect.height()/2) * mCellHeight, Height()-1)));
  pack->Left   = line.at(1)->bottomLeft();
  pack->Right  = line.at(line.size() - 2)->bottomRight();
  pack->Height = normalHeight;
  return true;
}

void UinAreaStat::AnalyzePlate()
{
  for (auto itr = mPlateList.begin(); itr != mPlateList.end(); ) {
    Plate* plate = &*itr;
    if (FilterPlateOne(plate)) {
      itr++;
    } else {
      itr = mPlateList.erase(itr);
    }
  }
}

bool UinAreaStat::FilterPlateOne(UinAreaStat::Plate* plate)
{
  Q_UNUSED(plate);
//  int whiteValue = plate->WhiteIndex << kHystFastShift;
//  int blackValue = plate->BlackIndex << kHystFastShift;
//  int blackThreshold = (2*blackValue + whiteValue)/3;
//  int whiteThreshold = (blackValue + 2*whiteValue)/3;
//  int count = 0;
//  for (int jCell = plate->Top; jCell <= plate->Bottom; jCell++) {
//    Plate::Line* line = &plate->LineList[jCell - plate->Top];
//    Cell* cell = mCellHyst.Data(line->Left, jCell);
//    for (int iCell = line->Left; iCell <= line->Right; iCell++) {
//      int whiteCount = cell->CellHyst.GreaterCount(whiteThreshold);
//      int blackCount = cell->CellHyst.LessCount(blackThreshold);
//      if (whiteCount > cell->CellHyst.TotalCount()/4 && blackCount > cell->CellHyst.TotalCount()/8) {
//        count++;
//      }
//      cell++;
//    }
//  }
//  if (count < kPlateSquareMin) {
//    return false;
//  }
  return true;
}

void UinAreaStat::PrepareDump(ByteRegion* debug)
{
  if (debug->Width() != Width() || debug->Height() != Height()) {
    debug->SetSize(Width(), Height());
  }
}

void UinAreaStat::PrepareDumpWhite(ByteRegion* debug)
{
  PrepareDump(debug);
  debug->FillData(255);
}

void UinAreaStat::PrepareDumpBlack(ByteRegion* debug)
{
  PrepareDump(debug);
  debug->FillData(0);
}

UinAreaStat::UinAreaStat(AnalyserOld* _Analyser)
  : mAnalyser(_Analyser), mCellWidth(0), mCellHeight(0), mSignalLengthMax(0), mSignalLengthGood(0)
{
}

UinAreaStat::~UinAreaStat()
{
}
