#pragma once

#include <QVector>
#include <QPoint>


class Plate
{
  QPointF          mCenter;
  QVector<QPointF> mBorder;
  QPointF          mMoved;
  QVector<QPointF> mMoveHistory;

  mutable qreal    mSquare;
  mutable bool     mPositive;

public:
  QPointF Center() const { return mCenter + mMoved; }
  bool IsEmpty() const { return mBorder.isEmpty(); }
  const QVector<QPointF>& Border() const { return mBorder; }
  QPointF Moved() const { return mMoved; }
  const QVector<QPointF>& MoveHistory() const { return mMoveHistory; }

public:
  void Init(const QPointF& _Center, const QVector<QPointF>& _Border);
  void Rotate(qreal radius, qreal angle);

  qreal Square() const;
  qreal Distance(const Plate& other) const;
  bool IsPositiveSquare() const;
  bool TestSelfIntersect() const;
  bool IsIntersect(const Plate& other) const;

  void CalcCenter();
  void AddMove(const QPointF& vect);
  void Move();
  void Move(const QPointF& vect);

  void DeformPlate(qreal disp, int count);
  void Randomize(qreal value);
  void MovePoint(int index, const QPointF& vectIdent, qreal len);
  bool CreateCut(int i1, int i2, int count, Plate* plate1, Plate* plate2, int& count1, int& count2) const;
  bool DeformCut(const QVector<QPointF>& cut, QVector<QPointF>* deformedCut) const;

  int NextBoarderPoint(int index);
  int PrevBoarderPoint(int index);

public:
  Plate();
};
