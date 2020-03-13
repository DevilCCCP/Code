#pragma once

#include <QVector>
#include <QRect>

#include <Lib/Include/Common.h>


class BlockBorder
{
  PROPERTY_GET_SET(QVector<int>, Left)
  PROPERTY_GET_SET(QVector<int>, Right)
  PROPERTY_GET_SET(QVector<int>, Top)
  PROPERTY_GET_SET(QVector<int>, Bottom)

public:
  void Init(const QRect& block);

  void TestTopLeft(int ii, int jj)
  {
    if (mRight[jj] >= 0) {
      mRight[jj] = ii;
    } else {
      mLeft[jj] = mRight[jj] = ii;
    }
    if (mBottom[ii] >= 0) {
      mBottom[ii] = jj;
    } else {
      mTop[ii] = mBottom[ii] = jj;
    }
  }

public:
  BlockBorder();
};

