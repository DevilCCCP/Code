#include "CellStatFtr.h"


enum SecondaryArtifact {
  eCell2,
  eCell4
};

void CellStatFtr::RegionInfoBlockWgb(FilterInfo* filterInfo)
{
  filterInfo->Name          = "Block wgb";

  RegionInfoBlockParams(filterInfo);
}

void CellStatFtr::RegionInfoBlockWb11(FilterInfo* filterInfo)
{
  filterInfo->Name          = "Block wb 1/1";

  RegionInfoBlockParams(filterInfo);
}

void CellStatFtr::RegionInfoBlockWb21(FilterInfo* filterInfo)
{
  filterInfo->Name          = "Block wb 2/1";

  RegionInfoBlockParams(filterInfo);
}

void CellStatFtr::RegionInfoBlockParams(FilterInfo* filterInfo)
{
  filterInfo->Param1Name    = "block size";
  filterInfo->Param1Min     = 4;
  filterInfo->Param1Max     = 32;
  filterInfo->Param1Default = 8;

  filterInfo->Param2Name    = "min diff";
  filterInfo->Param2Min     = 1;
  filterInfo->Param2Max     = 64;
  filterInfo->Param2Default = 16;
}

bool CellStatFtr::RegionTestBlockWgb(int cellSize, int minDiff)
{
  Q_ASSERT(cellSize > 2 && cellSize < 128);

  ByteRegion* debug = GetAnalyser()->PrepareResult();

  mCellWidth        = cellSize;
  mCellHeight       = cellSize;

  CalcCell();

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

  return true;
}

bool CellStatFtr::RegionTestBlockWb11(int cellSize, int minDiff)
{
  ByteRegion* debug = GetAnalyser()->PrepareResult();

  mCellWidth        = cellSize;
  mCellHeight       = cellSize;

  CalcCell();

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

  return true;
}

bool CellStatFtr::RegionTestBlockWb21(int cellSize, int minDiff)
{
  ByteRegion* debug = GetAnalyser()->PrepareResult();

  mCellWidth        = cellSize;
  mCellHeight       = cellSize;

  CalcCell();

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

  return true;
}

void CellStatFtr::CalcCell()
{
  Q_ASSERT(mCellWidth >= 2 && mCellWidth < 128 && mCellHeight >= 2 && mCellHeight < 128);

  if (!StartCalcPrimary()) {
    return;
  }

  mCellHyst.SetSize(0, 0);
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

void CellStatFtr::CalcCell2()
{
  if (!StartCalcSecondary(eCell2)) {
    return;
  }

  const int   kCell2Size = 2;

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

void CellStatFtr::CalcCell4()
{
  if (!StartCalcSecondary(eCell4)) {
    return;
  }

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


CellStatFtr::CellStatFtr(Analyser* _Analyser)
  : ImageFilter(_Analyser)
{
}
