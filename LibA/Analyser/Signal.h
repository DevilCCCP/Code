#pragma once

#include <QVector>


struct Signal {
  int  Left;
  int  Top;
  int  Right;
  int  LeftValue;
  int  TopValue;
  int  RightValue;

  int  SymbolId;
};
typedef QVector<Signal> SignalLine;
typedef QVector<SignalLine> SignalArea;

struct SignalPack {
  int  Left;
  int  Right;
};
typedef QVector<SignalPack> PackLine;
typedef QVector<PackLine> PackArea;

struct Signal2 {
  int  Left;
  int  Right;
  int  Good;
  int  Height;
};
typedef QVector<Signal2> Signal2Line;
typedef QVector<Signal2Line> Signal2Area;

namespace SignalCtor {
struct Extrem {
  int  Location;
  int  Value;
  bool IsMaxi;
};
typedef QVector<Extrem> LineExm;

struct Move {
  int  Left;
  int  LeftValue;
  int  Right;
  int  RightValue;
  int  Rise;
};
typedef QVector<Move> LineMov;
}
