#include "ByteRegion.h"


void ByteRegion::SetSource(uchar* _SourceData, int _Width, int _Height, int _Stride)
{
  mData     = _SourceData;
  mWidth    = _Width;
  mHeight   = _Height;
  mStride   = _Stride;
}

void ByteRegion::SetSource(const ByteRegion& other, int x, int y, int width, int height)
{
  mData     = other.Data(x, y);
  mStride   = other.mStride;
  mWidth    = width;
  mHeight   = height;
}

void ByteRegion::SetSource(const ByteRegion& other, const QRect& subRect)
{
  mData     = other.Data(subRect.x(), subRect.y());
  mStride   = other.mStride;
  mWidth    = subRect.width();
  mHeight   = subRect.height();
}

void ByteRegion::SetSource(const ByteRegion& other)
{
  mData     = other.mData;
  mWidth    = other.mWidth;
  mHeight   = other.mHeight;
  mStride   = other.mStride;
}

QRect ByteRegion::Rect() const
{
  return QRect(0, 0, mWidth, mHeight);
}

QSize ByteRegion::Size() const
{
  return QSize(mWidth, mHeight);
}

void ByteRegion::SetSize(const QSize& size)
{
  SetSize(size.width(), size.height());
}

void ByteRegion::SetSize(int _Width, int _Height)
{
  mWidth  = _Width;
  mHeight = _Height;
  mStride = mWidth;

  mInternalSource.resize(mWidth * mHeight);
  mData = mInternalSource.data();
}

void ByteRegion::ZeroData()
{
  if (mWidth == mStride) {
    memset(mData, 0, mWidth * mHeight * sizeof(uchar));
  } else {
    for (int j = 0; j < Height(); j++) {
      memset(Line(j), 0, mWidth * sizeof(uchar));
    }
  }
}

void ByteRegion::FillData(int value)
{
  if (mWidth == mStride) {
    memset(mData, value, mWidth * mHeight * sizeof(uchar));
  } else {
    for (int j = 0; j < Height(); j++) {
      memset(Line(j), value, mWidth * sizeof(uchar));
    }
  }
}

void ByteRegion::SetData(int val)
{
  if (mWidth == mStride) {
    memset(mData, val, mWidth * mHeight * sizeof(uchar));
  } else {
    for (int j = 0; j < Height(); j++) {
      memset(Line(j), val, mWidth * sizeof(uchar));
    }
  }
}

void ByteRegion::Copy(const ByteRegion& source, const QRect& sourceRect)
{
  for (int j = sourceRect.top(); j <= sourceRect.bottom(); j++) {
    const uchar* src = source.Data(sourceRect.left(), j);
    uchar* dst = Data(sourceRect.left(), j);
    memcpy(dst, src, sourceRect.width() * sizeof(uchar));
  }
}

void ByteRegion::Copy(const QPoint& destPoint, const ByteRegion& source, const QRect& sourceRect)
{
  if (destPoint.x() < 0 || destPoint.x() + sourceRect.width() >= Width()
      || destPoint.y() < 0 || destPoint.y() + sourceRect.height() >= Height()) {
    return;
  }

  for (int j = sourceRect.top(); j <= sourceRect.bottom(); j++) {
    const uchar* src = source.Data(sourceRect.left(), j);
    uchar* dst = Data(destPoint.x(), destPoint.y() + j - sourceRect.top());
    memcpy(dst, src, sourceRect.width() * sizeof(uchar));
  }
}

void ByteRegion::LineZero(int line)
{
  memset(Line(line), 0, mWidth * sizeof(uchar));
}

const uchar* ByteRegion::Line(int line) const
{
  Q_ASSERT(line >= 0 && line < Height());

  return &mData[line * mStride];
}

uchar* ByteRegion::Data(int i, int j) const
{
  Q_ASSERT(i >= 0 && i < Width());
  Q_ASSERT(j >= 0 && j < Height());

  return mData + (j * Stride() + i);
}

uchar* ByteRegion::Data(const QPoint& p) const
{
  Q_ASSERT(p.x() >= 0 && p.x() < Width());
  Q_ASSERT(p.y() >= 0 && p.y() < Height());

  return mData + (p.y() * Stride() + p.x());
}

uchar* ByteRegion::Line(int line)
{
  Q_ASSERT(line >= 0 && line < Height());

  return &mData[line * mStride];
}

void ByteRegion::FillRectBorder(const QRect& rect, int color)
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

void ByteRegion::FillRect(const QRect& rect, int color)
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

void ByteRegion::FillLine(const QPoint& p1, const QPoint& p2, int color)
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

int ByteRegion::SumLine(const QPoint& p1, const QPoint& p2) const
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

ByteRegion::ByteRegion()
  : mData(nullptr), mWidth(0), mHeight(0), mStride(0)
{
}

ByteRegion::ByteRegion(uchar* _SourceData, int _Width, int _Height, int _Stride)
{
  SetSource(_SourceData, _Width, _Height, _Stride);
}

ByteRegion::ByteRegion(const ByteRegion& other, int x, int y, int width, int height)
{
  SetSource(other, x, y, width, height);
}

ByteRegion::ByteRegion(const ByteRegion& other, const QRect& subRect)
{
  SetSource(other, subRect);
}

ByteRegion::ByteRegion(int _Width, int _Height)
{
  SetSize(_Width, _Height);
}

ByteRegion::ByteRegion(const QSize& _Size)
{
  SetSize(_Size);
}

ByteRegion::ByteRegion(const ByteRegion& other)
{
  SetSource(other);
  if (!other.mInternalSource.isEmpty()) {
    mInternalSource = other.mInternalSource;
    mData = mInternalSource.data();
  }
}

ByteRegion& ByteRegion::operator=(const ByteRegion& other)
{
  SetSource(other);
  if (!other.mInternalSource.isEmpty()) {
    mInternalSource = other.mInternalSource;
    mData = mInternalSource.data();
  }
  return *this;
}
