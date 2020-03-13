#pragma once
#include <QPointF>


inline int GetAlignedLeft(int value, int align)
{
  return value / align * align;
}

inline int GetAlignedRight(int value, int align)
{
  return (value + align - 1) / align * align;
}

template<typename TValue, typename TMaxValue>
bool NormalizeValue(TValue& value, TMaxValue maxValue)
{
  if (value < -maxValue) {
    value = -maxValue;
    return true;
  } else if (value > maxValue) {
    value = maxValue;
    return true;
  } else {
    return false;
  }
}

template<typename ContT>
void InitVector(QVector<ContT>& cont, int size)
{
  cont.resize(size);
  memset(cont.data(), 0, size * sizeof(ContT));
}

template<typename TypeT>
inline TypeT Radius2(TypeT x1, TypeT y1, TypeT x2, TypeT y2)
{
  return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

inline qreal Radius2(QPointF p1, QPointF p2)
{
  return (p1.x() - p2.x()) * (p1.x() - p2.x()) + (p1.y() - p2.y()) * (p1.y() - p2.y());
}

inline qreal Radius2(QPointF p)
{
  return (p.x()) * (p.x()) + (p.y()) * (p.y());
}

template<typename TypeT>
inline TypeT Radius(TypeT x1, TypeT y1, TypeT x2, TypeT y2)
{
  return sqrt(Radius2(x1, y1, x2, y2));
}

inline qreal Radius(QPointF p1, QPointF p2)
{
  return sqrt(Radius2(p1, p2));
}

inline qreal Radius(QPointF p)
{
  return sqrt(Radius2(p));
}

