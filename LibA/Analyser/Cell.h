#pragma once

#include "Hyst.h"


struct Cell {
  Hyst      CellHyst;
  int       BlackValue;
  int       WhiteValue;
  int       CountBlack;
  int       CountWhite;
  Cell(): BlackValue(0), WhiteValue(255) {}
};
