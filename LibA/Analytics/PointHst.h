#pragma once

#include <QPair>
#include <QPoint>
#include <QPointF>
#include <QLinkedList>


struct PointHst {
  typedef QPair<QPoint, int> Point;

  QLinkedList<Point> Points;
  int                Period;

  void Update(int time)
  {
    while (!Points.isEmpty() && time - Points.first().second > Period) {
      Points.removeFirst();
    }
  }

  void Append(const QPoint& point, int time)
  {
    Points.append(qMakePair(point, time));
  }

  PointHst(int _Period)
    : Period(_Period)
  { }
};

struct PointfHst {
  typedef QPair<QPointF, int> Point;

  QLinkedList<Point> Points;
  int                Period;

  void Update(int time)
  {
    while (!Points.isEmpty() && time - Points.first().second > Period) {
      Points.removeFirst();
    }
  }

  void Append(const QPointF& point, int time)
  {
    Points.append(qMakePair(point, time));
  }

  void Clear()
  {
    Points.clear();
  }

  PointfHst(int _Period)
    : Period(_Period)
  { }
};
