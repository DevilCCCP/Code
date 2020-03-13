#pragma once

#include <QVector>
#include <QRect>

#include "Tools.h"


template<typename DataT>
class Region {
  QVector<DataT>     mInternalSource;
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

  void SetSource(const Region& other, const QRect& subRect)
  {
    mData     = other.Data(subRect.x(), subRect.y());
    mStride   = other.mStride;
    mWidth    = subRect.width();
    mHeight   = subRect.height();
  }

  void SetSource(const Region& other)
  {
    mData     = other.mData;
    mWidth    = other.mWidth;
    mHeight   = other.mHeight;
    mStride   = other.mStride;
  }

  QRect Rect() const
  {
    return QRect(0, 0, mWidth, mHeight);
  }

  QSize Size() const
  {
    return QSize(mWidth, mHeight);
  }

  void SetSize(const QSize& size)
  {
    SetSize(size.width(), size.height());
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

  void FillData(int value)
  {
    if (mWidth == mStride) {
      memset(mData, value, mWidth * mHeight * sizeof(DataT));
    } else {
      for (int j = 0; j < Height(); j++) {
        memset(Line(j), value, mWidth * sizeof(DataT));
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

  void Copy(const Region<DataT>& source, const QRect& sourceRect)
  {
    for (int j = sourceRect.top(); j <= sourceRect.bottom(); j++) {
      const DataT* src = source.Data(sourceRect.left(), j);
      DataT* dst = Data(sourceRect.left(), j);
      memcpy(dst, src, sourceRect.width() * sizeof(DataT));
    }
  }

  void Copy(const QPoint& destPoint, const Region<DataT>& source, const QRect& sourceRect)
  {
    if (destPoint.x() < 0 || destPoint.x() + sourceRect.width() >= Width()
        || destPoint.y() < 0 || destPoint.y() + sourceRect.height() >= Height()) {
      return;
    }

    for (int j = sourceRect.top(); j <= sourceRect.bottom(); j++) {
      const DataT* src = source.Data(sourceRect.left(), j);
      DataT* dst = Data(destPoint.x(), destPoint.y() + j - sourceRect.top());
      memcpy(dst, src, sourceRect.width() * sizeof(DataT));
    }
  }

  void LineZero(int line)
  {
    memset(Line(line), 0, mWidth * sizeof(DataT));
  }

  const DataT* Line(int line) const
  {
    Q_ASSERT(line >= 0 && line < Height());

    return &mData[line * mStride];
  }

  DataT* Data(int i, int j) const
  {
    Q_ASSERT(i >= 0 && i < Width());
    Q_ASSERT(j >= 0 && j < Height());

    return mData + (j * Stride() + i);
  }

  DataT* Data(const QPoint& p) const
  {
    Q_ASSERT(p.x() >= 0 && p.x() < Width());
    Q_ASSERT(p.y() >= 0 && p.y() < Height());

    return mData + (p.y() * Stride() + p.x());
  }

  DataT* Line(int line)
  {
    Q_ASSERT(line >= 0 && line < Height());

    return &mData[line * mStride];
  }

  void FillRectBorder(const QRect& rect, int color)
  {
    int jmin = qMax(0, rect.top());
    int jmax = qMin(Height() - 1, rect.bottom());
    int imin = qMax(0, rect.left());
    int imax = qMin(Width() - 1, rect.right());
    int isize = imax - imin + 1;
    memset(Data(imin, jmin), color, isize);
    memset(Data(imin, jmax), color, isize);
    for (int j = jmin + 1; j < jmax; j++) {
      *Data(imin, j) = color;
      *Data(imax, j) = color;
    }
  }

  void FillRect(const QRect& rect, int color)
  {
    int jmin = qMax(0, rect.top());
    int jmax = qMin(Height() - 1, rect.bottom());
    int imin = qMax(0, rect.left());
    int imax = qMin(Width() - 1, rect.right());
    int isize = imax - imin + 1;
    memset(Data(imin, jmin), color, isize);
    memset(Data(imin, jmax), color, isize);
    for (int j = jmin; j <= jmax; j++) {
      memset(Data(imin, j), color, isize);
    }
  }

  void FillLine(const QPoint& p1, const QPoint& p2, int color)
  {
    Q_ASSERT(p1.x() >= 0 && p1.x() < Width());
    Q_ASSERT(p1.y() >= 0 && p1.y() < Height());
    Q_ASSERT(p2.x() >= 0 && p2.x() < Width());
    Q_ASSERT(p2.y() >= 0 && p2.y() < Height());

    int l = p2.x() - p1.x();
    int h = p2.y() - p1.y();
    if (qAbs(l) <= qAbs(h)) {
      if (l == 0) {
        *Data(p1) = color;
        return;
      }
      int inc = p2.y() > p1.y()? 1: -1;
      for (int j = 0; j != h; j += inc) {
        int i = p1.x() + j * l/h;
        *Data(i, p1.y() + j) = color;
      }
    } else {
      int inc = p2.x() > p1.x()? 1: -1;
      for (int i = 0; i != l; i += inc) {
        int j = p1.y() + i * h/l;
        *Data(p1.x() + i, j) = color;
      }
    }
  }

  int SumLine(const QPoint& p1, const QPoint& p2) const
  {
    Q_ASSERT(p1.x() >= 0 && p1.x() < Width());
    Q_ASSERT(p1.y() >= 0 && p1.y() < Height());
    Q_ASSERT(p2.x() >= 0 && p2.x() < Width());
    Q_ASSERT(p2.y() >= 0 && p2.y() < Height());

    int result = 0;
    int l = p2.x() - p1.x();
    int h = p2.y() - p1.y();
    if (qAbs(l) <= qAbs(h)) {
      if (l == 0) {
        return *Data(p1.x(), p2.x());
      }
      int inc = p2.y() > p1.y()? 1: -1;
      for (int j = 0; j != h; j += inc) {
        int i = p1.x() + j * l/h;
        result += *Data(i, p1.y() + j);
      }
    } else {
      int inc = p2.x() > p1.x()? 1: -1;
      for (int i = 0; i != l; i += inc) {
        int j = p1.y() + i * h/l;
        result += *Data(p1.x() + i, j);
      }
    }
    return result;
  }

public:
  Region()
    : mData(nullptr), mWidth(0), mHeight(0), mStride(0)
  {
  }

  Region(DataT* _SourceData, int _Width, int _Height, int _Stride)
  {
    SetSource(_SourceData, _Width, _Height, _Stride);
  }

  Region(const Region& other, int x, int y, int width, int height)
  {
    SetSource(other, x, y, width, height);
  }

  Region(const Region& other, const QRect& subRect)
  {
    SetSource(other, subRect);
  }

  Region(int _Width, int _Height)
  {
    SetSize(_Width, _Height);
  }

  Region(const QSize& _Size)
  {
    SetSize(_Size);
  }

  Region(const Region<DataT>& other)
  {
    SetSource(other);
    if (!other.mInternalSource.isEmpty()) {
      mInternalSource = other.mInternalSource;
      mData = mInternalSource.data();
    }
  }
};
