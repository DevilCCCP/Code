#include "SymbolMarkFtr.h"
#include "SignalMarkFtr.h"


const int kSignalFitMax   = 20;
const int kSymbolWidthMin  = 6;
const int kSymbolWidthMax  = 20;
const int kSymbolHeightMin = 8;
const int kSymbolHeightMax = 28;

void SymbolMarkFtr::RegionInfoSymbolPre(FilterInfo* filterInfo)
{
  filterInfo->Name          = "Symbol pre";

  filterInfo->Param1Name    = "max diff";
  filterInfo->Param1Min     = 4;
  filterInfo->Param1Max     = 255;
  filterInfo->Param1Default = 255;
}

void SymbolMarkFtr::RegionInfoSymbolRaw(FilterInfo* filterInfo)
{
  filterInfo->Name          = "Symbol raw";

  filterInfo->Param1Name    = "scale";
  filterInfo->Param1Min     = 1;
  filterInfo->Param1Max     = 20;
  filterInfo->Param1Default = 3;

  filterInfo->Param2Name    = "hz clued";
  filterInfo->Param2Min     = 1;
  filterInfo->Param2Max     = 6;
  filterInfo->Param2Default = 3;
}

bool SymbolMarkFtr::RegionTestSymbolPre(int fitMax, int)
{
  mSignalFitMax = fitMax;

  const SignalMarkFtrS& signalMarkFtr = GetAnalyser()->GetSignalMarkFtr();
  signalMarkFtr->CalcSignal();

  CalcSymbol();

  ByteRegion* debug = GetAnalyser()->PrepareResultWhite();
//  for (int i = 1; i < mSymbolList.size(); i++) {
//    const Symbol* symbol = &mSymbolList.at(i);
//    if (symbol->Parent == 0) {
//      int value = 50 + (i % 10) * 20;
//      for (int jj = symbol->Top; jj <= symbol->Bottom; jj++) {
//        memset(debug->Data(symbol->Left, jj), value, symbol->Right - symbol->Left + 1);
//      }
//    }
//  }

  const SignalArea& signalArea = signalMarkFtr->GetSignalArea();
  for (int j = 0; j < Source().Height(); j++) {
    const SignalLine* lineSig = &signalArea.at(j);
    foreach (const Signal& sig, *lineSig) {
      int symbolId = MarkSymbolGetId(sig.SymbolId);
      int value = 50 + (symbolId % 10) * 20;
      uchar* dbg = debug->Data(sig.Left, j);
      for (int i = sig.Left; i <= sig.Right; i++) {
        *dbg = value;

        dbg++;
      }
    }
  }

  return true;
}

bool SymbolMarkFtr::RegionTestSymbolRaw(int scale, int hzCount)
{
  mSymbolWidthMin  = kSymbolWidthMin * scale / 2;
  mSymbolWidthMax  = kSymbolWidthMax * scale * hzCount / 2;
  mSymbolHeightMin = kSymbolHeightMin * scale / 2;
  mSymbolHeightMax = kSymbolHeightMax * scale / 2;

  const SignalMarkFtrS& signalMarkFtr = GetAnalyser()->GetSignalMarkFtr();
  signalMarkFtr->CalcSignal();
  CalcSymbol();

  ByteRegion* debug = GetAnalyser()->PrepareResultWhite();
  for (int i = 1; i < mSymbolVList.size(); i++) {
    const SymbolV* symbolV = &mSymbolVList.at(i);
    int value = 150 + (i % 10) * 10;
    for (int jj = symbolV->Top; jj <= symbolV->Bottom; jj++) {
      memset(debug->Data(symbolV->Left, jj), value, symbolV->Right - symbolV->Left + 1);
    }
  }

  const SignalArea& signalArea = signalMarkFtr->GetSignalArea();
  for (int j = 0; j < Source().Height(); j++) {
    const SignalLine* lineSig = &signalArea.at(j);
    foreach (const Signal& sig, *lineSig) {
      int symbolId = MarkSymbolGetId(sig.SymbolId);
      int symbolIdV = mSymbolList[symbolId].ParentV;
      if (symbolIdV > 0) {
        int value = 50 + (symbolIdV % 10) * 10;
        uchar* dbg = debug->Data(sig.Left, j);
        for (int i = sig.Left; i <= sig.Right; i++) {
          *dbg = value;

          dbg++;
        }
      }
    }
  }

  return true;
}

void SymbolMarkFtr::CalcSymbol()
{
  if (!StartCalcPrimary()) {
    return;
  }

  const SignalMarkFtrS& signalMarkFtr = GetAnalyser()->GetSignalMarkFtr();
  SignalArea* signalArea = signalMarkFtr->CalcSignal();

  mSymbolList.clear();
  mSymbolList.append(Symbol());
  mSymbolVList.clear();
  mSymbolVList.append(SymbolV());

  SignalLine nullLine;
  SignalLine* signalLinep = &nullLine;
  for (mCurrentLine = 0; mCurrentLine < Height(); mCurrentLine++) {
    SignalLine* signalLine = &(*signalArea)[mCurrentLine];
    MarkSymbol(signalLine, signalLinep);
    signalLinep = signalLine;
  }

  MarkSymbolOptimize();
}

void SymbolMarkFtr::MarkSymbol(SignalLine* signalLine, SignalLine* signalLinep)
{
  const int kDiagonal = 0; /// valid: {0, 1}

  auto itrp = signalLinep->begin();
  auto itr = signalLine->begin();
  while (itr != signalLine->end() && itrp != signalLinep->end()) {
    if (itrp->Right + kDiagonal < itr->Left) {
      itrp++;
      continue;
    } else if (itr->Right + kDiagonal < itrp->Left) {
      if (itr->SymbolId == 0) {
        MarkSymbolRegisterNew(itr);
      }
      itr++;
      continue;
    }

    if (qAbs(itr->TopValue - itrp->TopValue) <= mSignalFitMax) {
      Symbol* symbol;
      int symbolId = itr->SymbolId;
      int symbolIdp = MarkSymbolGetId(itrp->SymbolId);
      if (symbolId == 0) {
        symbolId = symbolIdp;
        itr->SymbolId = symbolId;
        symbol = &mSymbolList[symbolId];
      } else {
        if (symbolId < symbolIdp) {
          MarkSymbolSetId(symbolIdp, symbolId);
        } else if (symbolIdp < symbolId) {
          MarkSymbolSetId(symbolId, symbolIdp);
          symbolId = symbolIdp;
          itr->SymbolId = symbolId;
        }
        symbol = &mSymbolList[symbolId];
        Symbol* symbolp = &mSymbolList[symbolIdp];
        symbol->Left   = qMin(symbol->Left, symbolp->Left);
        symbol->Right  = qMax(symbol->Right, symbolp->Right);
        symbol->Top    = qMin(symbol->Top, symbolp->Top);
      }

      symbol->Left   = qMin(symbol->Left, itr->Left);
      symbol->Right  = qMax(symbol->Right, itr->Right);
      symbol->Bottom = mCurrentLine;
    }

    if (itrp->Right < itr->Right) {
      ++itrp;
    } else {
      if (itr->SymbolId == 0) {
        MarkSymbolRegisterNew(itr);
      }
      ++itr;
    }
  }

  /// finish or first line
  for (; itr != signalLine->end(); ++itr) {
    if (itr->SymbolId != 0) { /// already registered
      continue;
    }

    MarkSymbolRegisterNew(itr);
  }
}

int SymbolMarkFtr::MarkSymbolGetId(int childId)
{
  if (!mSymbolList[childId].Parent) {
    return childId;
  }

  int parentId = mSymbolList[childId].Parent;
  for ( ; parentId; ) {
    childId = parentId;
    parentId = mSymbolList[parentId].Parent;
  }
  return childId;
}

void SymbolMarkFtr::MarkSymbolSetId(int oldId, int newId)
{
  int parentId = mSymbolList[oldId].Parent;
  while (parentId) {
    int upId = mSymbolList[oldId].Parent;
    mSymbolList[oldId].Parent = newId;
    mSymbolList[newId].Left  = qMin(mSymbolList[newId].Left, mSymbolList[oldId].Left);
    mSymbolList[newId].Right = qMax(mSymbolList[newId].Right, mSymbolList[oldId].Right);
    mSymbolList[newId].Top   = qMin(mSymbolList[newId].Top, mSymbolList[oldId].Top);
    Q_ASSERT(mSymbolList[oldId].Parent != oldId);
    oldId = parentId;
    parentId = upId;
  }
  mSymbolList[oldId].Parent = newId;
  mSymbolList[newId].Left  = qMin(mSymbolList[newId].Left, mSymbolList[oldId].Left);
  mSymbolList[newId].Right = qMax(mSymbolList[newId].Right, mSymbolList[oldId].Right);
  mSymbolList[newId].Top   = qMin(mSymbolList[newId].Top, mSymbolList[oldId].Top);
  Q_ASSERT(mSymbolList[oldId].Parent != oldId);
}

void SymbolMarkFtr::MarkSymbolFixId(int id, int newParentId)
{
  int parentId = mSymbolList[id].Parent;
  if (mSymbolList[id].Parent == 0) {
    return;
  }
  mSymbolList[id].Parent = newParentId;
//  mSymbolList[newParentId].Left  = qMin(mSymbolList[newParentId].Left, mSymbolList[id].Left);
//  mSymbolList[newParentId].Right = qMax(mSymbolList[newParentId].Right, mSymbolList[id].Right);
//  mSymbolList[newParentId].Top   = qMin(mSymbolList[newParentId].Top, mSymbolList[id].Top);
  Q_ASSERT(mSymbolList[id].Parent != id);
  while (parentId) {
    int upId = mSymbolList[id].Parent;
    if (mSymbolList[id].Parent == 0) {
      return;
    }
    mSymbolList[id].Parent = newParentId;
//    mSymbolList[newParentId].Left  = qMin(mSymbolList[newParentId].Left, mSymbolList[id].Left);
//    mSymbolList[newParentId].Right = qMax(mSymbolList[newParentId].Right, mSymbolList[id].Right);
//    mSymbolList[newParentId].Top   = qMin(mSymbolList[newParentId].Top, mSymbolList[id].Top);
    Q_ASSERT(mSymbolList[id].Parent != id);
    id = parentId;
    parentId = upId;
  }
}

void SymbolMarkFtr::MarkSymbolRegisterNew(Signal* signal)
{
  signal->SymbolId = mSymbolList.size();
  mSymbolList.append(Symbol());
  Symbol* symbol = &mSymbolList.last();
  symbol->Left   = signal->Left;
  symbol->Right  = signal->Right;
  symbol->Top    = mCurrentLine;
  symbol->Bottom = mCurrentLine;
}

void SymbolMarkFtr::MarkSymbolOptimize()
{
  for (int symbolId = 1; symbolId < mSymbolList.size(); symbolId++) {
    int parentId = MarkSymbolGetId(symbolId);
    MarkSymbolFixId(symbolId, parentId);
    Symbol* symbol = &mSymbolList[parentId];
    if (!symbol->ParentV) {
      int w = symbol->Right - symbol->Left + 1;
      int h = symbol->Bottom - symbol->Top + 1;
      if (w >= mSymbolWidthMin && w <= mSymbolWidthMax && h >= mSymbolHeightMin && h <= mSymbolHeightMax) {
        symbol->ParentV = mSymbolVList.size();
        mSymbolVList.append(SymbolV());
        SymbolV* symbolV = &mSymbolVList.last();
        symbolV->Left   = symbol->Left;
        symbolV->Right  = symbol->Right;
        symbolV->Top    = symbol->Top;
        symbolV->Bottom = symbol->Bottom;
      } else {
        symbol->ParentV = -1;
      }
    }
  }
}


SymbolMarkFtr::SymbolMarkFtr(Analyser* _Analyser)
  : ImageFilter(_Analyser)
{
  mSignalFitMax    = kSignalFitMax;
  mSymbolWidthMin  = kSymbolWidthMin;
  mSymbolWidthMax  = kSymbolWidthMax;
  mSymbolHeightMin = kSymbolHeightMin;
  mSymbolHeightMax = kSymbolHeightMax;
}
