#pragma once

#include "ImageFilter.h"
#include "Hyst.h"
#include "Region.h"
#include "Cell.h"


class CellStatFtr: public ImageFilter
{
  PROPERTY_GET(int,          CellWidth)
  PROPERTY_GET(int,          CellHeight)

  PROPERTY_GET(Region<Cell>, CellHyst)
  PROPERTY_GET(Region<Cell>, Cell2Hyst)
  PROPERTY_GET(Region<Cell>, Cell4Hyst)

public:
  void RegionInfoBlockWgb(FilterInfo* filterInfo);
  void RegionInfoBlockWb11(FilterInfo* filterInfo);
  void RegionInfoBlockWb21(FilterInfo* filterInfo);
  void RegionInfoBlockParams(FilterInfo* filterInfo);

  bool RegionTestBlockWgb(int cellSize, int minDiff);
  bool RegionTestBlockWb11(int cellSize, int minDiff);
  bool RegionTestBlockWb21(int cellSize, int minDiff);

public:
  void CalcCell();
  void CalcCell2();
  void CalcCell4();

public:
  CellStatFtr(Analyser* _Analyser);
};

