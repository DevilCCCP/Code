#pragma once

#include <QPointF>
#include <QtMath>


inline qreal qSquare2(const QPointF& r)
{
  return r.x() * r.x() + r.y() * r.y();
}

inline qreal qSquare(const QPointF& r)
{
  return qSqrt(r.x() * r.x() + r.y() * r.y());
}

qreal IntersectA(const QPointF& a1, const QPointF& a2, const QPointF& b1, const QPointF& b2);

/// Returns t when intersect, -1 if not
inline qreal qIntersectA(const QPointF& a1, const QPointF& a2, const QPointF& b1, const QPointF& b2)
{ return IntersectA(a1, a2, b1, b2); }

/// Returns t when intersect, -1 if not
inline qreal qIntersectB(const QPointF& a1, const QPointF& a2, const QPointF& b1, const QPointF& b2)
{ return IntersectA(b1, b2, a1, a2); }

inline bool qIntersect(const QPointF& a1, const QPointF& a2, const QPointF& b1, const QPointF& b2)
{
  qreal t1 = qIntersectA(a1, a2, b1, b2);
  qreal t2 = qIntersectB(a1, a2, b1, b2);
  return t1 >= 0 && t1 <= 1 && t2 >= 0 && t2 <= 1;
}
