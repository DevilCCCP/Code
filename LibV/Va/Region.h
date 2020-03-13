#pragma once

#include <vector>

#include "Tools.h"


template<typename DataT>
class Region {
  std::vector<DataT> mInternalSource;
  DataT*             mData;
  int                mWidth;
  int                mHeight;
  int                mStride;

public:
  int  Width()  const { return mWidth; }
  int  Height() const { return mHeight; }
  int  Stride() const { return mStride; }

  void SetSource(DataT* _SourceData, int _Width, int _Height, int _Stride)
  {
    mData     = _SourceData;
    mWidth    = _Width;
    mHeight   = _Height;
    mStride   = _Stride;
  }

  void SetSource(const Region& other, int x, int y, int width, int height)
  {
    mData     = other.Data(x, y);
    mStride   = other.mStride;
    mWidth    = width;
    mHeight   = height;
  }

  void SetSize(int _Width, int _Height)
  {
    mWidth  = _Width;
    mHeight = _Height;
    mStride = mWidth;

    mInternalSource.resize(mWidth * mHeight);
    mData = mInternalSource.data();
  }

public:
  void ZeroData()
  {
    if (mWidth == mStride) {
      memset(mData, 0, mWidth * mHeight * sizeof(DataT));
    } else {
      for (int j = 0; j < Height(); j++) {
        memset(Line(j), 0, mWidth * sizeof(DataT));
      }
    }
  }

  void SetData(int val)
  {
    if (mWidth == mStride) {
      memset(mData, val, mWidth * mHeight * sizeof(DataT));
    } else {
      for (int j = 0; j < Height(); j++) {
        memset(Line(j), val, mWidth * sizeof(DataT));
      }
    }
  }
  void LineZero(int line)
  {
    memset(Line(line), 0, mWidth * sizeof(DataT));
  }

  const DataT* Line(int line) const
  {
    Q_ASSERT(line < Height());

    return &mData[line * mStride];
  }

  DataT* Data(int i, int j) const
  {
    Q_ASSERT(i < Width());
    Q_ASSERT(j < Height());

    return mData + (j * Stride() + i);
  }

  DataT* Data(const QPoint& p) const
  {
    Q_ASSERT(p.x() < Width());
    Q_ASSERT(p.y() < Height());

    return mData + (p.y() * Stride() + p.x());
  }

  DataT* Line(int line)
  {
    Q_ASSERT(line < Height());

    return &mData[line * mStride];
  }

public:
  Region()
    : mData(nullptr), mWidth(0), mHeight(0), mStride(0)
  {
  }
};
