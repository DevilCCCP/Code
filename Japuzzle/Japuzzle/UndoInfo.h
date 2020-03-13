#pragma once

#include <QPoint>

#include "Cell.h"
#include "Propotion.h"


struct UndoInfo {
  QPoint             P1;
  QPoint             P2;
  QVector<Cell>      Cells;
  QVector<Propotion> Prop;

  UndoInfo(): P1(-1, -1), P2(-1, -1) { }
};
