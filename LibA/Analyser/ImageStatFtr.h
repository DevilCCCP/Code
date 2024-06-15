#pragma once

#include "ImageFilter.h"
#include "Hyst.h"
#include "Region.h"


class ImageStatFtr: public ImageFilter
{
  Hyst                      mHyst;

  int                       mCellWidth;
  int                       mCellHeight;

  struct Cell {
    Hyst      CellHyst;
    int       BlackValue;
    int       WhiteValue;
    int       CountBlack;
    int       CountWhite;
    Cell(): BlackValue(0), WhiteValue(255) {}
  };

  Region<Cell>              mCellHyst;
  Region<Cell>              mCell2Hyst;
  Region<Cell>              mCell4Hyst;

public:
  void MakeGrad();
  void MakeWhiteBallance(int white = 950, int black = 50);

public:
  void CalcHyst(int x, int y, int width, int height);
  void CalcCell();
  void CalcCell2();
  void CalcCell4();

public:
  ImageStatFtr(Analyser* _Analyser);
};

