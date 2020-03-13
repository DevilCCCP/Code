#pragma once

#include <QVector>
#include <QRect>

#include "Analyser.h"


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
  void ConnectAny(const Region<uchar>& region);
  void ConnectBlack(const Region<uchar>& region, int maxValue);
  bool FillObjectBlack(int index, const QRect& rect, Region<uchar>* region);

private:
  void SetAny(const Region<uchar>& region);
  void SetBlack(const Region<uchar>& region, int maxValue);
  void Connect();
  void Construct();

public:
  ObjConnect();
};
