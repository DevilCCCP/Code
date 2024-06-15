#pragma once

#include <QVector>

#include "ImageFilter.h"
#include "Signal.h"
#include "Symbol.h"
#include "Region.h"


DefineClassS(ObjConnect);

class SymbolMarkFtr: public ImageFilter
{
  PROPERTY_GET_SET(int, SignalFitMax)
  PROPERTY_GET_SET(int, SymbolWidthMin)
  PROPERTY_GET_SET(int, SymbolWidthMax)
  PROPERTY_GET_SET(int, SymbolHeightMin)
  PROPERTY_GET_SET(int, SymbolHeightMax)

  int                   mCurrentLine;
  QVector<Symbol>       mSymbolList;
  QVector<SymbolV>      mSymbolVList;

public:
  void RegionInfoSymbolPre(FilterInfo* filterInfo);
  void RegionInfoSymbolRaw(FilterInfo* filterInfo);

  bool RegionTestSymbolPre(int fitMax, int);
  bool RegionTestSymbolRaw(int scale, int hzCount);

  void CalcSymbol();

private:
  void MarkSymbol(SignalLine* signalLine, SignalLine* signalLinep);
  int  MarkSymbolGetId(int childId);
  void MarkSymbolSetId(int oldId, int newId);
  void MarkSymbolFixId(int id, int newParentId);
  void MarkSymbolRegisterNew(Signal* signal);
  void MarkSymbolOptimize();

public:
  SymbolMarkFtr(Analyser* _Analyser);
};

