#include <QtMath>

#include "DMath.h"


qreal IntersectA(const QPointF& a1, const QPointF& a2, const QPointF& b1, const QPointF& b2)
{
  QPointF v1 = a2 - a1;
  QPointF v2 = b2 - b1;
  qreal num = b1.y() * v2.x() - b1.x() * v2.y() + a1.x() * v2.y() - a1.y() * v2.x();
  qreal denum = v2.x() * v1.y() - v1.x() * v2.y();
  if (denum == 0) {
    qreal p1, p2, q1, q2;
    if (v1.x() > v1.y()) {
      p1 = qMin(a1.x(), a2.x());
      p2 = qMax(a1.x(), a2.x());
      q1 = qMin(b1.x(), b2.x());
      q2 = qMax(b1.x(), b2.x());
    } else {
      p1 = qMin(a1.y(), a2.y());
      p2 = qMax(a1.y(), a2.y());
      q1 = qMin(b1.y(), b2.y());
      q2 = qMax(b1.y(), b2.y());
    }
    if (q1 < p1 && q2 > p2) {
      return 0;
    } else if (q1 > p2 || q2 < p1) {
      return -1;
    }
    if (v1.x() > v1.y()) {
      qreal t1 = (b1.x() - a1.x()) / v1.x();
      qreal t2 = (b2.x() - a1.x()) / v1.x();
      if (t1 < 0) {
        return t2;
      } else if (t2 < 0) {
        return t1;
      }
      return qMin(t1, t2);
    } else {
      qreal t1 = (b1.y() - a1.y()) / v1.y();
      qreal t2 = (b2.y() - a1.y()) / v1.y();
      if (t1 < 0) {
        return t2;
      } else if (t2 < 0) {
        return t1;
      }
      return qMin(t1, t2);
    }
  }
  return num / denum;
}
