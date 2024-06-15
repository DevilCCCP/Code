#include "ImageStatFtr.h"


void ImageStatFtr::MakeGrad()
{
  ByteRegion* debug = GetAnalyser()->PrepareResult();

  for (int j = 0; j < Height() - 1; j++) {
    const uchar* srcnn = Source().Line(j);
    const uchar* srcnp = Source().Line(j + 1);
    const uchar* srcpn = Source().Line(j) + 1;
    uchar* img = debug->Line(j);
    for (int i = 0; i < Width() - 1; i++) {
      int h = qAbs((int)*srcnn - (int)*srcpn);
      int v = qAbs((int)*srcnn - (int)*srcnp);
      int d = qMax(h, v);
      d = (d > 5)? qMin(255, 127 + d): 5*d;
      *img = (uchar)(uint)d;

      srcnn++;
      srcnp++;
      srcpn++;
      img++;
    }
    *img = 0;
  }
  memset(debug->Line(Height() - 1), 0, Width());
}

void ImageStatFtr::MakeWhiteBallance(int white, int black)
{
  CalcHyst(0, 0, Source().Width(), Source().Height());

  int whiteValue = mHyst.GetValue(white);
  int blackValue = mHyst.GetValue(black);
  int denum = qMax(whiteValue - blackValue, 1);

  ByteRegion* debug = GetAnalyser()->PrepareResult();
  for (int j = 0; j < Height(); j++) {
    const uchar* src = Source().Line(j);
    uchar*       dst = debug->Line(j);
    for (int i = 0; i < Width(); i++) {
      int v = qMax(0, *src - blackValue);
      *dst = qMin(255, v * 255 / denum);

      src++;
      dst++;
    }
  }
}

void ImageStatFtr::CalcHyst(int x, int y, int width, int height)
{
  mHyst.Clear();
  for (int j = 0; j < height; j++) {
    const uchar* src = Source().Data(x, y + j);
    for (int i = 0; i < width; i++) {
      mHyst.Inc(*src);
      src++;
    }
  }
}

void ImageStatFtr::CalcCell()
{
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

void ImageStatFtr::CalcCell2()
{
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

void ImageStatFtr::CalcCell4()
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


ImageStatFtr::ImageStatFtr(Analyser* _Analyser)
  : ImageFilter(_Analyser)
{
}
