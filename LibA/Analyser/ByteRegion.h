#pragma once

#include <QVector>
#include <QRect>


class ByteRegion {
  QVector<uchar>     mInternalSource;
  uchar*             mData;
  int                mWidth;
  int                mHeight;
  int                mStride;

public:
  int  Width()  const { return mWidth; }
  int  Height() const { return mHeight; }
  int  Stride() const { return mStride; }
  bool IsValid() const { return mData && mWidth > 0 && mHeight > 0; }
  bool IsNull() const { return !IsValid(); }

public:
  void SetSource(uchar* _SourceData, int _Width, int _Height, int _Stride);
  void SetSource(const ByteRegion& other, int x, int y, int width, int height);
  void SetSource(const ByteRegion& other, const QRect& subRect);
  void SetSource(const ByteRegion& other);
  QRect Rect() const;
  QSize Size() const;
  void SetSize(const QSize& size);
  void SetSize(int _Width, int _Height);

public:
  void ZeroData();
  void FillData(int value);
  void SetData(int val);
  void Copy(const ByteRegion& source, const QRect& sourceRect);
  void Copy(const QPoint& destPoint, const ByteRegion& source, const QRect& sourceRect);
  void LineZero(int line);
  const uchar* Line(int line) const;
  uchar* Data(int i, int j) const;
  uchar* Data(const QPoint& p) const;
  uchar* Line(int line);
  void FillRectBorder(const QRect& rect, int color);
  void FillRect(const QRect& rect, int color);
  void FillLine(const QPoint& p1, const QPoint& p2, int color);
  int SumLine(const QPoint& p1, const QPoint& p2) const;

public:
  ByteRegion();
  ByteRegion(uchar* _SourceData, int _Width, int _Height, int _Stride);
  ByteRegion(const ByteRegion& other, int x, int y, int width, int height);
  ByteRegion(const ByteRegion& other, const QRect& subRect);
  ByteRegion(int _Width, int _Height);
  ByteRegion(const QSize& _Size);
  ByteRegion(const ByteRegion& other);
  ByteRegion& operator=(const ByteRegion& other);
};
