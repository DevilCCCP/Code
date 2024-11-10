#include <QtMath>
#include <QDebug>
#include <QRandomGenerator>

#include <Lib/Common/DMath.h>

#include "Plate.h"


const qreal kPartDiffMin = 0.8;
const qreal kPartDiffMax = 1.2;
const int kCutDeformCount = 7;
const qreal kCutDeformValue = 0.05;

void Plate::Init(const QPointF& _Center, const QVector<QPointF>& _Border)
{
  mCenter   = _Center;
  mBorder   = _Border;
  mSquare   = -1;
  mPositive = true;
}

void RotatePoint(qreal radius, qreal angle, QPointF& p)
{
  qreal alpha = p.y() / radius;
  qreal d = radius * qCos(alpha);
  p.rx() += angle * d;
}

void Plate::Rotate(qreal radius, qreal angle)
{
  RotatePoint(radius, angle, mCenter);
  for (QPointF& p: mBorder) {
    RotatePoint(radius, angle, p);
  }
}

qreal Plate::Square() const
{
  if (mSquare >= 0) {
    return mSquare;
  }

  mSquare = 0;
  if (mBorder.size() < 3) {
    return mSquare;
  }

  for (int i = 0; i < mBorder.size(); i++) {
    const QPointF& a = mBorder.at(i);
    const QPointF& b = mBorder.at(i + 1 < mBorder.size()? i + 1: 0);
    qreal s = a.x() * b.y() - a.y() * b.x();
    mSquare += s;
  }
  mSquare *= 0.5;
  mPositive = mSquare >= 0;
  mSquare = qAbs(mSquare);

  return mSquare;
}

qreal Plate::Distance(const Plate& other) const
{
  const int kPointStep = 10;

  if (mBorder.isEmpty() || other.Border().isEmpty()) {
    return 0;
  }

  QPointF d = Border().first() - other.Border().first();
  qreal distance = d.manhattanLength();
  int bestI = 0;
  int bestJ = 0;
  for (int i = 0; i < Border().size(); i += kPointStep) {
    for (int j = 0; j < other.Border().size(); j += kPointStep) {
      QPointF d = Border().at(i) - other.Border().at(j);
      qreal dist = d.manhattanLength();
      if (dist < distance) {
        bestI = i;
        bestJ = j;
        distance = dist;
      }
    }
  }

  for (int i = bestI - kPointStep + 1; i < bestI + kPointStep; i++) {
    int ii = i;
    if (ii < 0) {
      ii += Border().size();
    } else if (ii >= Border().size()) {
      ii -= Border().size();
    }
    for (int j = bestJ - kPointStep + 1; j < bestJ + kPointStep; j++) {
      int jj = j;
      if (jj < 0) {
        jj += other.Border().size();
      } else if (jj >= other.Border().size()) {
        jj -= other.Border().size();
      }
      QPointF d = Border().at(ii) - other.Border().at(jj);
      qreal dist = d.manhattanLength();
      distance = qMin(distance, dist);
    }
  }
  return distance;
}

bool Plate::IsPositiveSquare() const
{
  Square();
  return mPositive;
}

bool Plate::TestSelfIntersect() const
{
  for (int i = 0; i < mBorder.size(); i++) {
    int i2 = i + 1 < mBorder.size()? i + 1: 0;
    const QPointF& a1 = mBorder.at(i);
    const QPointF& a2 = mBorder.at(i2);
    for (int j = 0; j < mBorder.size(); j++) {
      int j2 = j + 1 < mBorder.size()? j + 1: 0;
      if (i == j || i == j2 || i2 == j || i2 == j2) {
        continue;
      }
      const QPointF& b1 = mBorder.at(j);
      const QPointF& b2 = mBorder.at(j2);
      if (qIntersect(a1, a2, b1, b2)) {
        return false;
      }
    }
  }
  return true;
}

bool Plate::IsIntersect(const Plate& other) const
{
  for (int i = 0; i < mBorder.size(); i++) {
    int i2 = i + 1 < mBorder.size()? i + 1: 0;
    const QPointF& a1 = mBorder.at(i);
    const QPointF& a2 = mBorder.at(i2);
    for (int j = 0; j < other.mBorder.size(); j++) {
      int j2 = j + 1 < other.mBorder.size()? j + 1: 0;
      const QPointF& b1 = other.mBorder.at(j);
      const QPointF& b2 = other.mBorder.at(j2);
      if (qIntersect(a1, a2, b1, b2)) {
        return true;
      }
    }
  }
  return false;
}

void Plate::CalcCenter()
{
  if (mBorder.size() <= 0) {
    return;
  }

  mCenter = QPointF(0, 0);
  qreal weight = 0;
  for (int i = 0; i < mBorder.size() - 1; i++) {
    const QPointF& p1 = mBorder.at(i);
    const QPointF& p2 = mBorder.at(i + 1);
    QPointF v = p2 - p1;
    qreal w = v.manhattanLength();
    mCenter += p1 * w;
    weight += w;
  }
  if (weight > 1) {
    mCenter /= weight;
  }
}

void Plate::AddMove(const QPointF& vect)
{
  mMoveHistory.append(vect);
  mMoved += vect;
}

void Plate::Move()
{
   for (int i = 0; i < mBorder.size(); i++) {
     mBorder[i] += mMoved;
   }
   mMoved = QPointF(0, 0);
}

void Plate::Move(const QPointF& vect)
{
  AddMove(vect);
  Move();
}

void Plate::DeformPlate(QRandomGenerator* rand, qreal disp, int count)
{
  for (int i = 0; i < count; i++) {
    Randomize(rand, disp);
  }
  for (int i = 0; i < count/2; i++) {
    Randomize(rand, 0.5 * disp);
  }
  for (int i = 0; i < count/2; i++) {
    Randomize(rand, 0.25 * disp);
  }
}

void Plate::Randomize(QRandomGenerator* rand, qreal value)
{
  qreal rndLen = rand->bounded(20, 101) * value * 0.01;
  qreal rndAlpha = rand->bounded(180) * M_PI / 90.0;
  QPointF rndIdent(qSin(rndAlpha), qCos(rndAlpha));
  int pointIndex = rand->bounded(mBorder.size());

  MovePoint(pointIndex, rndIdent, rndLen);
}

void Plate::MovePoint(int index, const QPointF& vectIdent, qreal len)
{
  QPointF basePoint = mBorder[index];
  mBorder[index] += vectIdent * len;
  qreal maxLen2 = 3 * len * len;

  for (int i = NextBoarderPoint(index); ; i = NextBoarderPoint(i)) {
    if (i == index) {
      return;
    }
    QPointF l = basePoint - mBorder.at(i);
    qreal len2 = l.x() * l.x() + l.y() * l.y();
    if (len2 > maxLen2) {
      break;
    }
    qreal k = (maxLen2 - len2) / maxLen2;
    mBorder[i] += vectIdent * (k * k * len);
  }

  for (int i = PrevBoarderPoint(index); ; i = PrevBoarderPoint(i)) {
    if (i == index) {
      return;
    }
    QPointF l = basePoint - mBorder.at(i);
    qreal len2 = l.x() * l.x() + l.y() * l.y();
    if (len2 > maxLen2) {
      break;
    }
    qreal k = (maxLen2 - len2) / maxLen2;
    mBorder[i] += vectIdent * (k * k * len);
  }
}

bool Plate::CreateCut(QRandomGenerator* rand, int i1, int i2, int count, Plate* plate1, Plate* plate2, int& count1, int& count2) const
{
  if (i1 > i2) {
    qSwap(i1, i2);
  }
  const QPointF& p1 = Border().at(i1);
  const QPointF& p2 = Border().at(i2);

  QPointF v = p2 - p1;
  qreal distance = qSqrt(v.x() * v.x() + v.y() * v.y());
  if (distance < 1.0) {
    return false;
  }

  for (int i = 0; i < Border().size(); i++) {
    int in = i + 1;
    if (in >= Border().size()) {
      in -= Border().size();
    }
    if (i == i1 || i == i2 || in == i1 || in == i2) {
      continue;
    }

    const QPointF& b1 = Border().at(i);
    const QPointF& b2 = Border().at(in);
    qreal t2 = qIntersectB(p1, p2, b1, b2);
    if (t2 < 0 || t2 > 1) {
      continue;
    }
    qreal t1 = qIntersectA(p1, p2, b1, b2);
    if (t1 < 0 || t1 > 1) {
      continue;
    }
    return false;
  }

  v *= 1.0 / distance;
  QVector<QPointF> cut;
  for (qreal t = 1.0; t < distance - 0.5; t += 1.0) {
    QPointF p = p1 + t * v;
    cut.append(p);
  }

  for (int i = 0; i < 10; i++) {
    QVector<QPointF> deformedCut;
    if (DeformCut(rand, cut, &deformedCut)) {
      cut = deformedCut;
      break;
    }
  }

  QVector<QPointF> border1;
  border1.append(Border().mid(0, i1 + 1));
  border1.append(cut);
  border1.append(Border().mid(i2));
  QVector<QPointF> border2;
  border2.append(Border().mid(i1, i2 - i1 + 1));
  std::reverse(cut.begin(), cut.end());
  border2.append(cut);
  plate1->Init(Center(), border1);
  plate2->Init(Center(), border2);

  if (plate1->IsPositiveSquare() != plate2->IsPositiveSquare()) {
    return false;
  }
//  qDebug() << plate1->Square() << plate1->IsPositiveSquare() << plate2->Square() << plate2->IsPositiveSquare() << plate1->Square() + plate2->Square() << Square();
//  if (plate1->Square() + plate2->Square() > Square()) {
//  }
  qreal s1 = plate1->Square();
  qreal s2 = plate2->Square();
  qreal s = s1 + s2;
  qreal s1p = s1 / s * count;
  qreal s2p = s2 / s * count;
  count1 = qRound(s1p);
  count2 = qRound(s2p);
  if (count1 + count2 != count) {
    return false;
  }

  if (!(s1p * kPartDiffMin <= count1 && count1 <= s1p * kPartDiffMax)
      || !(s2p * kPartDiffMin <= count2 && count2 <= s2p * kPartDiffMax)) {
    return false;
  }

  return true;
}

bool Plate::DeformCut(QRandomGenerator* rand, const QVector<QPointF>& cut, QVector<QPointF>* deformedCut) const
{
  if (cut.size() <= 2) {
    return false;
  }
  QPointF v = cut.last() - cut.first();
  qreal distance = qSqrt(v.x() * v.x() + v.y() * v.y());
  Plate cutPlate;
  cutPlate.Init(QPointF(0, 0), cut);
  for (int i = 0; i < kCutDeformCount; i++) {
    qreal rndLen = rand->bounded(20, 101) * distance * kCutDeformValue * 0.01;
    qreal rndAlpha = rand->bounded(180) * M_PI / 90.0;
    QPointF rndIdent(qSin(rndAlpha), qCos(rndAlpha));
    int minIndex = (int)rndLen + 1;
    if (minIndex > cutPlate.Border().size()/4) {
      continue;
    }

    int pointIndex = minIndex + rand->bounded(cutPlate.Border().size() - 2*minIndex);
    cutPlate.MovePoint(pointIndex, rndIdent, rndLen);
  }

  if (cutPlate.TestSelfIntersect()) {
//    qDebug() << "Cut self intersect";
    return false;
  }

  *deformedCut = cutPlate.Border();
  for (int i = 0; i < mBorder.size(); i++) {
    int i2 = i + 1 < mBorder.size()? i + 1: 0;
    const QPointF& a1 = mBorder.at(i);
    const QPointF& a2 = mBorder.at(i2);
    for (int j = 0; j < deformedCut->size() - 1; j++) {
      const QPointF& b1 = deformedCut->at(j);
      const QPointF& b2 = deformedCut->at(j + 1);
      if (qIntersect(a1, a2, b1, b2)) {
//        qDebug() << "Cut intersect plate";
        return false;
      }
    }
  }

  return true;
}

int Plate::NextBoarderPoint(int index)
{
  ++index;
  if (index >= mBorder.size()) {
    index -= mBorder.size();
  }
  return index;
}

int Plate::PrevBoarderPoint(int index)
{
  --index;
  if (index < 0) {
    index += mBorder.size();
  }
  return index;
}


Plate::Plate()
  : mCenter(0, 0), mMoved(0, 0), mSquare(-1), mPositive(true)
{
}
