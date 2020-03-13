#include <QMouseEvent>
#include <QResizeEvent>
#include <QPainter>

#include <Lib/Settings/DbSettings.h>
#include <Lib/Log/Log.h>

#include "ImageWithPoints.h"


void ImageWithPoints::resizeEvent(QResizeEvent* event)
{
  mPointEq.setX(3.5 / event->size().width());
  mPointEq.setY(3.5 / event->size().height());
}

void ImageWithPoints::mouseMoveEvent(QMouseEvent* event)
{
  if (mPointsType != eEmpty && mSelIndex >= 0) {
    QPointF relPos = RelativePos(event->pos());
    mPoints[mSelIndex] = relPos;

    emit NeedSave();
    emit Redraw();
  }

  return QLabel::mouseMoveEvent(event);
}

void ImageWithPoints::mousePressEvent(QMouseEvent* event)
{
  if (mPointsType != eEmpty) {
    QPointF relPos = RelativePos(event->pos());

    if (mSourceImg.isNull()) {
      QImage image(width(), height(), QImage::Format_RGB32);
      image.fill(Qt::white);
      SetImage(image);
    }
    mSelIndex = -1;
    if ((event->buttons() & (Qt::LeftButton | Qt::RightButton)) == (Qt::LeftButton | Qt::RightButton)) {
      emit Redraw();
    } else if (event->buttons() & Qt::RightButton) {
      if (RemovePoint(relPos)) {
        emit NeedSave();
        emit Redraw();
      }
    } else if (event->buttons() & Qt::LeftButton) {
      if (AddPoint(relPos)) {
        emit NeedSave();
        emit Redraw();
      }
    }
  }

  return QLabel::mousePressEvent(event);
}

void ImageWithPoints::mouseReleaseEvent(QMouseEvent* event)
{
  if (mPointsType != eEmpty) {
    mSelIndex = -1;

    emit Redraw();
  }

  return QLabel::mouseReleaseEvent(event);
}

bool ImageWithPoints::SetPointsType(const Db& _Db, int objectId, EPointsType _PointsType)
{
  mPointSettings.reset(new DbSettings(_Db));
  mPointSettings->SetSilent(true);
  mPointsType = _PointsType;
  mObjectId = objectId;

  return LoadPoints(false);
}

void ImageWithPoints::SetImage(const QImage& image)
{
  mSourceImg = image;
  emit Redraw();
}

bool ImageWithPoints::LoadPoints(bool redraw)
{
  if (!mPointSettings->Open(QString::number(mObjectId))) {
    return false;
  }
  mPoints.clear();
  for (int i = 0; ; i++) {
    QString key = QString("Point #") + QString::number(i);
    QString value = mPointSettings->GetValue(key, "").toString();
    QStringList values = value.split(QChar(','), QString::SkipEmptyParts);
    if (values.size() == 2) {
      bool ok1, ok2;
      qreal px = values[0].toFloat(&ok1);
      qreal py = values[1].toFloat(&ok2);
      if (ok1 && ok2) {
        mPoints.append(QPointF(px, py));
      }
    } else {
      break;
    }
  }

  if (redraw) {
    emit Redraw();
  }

  return true;
}

void ImageWithPoints::SavePoints()
{
  if (mPointsType == eEmpty) {
    return;
  }

  mPointSettings->Clean();
  int size = mPoints.size();
  for (int i = 0; i < size; i++) {
    QString key = QString("Point #") + QString::number(i);
    mPointSettings->SetValue(key, QString("%1,%2").arg(mPoints[i].x()).arg(mPoints[i].y()));
  }
  mPointSettings->Sync();
}

bool ImageWithPoints::RemovePoint(const QPointF& relPos)
{
  for (int i = 0; i < mPoints.size(); i++) {
    if (IsNear(mPoints[i], relPos)) {
      mPoints.removeAt(i);
      return true;
    }
  }
  return false;
}

bool ImageWithPoints::AddPoint(const QPointF& relPos)
{
  if (mPoints.isEmpty()) {
    mPoints.append(relPos);
    return true;
  }

  for (int i = 0; i < mPoints.size(); i++) {
    if (IsNear(mPoints[i], relPos)) {
      mSelIndex = i;
      return true;
    }
  }

  QPointF lastPoint = (IsTypeArea())? mPoints.last(): mPoints.first();
  for (int i = 0; i < mPoints.size(); lastPoint = mPoints[i], i++) {
    QPointF curPoint = mPoints[i];

    QPointF v = curPoint - lastPoint;
    if (IsZero(v)) {
      continue;
    } else if (qAbs(v.x()) >= qAbs(v.y())) {
      qreal t = (relPos.x() - curPoint.x()) / (lastPoint.x() - curPoint.x());
      qreal relY2 = (lastPoint.y() - curPoint.y()) * t + curPoint.y();
      if (qAbs(relPos.y() - relY2) > mPointEq.y()) {
        continue;
      }
    } else {
      qreal t = (relPos.y() - curPoint.y()) / (lastPoint.y() - curPoint.y());
      qreal relX2 = (lastPoint.x() - curPoint.x()) * t + curPoint.x();
      if (qAbs(relPos.x() - relX2) > mPointEq.x()) {
        continue;
      }
    }

    mPoints.insert(i, relPos);
    mSelIndex = i;
    return true;
  }

  mSelIndex = mPoints.size();
  mPoints.append(relPos);
  return true;
}

QPointF ImageWithPoints::RelativePos(const QPoint& point)
{
  int x = qMin(qMax(0, point.x()), width() - 1);
  int y = qMin(qMax(0, point.y()), height() - 1);
  return QPointF((qreal)x / (width() - 1), (qreal)y / (height() - 1));
}

QPoint ImageWithPoints::AbsolutPos(const QPointF& point)
{
  return QPoint(point.x() * mSourceImg.width(), point.y() * mSourceImg.height());
}

bool ImageWithPoints::IsNear(const QPointF& point1, const QPointF& point2)
{
  return qAbs(point1.x() - point2.x()) < mPointEq.x() && qAbs(point1.y() - point2.y()) < mPointEq.y();
}

bool ImageWithPoints::IsZero(const QPointF& point)
{
  return qAbs(point.x()) < mPointEq.x() && qAbs(point.y()) < mPointEq.y();
}

void ImageWithPoints::Redraw()
{
  if (!mDrawTimer.isActive()) {
    mDrawTimer.start(20);
  }
}

void ImageWithPoints::DrawImpl()
{
  if (mSourceImg.isNull()) {
    return;
  }

  QPixmap pixmap = QPixmap::fromImage(mSourceImg);
  if (mPoints.isEmpty()) {
    setPixmap(pixmap);
    return;
  }

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);

  if (IsTypeArea()) {
    DrawArea(&painter);
    DrawLine(&painter);
  } else if (IsTypeLine()) {
    DrawLine(&painter);
  } else if (IsTypeLineZone()) {
    DrawLineZone(&painter);
  }
  DrawPoints(&painter);

  setPixmap(pixmap);
}

void ImageWithPoints::DrawArea(QPainter* painter)
{
  if (mPoints.size() < 3) {
    return;
  }

  QVector<QPoint> vect(mPoints.size());
  for (int i = 0; i < mPoints.size(); i++) {
    vect[i] = AbsolutPos(mPoints[i]);
  }
  if (mPointsType != eAreaIgnore) {
    painter->setBrush(QBrush(QColor(129, 198, 83, 100)));
  } else {
    painter->setBrush(QBrush(QColor(178, 177, 198, 200)));
  }
  painter->drawPolygon(vect.constData(), vect.size());
}

void ImageWithPoints::DrawLineZone(QPainter* painter)
{
  if (mPoints.size() < 3) {
    return;
  }

  QPointF basePoint = mPoints[0];
  QPointF leftPoint = mPoints[1];
  QPointF rightPoint = mPoints[2];
  QPointF midPoint = (leftPoint + rightPoint) * 0.5;
  leftPoint += basePoint - midPoint;
  rightPoint += basePoint - midPoint;
  QVector<QPoint> vect(mPoints.size() + 1);
  QVector<QPoint> leftVect((mPoints.size())/2 + 2);
  QVector<QPoint> rightVect((mPoints.size() - 1)/2 + 2);
  vect[0] = AbsolutPos(leftPoint);
  leftVect[0] = rightVect[0] = AbsolutPos(basePoint);
  leftVect[1] = AbsolutPos(leftPoint);
  rightVect[1] = AbsolutPos(rightPoint);
  for (int i = 1; i < mPoints.size(); i += 2) {
    vect[i/2 + 1] = AbsolutPos(mPoints[i]);
    leftVect[i/2 + 2] = AbsolutPos(mPoints[i]);
  }
  for (int i = 2; i < mPoints.size(); i += 2) {
    vect[vect.size() - i/2 - 1] = AbsolutPos(mPoints[i]);
    rightVect[i/2 + 1] = AbsolutPos(mPoints[i]);
  }
  vect[vect.size() - 1] = AbsolutPos(rightPoint);
  painter->setBrush(QBrush(QColor(200, 150, 0, 200)));
  painter->drawPolygon(vect.constData(), vect.size());


  for (int i = 1; i < leftVect.size(); i++) {
    QPoint a = leftVect[i-1];
    QPoint b = leftVect[i];

    QLinearGradient gradient(a, b);
    gradient.setColorAt(0, QColor(71, 212, 255));
    gradient.setColorAt(1, QColor(27, 158, 198));

    QPen pen(QBrush(gradient), 3);
    painter->setPen(pen);
    painter->drawLine(a, b);
  }
  for (int i = 1; i < rightVect.size(); i++) {
    QPoint a = rightVect[i-1];
    QPoint b = rightVect[i];

    QLinearGradient gradient(a, b);
    gradient.setColorAt(0, QColor(71, 212, 255));
    gradient.setColorAt(1, QColor(27, 158, 198));

    QPen pen(QBrush(gradient), 3);
    painter->setPen(pen);
    painter->drawLine(a, b);
  }
}

void ImageWithPoints::DrawLine(QPainter* painter)
{
  QPointF lastPoint = (IsTypeArea())? mPoints.last(): mPoints.first();
  for (int i = (IsTypeArea())? 0: 1; i < mPoints.size(); lastPoint = mPoints[i], i++) {
    QPointF curPoint = mPoints[i];
    QPoint a = AbsolutPos(lastPoint);
    QPoint b = AbsolutPos(curPoint);

    QLinearGradient gradient(a, b);
    gradient.setColorAt(0, QColor(71, 212, 255));
    gradient.setColorAt(1, QColor(27, 158, 198));

    QPen pen(QBrush(gradient), 3);
    painter->setPen(pen);
    painter->drawLine(a, b);

    if (mPointsType == eLineWithOrder) {
      QPoint c = (a + b) / 2;
      qreal angl = atan2((qreal)(b.y() - a.y()), (qreal)(b.x() - a.x()));
      qreal pi2 = acos(0.0);
      angl -= pi2;
      if (angl < -4*pi2) {
        angl += 4*pi2;
      }
      QPoint d = c + QPoint(20 * cos(angl), 20 * sin(angl));
      QPoint d1 = d - QPoint(5 * cos(angl - 0.5), 5 * sin(angl - 0.5));
      QPoint d2 = d - QPoint(5 * cos(angl + 0.5), 5 * sin(angl + 0.5));

      QLinearGradient gradient(c, d);
      gradient.setColorAt(0, QColor(38, 219, 110));
      gradient.setColorAt(1, QColor(61, 219, 110));

      QPen pen(QBrush(gradient), 3);
      painter->setPen(pen);
      painter->drawLine(d, d1);
      painter->drawLine(d, d2);
      painter->drawLine(c, d);
    }
  }
}

void ImageWithPoints::DrawPoints(QPainter* painter)
{
  QPen pen(QBrush(QColor(19, 121, 121)), 6);
  pen.setCapStyle(Qt::RoundCap);
  painter->setPen(pen);

  for (int i = 0; i < mPoints.size(); i++) {
    painter->drawPoint(AbsolutPos(mPoints[i]));
  }
}

void ImageWithPoints::Draw()
{
  DrawImpl();
}


ImageWithPoints::ImageWithPoints(QWidget *parent)
  : QLabel(parent)
  , mPointsType(eEmpty)
  , mSelIndex(-1)
{
  mDrawTimer.setSingleShot(true);
  connect(&mDrawTimer, SIGNAL(timeout()), this, SLOT(Draw()), Qt::QueuedConnection);
}

