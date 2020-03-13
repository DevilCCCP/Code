#pragma once

#include <QVector>
#include <QRect>

#include <LibV/Include/Tools.h>


class BlockScene {
  int       mBlockSize;
  int       mSafeWidth;
  int       mSafeHeight;
  int       mFullWidth;
  int       mFullHeight;
  int       mSrcWidth;
  int       mSrcHeight;
  int       mBlockWidth;
  int       mBlockHeight;

public:
  int BlockSize()   const { return mBlockSize; }
  int Width()       const { return mSafeWidth; }
  int Height()      const { return mSafeHeight; }
  int BlockWidth()  const { return mBlockWidth; }
  int BlockHeight() const { return mBlockHeight; }
  int SrcWidth()    const { return mSrcWidth; }
  int SrcHeight()   const { return mSrcHeight; }

public:
  void SetScene(int blockSize, int width, int height)
  {
    mBlockSize   = blockSize;
    mSrcWidth    = width;
    mSrcHeight   = height;
    mSafeWidth   = GetAlignedLeft(width, mBlockSize);
    mSafeHeight  = GetAlignedLeft(height, mBlockSize);
    mFullWidth   = GetAlignedRight(width, mBlockSize);
    mFullHeight  = GetAlignedRight(height, mBlockSize);
    mBlockWidth  = mSafeWidth / mBlockSize;
    mBlockHeight = mSafeHeight / mBlockSize;
  }

public:
  BlockScene()
  { }
};

template<typename DataT>
class ImageSrc {
  const BlockScene& mBlockScene;

  QVector<DataT> mInternalSource;
  const DataT*   mExternalSource;
  int            mStride;
  bool           mPacked;

public:
  void setPacked(bool _Packed) { mPacked = _Packed; }
  bool isPacked() const { return mPacked; }
  int  Width()  const { return mBlockScene.Width(); }
  int  Height() const { return mBlockScene.Height(); }
  int  Stride() const { return mStride; }
  void SetStride(int _Stride) { mStride = _Stride; }
  void SetSource(const DataT* _ImageSource) { mExternalSource = _ImageSource; }

public:
  void InitSource()
  {
    InitVector(mInternalSource, mBlockScene.Width() * mBlockScene.Height());
    SetSource(mInternalSource.constData());
    SetStride(mBlockScene.Width());
  }

  void ClearSource()
  {
    memset(const_cast<DataT*>(mExternalSource), 0, Stride() * mBlockScene.SrcHeight() * sizeof(DataT));
  }

  void ZeroSource()
  {
    memset(const_cast<DataT*>(mExternalSource), 0, mBlockScene.Width() * mBlockScene.Height() * sizeof(DataT));
  }

  void InitSourceWithSafe(int safeSizeLeft, int safeSizeRight)
  {
    InitVector(mInternalSource, (mBlockScene.Width() + safeSizeLeft + safeSizeRight) * mBlockScene.Height());
    SetSource(mInternalSource.constData() + safeSizeLeft);
    SetStride(mBlockScene.Width() + safeSizeLeft + safeSizeRight);
  }

  void PrepareSource()
  {
    mInternalSource.resize(mBlockScene.Width() * mBlockScene.Height());
    SetSource(mInternalSource.constData());
    SetStride(mBlockScene.Width());
  }

  void LineZero(int line)
  {
    memset(Line(line), 0, mStride * sizeof(DataT));
  }

  const DataT* Line(int line) const
  {
    Q_ASSERT(line < Height());

    return const_cast<DataT*>(&mExternalSource[line * mStride]);
  }

  DataT* Line(int line)
  {
    Q_ASSERT(line < Height());

    return const_cast<DataT*>(&mExternalSource[line * mStride]);
  }

  void FillRect(const DataT& val, const QRect& rect)
  {
    int x1 = rect.left();
    int y1 = rect.top();
    int x2 = rect.right();
    int y2 = rect.bottom();
    for (int j = y1; j <= y2; j++) {
      DataT* data = Line(j) + x1;
      for (int i = x1; i <= x2; i++) {
        *data = val;
        data++;
      }
    }
  }

  void FillRect(const DataT& val, int x, int y, int w, int h)
  {
    int x1 = qMax(0, x - w);
    int y1 = qMax(0, y - h);
    int x2 = qMin(Width() - 1, x + w);
    int y2 = qMin(Height() - 1, y + h);
    for (int j = y1; j <= y2; j++) {
      DataT* data = Line(j) + x1;
      for (int i = x1; i <= x2; i++) {
        *data = val;
        data++;
      }
    }
  }

  void FillLine4(const DataT& val, const QRect& rect)
  {
    FillLine4(val, rect.left(), rect.top(), rect.right(), rect.bottom());
  }

  void FillLine4(const DataT& val, int x1, int y1, int x2, int y2)
  {
    FillLine(val, x1, y1, x2, y1);
    FillLine(val, x1, y2, x2, y2);
    FillLine(val, x1, y1, x1, y2);
    FillLine(val, x2, y1, x2, y2);
  }

  void FillLine(const DataT& val, const QPoint& p1, const QPoint& p2)
  {
    FillLine(val, p1.x(), p1.y(), p2.x(), p2.y());
  }

  void FillLine(const DataT& val, int x1, int y1, int x2, int y2)
  {
    if (qAbs(x2 - x1) >= qAbs(y2 - y1)) {
      int dx = x2 - x1;
      if (dx == 0) {
        return;
      }
      int inc = (dx > 0)? 1: -1;
      for (int i = x1; i != x2 + inc; i += inc) {
        int j = y1 + (y2 - y1) * (i - x1) / (x2 - x1);
        WriteSafe(i, j, val);
      }
    } else {
      int dy = y2 - y1;
      int inc = (dy > 0)? 1: -1;
      for (int j = y1; j != y2 + inc; j += inc) {
        int i = x1 + (x2 - x1) * (j - y1) / (y2 - y1);
        WriteSafe(i, j, val);
      }
    }
  }

  void FillDiamond(const DataT& val, int x, int y, int w, int h)
  {
    int y1 = qMax(0, y - h);
    int y2 = qMin(Height() - 1, y + h);
    for (int j = y1; j <= y2; j++) {
      int w_ = (w - qAbs(y - j));
      int x1 = qMax(0, x - w_);
      int x2 = qMin(Width() - 1, x + w_);
      DataT* data = Line(j) + x1;
      for (int i = x1; i <= x2; i++) {
        *data = val;
        data++;
      }
    }
  }

  void FillRhombus(const DataT& val, int x, int y, int w, int h)
  {
    int y1 = qMax(0, y - h);
    int y2 = qMin(Height() - 1, y + h);
    for (int j = y1; j <= y2; j++) {
      int w_ = (w - qAbs(y - j));
      int x1 = x - w_;
      int x2 = x + w_;
      if (x1 >= 0) {
        DataT* data = Line(j) + x1;
        *data = val;
      } if (x2 < Width()) {
        DataT* data = Line(j) + x2;
        *data = val;
      }
    }
  }

  const DataT* Data(int i, int j) const
  {
    return const_cast<DataT*>(&mExternalSource[j * mStride + i]);
  }

  DataT* Data(int i, int j)
  {
    return const_cast<DataT*>(&mExternalSource[j * mStride + i]);
  }

  void WriteSafe(int i, int j, const DataT& value)
  {
    int ind = j * mStride + i;
    if (ind >= 0 && ind < mStride * Height()) {
      const_cast<DataT*>(mExternalSource)[ind] = value;
    }
  }

public:
  ImageSrc(const BlockScene& _BlockScene)
    : mBlockScene(_BlockScene)
    , mExternalSource(nullptr)
  {
  }
};

template<typename DataT>
class BlockSrc {
  const BlockScene& mBlockScene;

  QVector<DataT> mInternalSource;
  const DataT*   mExternalSource;
  int            mStride;

  mutable DataT* mData;

public:
  int Width()  const { return mBlockScene.BlockWidth(); }
  int Height() const { return mBlockScene.BlockHeight(); }
  int Stride() const { return mStride; }
  int Size()   const { return mBlockScene.BlockHeight() * mStride; }

private:
  void SetStride(int _Stride) { mStride = _Stride; }
  void SetSource(const DataT* _ImageSource) { mExternalSource = _ImageSource; }

public:
  void ZeroSource()
  {
    memset(const_cast<DataT*>(mExternalSource), 0, Width() * Height() * sizeof(DataT));
  }

  void InitSource()
  {
    InitVector(mInternalSource, mBlockScene.BlockWidth() * mBlockScene.BlockHeight());
    SetSource(mInternalSource.constData());
    SetStride(mBlockScene.BlockWidth());
  }

  void PrepareSource()
  {
    mInternalSource.resize(mBlockScene.BlockWidth() * mBlockScene.BlockHeight());
    SetSource(mInternalSource.constData());
    SetStride(mBlockScene.BlockWidth());
  }

  void LineZero(int blockLine)
  {
    memset(Line(blockLine), 0, mStride * sizeof(DataT));
  }

  const DataT* Line(int blockLine) const
  {
    Q_ASSERT(blockLine < Height());

    mData = const_cast<DataT*>(&mExternalSource[blockLine * mStride]);
    return mData;
  }

  DataT* Line(int blockLine)
  {
    Q_ASSERT(blockLine < Height());

    mData = const_cast<DataT*>(&mExternalSource[blockLine * mStride]);
    return mData;
  }

  const DataT* BlockLine(int line) const
  {
    int blockLine = line / mBlockScene.BlockSize();
    Q_ASSERT(blockLine < Height());

    mData = const_cast<DataT*>(&mExternalSource[blockLine * mStride]);
    return mData;
  }

  DataT* BlockLine(int line)
  {
    int blockLine = line / mBlockScene.BlockSize();
    Q_ASSERT(blockLine < Height());

    mData = const_cast<DataT*>(&mExternalSource[blockLine * mStride]);
    return mData;
  }

  const DataT* Data(int i, int j) const
  {
    return const_cast<DataT*>(&mExternalSource[j * mStride + i]);
  }

  DataT* Data(int i, int j)
  {
    return const_cast<DataT*>(&mExternalSource[j * mStride + i]);
  }

  const DataT* BlockData(int i, int j) const
  {
    return const_cast<DataT*>(&mExternalSource[j / mBlockScene.BlockSize() * mStride + i / mBlockScene.BlockSize()]);
  }

  DataT* BlockData(int i, int j)
  {
    return const_cast<DataT*>(&mExternalSource[j / mBlockScene.BlockSize() * mStride + i / mBlockScene.BlockSize()]);
  }

  void operator++() const { mData++; }
  void operator++(int) const { mData++; }
  const DataT& operator*() const { return *mData; }
  DataT& operator*() { return *mData; }

  DataT CalcMaxValue() const
  {
    DataT maxValue = 0;
    for (int jj = 0; jj < Height(); jj++) {
      const DataT* src = Line(jj);
      for (int ii = 0; ii < Width(); ii++) {
        maxValue = qMax(maxValue, *src);
        src++;
      }
    }
    return maxValue;
  }

public:
  BlockSrc(const BlockScene& _BlockScene)
    : mBlockScene(_BlockScene)
    , mExternalSource(nullptr)
    , mData(nullptr)
  {
  }
};
