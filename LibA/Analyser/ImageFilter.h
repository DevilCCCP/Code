#pragma once

#include <QVector>

#include "Analyser.h"
#include "ByteRegion.h"


class ImageFilter
{
  Analyser*    mAnalyser;

  int          mPrimaryVersion;
  QVector<int> mSecondaryVersionList;

protected:
  int Width() const { return mAnalyser->Width(); }
  int Height() const { return mAnalyser->Height(); }
  Analyser* GetAnalyser() { return mAnalyser; }
  const ByteRegion& Source() const { return mAnalyser->Source(); }

  const uchar*    Line()     { return mAnalyser->Line(); }
  int             LineSize() { return mAnalyser->LineSize(); }

protected:
  bool StartCalcPrimary();
  bool StartCalcSecondary(int index);
  void ClearPrimary();
  void ClearSecondary(int index);

public:
  ImageFilter(Analyser* _Analyser);
  virtual ~ImageFilter();
};

