#pragma once

#include <QList>
#include <QImage>

#include <Lib/Include/Common.h>
#include <LibA/Analytics/Hyst.h>
#include <LibV/Va/Region.h>


class Uin
{
  typedef Region<uchar>* RegionPtr;

  PROPERTY_GET_SET(RegionPtr, Region)
  PROPERTY_GET    (int,       Quality)
  PROPERTY_GET    (QChar,     Char)
  PROPERTY_GET    (RegionPtr, DebugRegion)

  struct CharMap {
    QChar         Char;
    Region<uchar> Source;
    Region<uchar> Map;
  };

  QList<CharMap>            mCharsMap;

  Hyst                      mCharHyst;
  Region<uchar>             mCharRegion;
  Region<uchar>             mCharMapRegion;
  std::vector<int>          mCharLine;
  std::vector<int>          mCharColumn;

public:
  void AddChar(const QChar& _Char, const QImage& _Image);
  bool Calc();

private:
  void MkChar3Color();
  void CalcQuality();
  int CalcQualityOne(const CharMap& chmap);

  static void CalcRegionHyst(const Region<uchar>& region, Hyst& hyst);
  bool CutChar(const Region<uchar>& region, Region<uchar>& regionResized, int* dx = nullptr, int* dy = nullptr);

public:
  Uin();
  ~Uin();
};
