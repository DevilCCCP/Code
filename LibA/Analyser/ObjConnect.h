#pragma once

#include <QVector>
#include <QRect>


#include "Analyser.h"
#include "Region.h"
#include "ByteRegion.h"


class ObjConnect
{
  Region<int>            mRegionMark;

  QVector<int>           mObjIds;
  QVector<int>           mObjIds2;
  int                    mObjCount;
  QVector<QRect>         mObjList;

public:
  const QVector<QRect>& GetObjList() const { return mObjList; }
  int GetObjCount() const { return mObjCount; }

public:
  void ConnectAny(const ByteRegion& region);
  void ConnectBlack(const ByteRegion& region, int maxValue);
  bool FillObjectBlack(int index, const QRect& rect, ByteRegion* region);

private:
  void SetAny(const ByteRegion& region);
  void SetBlack(const ByteRegion& region, int maxValue);
  void Connect();
  void Construct();

public:
  ObjConnect();
};
