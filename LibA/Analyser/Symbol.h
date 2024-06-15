#pragma once

#include <QVector>

#include "Signal.h"
#include "ByteRegion.h"


struct Symbol {
  int        Left;
  int        Right;
  int        Top;
  int        Bottom;

  int        Parent;
  int        ParentV;
};

struct SymbolV {
  int        Left;
  int        Right;
  int        Top;
  int        Bottom;

  ByteRegion Data;
};
