#include <QPainter>
#include <QMouseEvent>

#include "FormImageLineRegion.h"


const int kPointNear = 6;

void FormImageLineRegion::paintEvent(QPaintEvent* event)
{
  FormImageRegion::paintEvent(event);

  if (mMode == FormImageLineView::eSelect) {
    return;
  }

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  if (mMode == FormImageLineView::eLine) {
    for (int i = 0; i < mLinePoints.size() - 1; i++) {
      QPointF p1 = ToScreen(mLinePoints.at(i));
      QPointF p2 = ToScreen(mLinePoints.at(i + 1));
      QLinearGradient gradient(p1, p2);
      gradient.setColorAt(0, QColor(71, 212, 255));
      gradient.setColorAt(1, QColor(27, 158, 198));

      QPen pen(QBrush(gradient), 3);
      painter.setPen(pen);
      painter.drawLine(p1, p2);
    }
  }

  if (mMode == FormImageLineView::eRectangle && mLinePoints.size() == 4) {
    QPointF p1 = ToScreen(mLinePoints.at(0));
    QPointF p2 = ToScreen(mLinePoints.at(3));
    QLinearGradient gradient(p1, p2);
    gradient.setColorAt(0, QColor(71, 212, 255, 100));
    gradient.setColorAt(1, QColor(27, 158, 198, 100));
    painter.fillRect(QRectF(p1, p2), QBrush(gradient));
  }

  for (int i = 0; i < mLinePoints.size(); i++) {
    QPointF p1 = ToScreen(mLinePoints.at(i));
    QPen pen(QBrush(QColor(19, 121, 121)), i == mCurrentPoint? 9: 6);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);

    painter.drawPoint(p1);
  }
}

//void FormImageLineRegion::mouseDoubleClickEvent(QMouseEvent* event)
//{
//  SetPoint(FromScreen(QPointF(event->pos())));
//}

void FormImageLineRegion::mouseMoveEvent(QMouseEvent* event)
{
  if (mMousePressPos != event->pos()) {
    mMousePressPos = QPoint();
  }

  if (mCurrentPoint >= 0 && mCurrentPoint < mLinePoints.size()) {
    mLinePoints[mCurrentPoint] = FromScreen(event->pos());
    if (mMode == FormImageLineView::eLine) {
      if (event->modifiers() == Qt::ShiftModifier) {
        mLinePoints[mCurrentPoint] = QPointF(mLinePoints[mCurrentPoint].toPoint());
        int vsPoint = 1 - mCurrentPoint;
        if (vsPoint >= 0 && vsPoint < mLinePoints.size()) {
          mLinePoints[vsPoint] = QPointF(mLinePoints[vsPoint].toPoint());
          mLinePoints[vsPoint].setY(mLinePoints[mCurrentPoint].y());
        }
      }
    } else if (mMode == FormImageLineView::eRectangle) {
      if (mLinePoints.size() == 4) {
        switch (mCurrentPoint) {
        case 0: mLinePoints[1].setY(mLinePoints[0].y()); mLinePoints[2].setX(mLinePoints[0].x()); break;
        case 1: mLinePoints[0].setY(mLinePoints[1].y()); mLinePoints[3].setX(mLinePoints[1].x()); break;
        case 2: mLinePoints[3].setY(mLinePoints[2].y()); mLinePoints[0].setX(mLinePoints[2].x()); break;
        case 3: mLinePoints[2].setY(mLinePoints[3].y()); mLinePoints[1].setX(mLinePoints[3].x()); break;
        }
      }
    }
    event->accept();

    update();
    emit LineChanged();
    return;
  }

  FormImageRegion::mouseMoveEvent(event);
}

void FormImageLineRegion::mousePressEvent(QMouseEvent* event)
{
  mMousePressPos = event->pos();
  if (event->buttons() == Qt::LeftButton) {
    for (int i = 0; i < mLinePoints.size(); i++) {
      QPointF p1 = ToScreen(mLinePoints.at(i));
      if ((p1 - event->pos()).manhattanLength() < kPointNear) {
        mCurrentPoint = i;
        event->accept();

        update();
        emit LineChanged();
        return;
      }
    }
  }

  if (mCurrentPoint != -1) {
    mCurrentPoint = -1;
    update();
  }

  FormImageRegion::mousePressEvent(event);
}

void FormImageLineRegion::mouseReleaseEvent(QMouseEvent* event)
{
  if (mCurrentPoint != -1) {
    mCurrentPoint = -1;
    update();

    emit LineChanged();
  } else if (mMousePressPos == event->pos()) {
    SetPoint(FromScreen(QPointF(event->pos())));
  }

  FormImageRegion::mouseReleaseEvent(event);
}

void FormImageLineRegion::SetNone()
{
  mMode = FormImageLineView::eSelect;
}

void FormImageLineRegion::SetLine(const QVector<QPointF>& linePoints)
{
  mMode = FormImageLineView::eLine;

  mLinePoints   = linePoints;
  mCurrentPoint = -1;

  update();
}

void FormImageLineRegion::SetRegion(const QVector<QPointF>& linePoints)
{
  mMode = FormImageLineView::eRectangle;

  mLinePoints   = linePoints;
  mCurrentPoint = -1;

  update();
}

const QVector<QPoint> FormImageLineRegion::LinePoints() const
{
  QVector<QPoint> values;
  foreach (const QPointF& p, mLinePoints) {
    values.append(ToSource(p));
  }

  return values;
}

const QList<uchar> FormImageLineRegion::LineValues() const
{
  QList<uchar> values;
  for (int i = 0; i < mLinePoints.size() - 1; i++) {
    QPoint p1 = ToSource(mLinePoints.at(i));
    QPoint p2 = ToSource(mLinePoints.at(i + 1));
    int w = p2.x() - p1.x();
    int h = p2.y() - p1.y();

    if (w == 0 && h == 0) {
      continue;
    } else if (qAbs(w) > qAbs(h)) {
      int d = (p2.x() > p1.x())? 1: -1;
      qreal k = ((qreal)h) / w;
      for (int i = p1.x(); i != p2.x(); i += d) {
        int dx = i - p1.x();
        int dy = (int)(dx * k + 0.5);
        int j = p1.y() + dy;
        QColor color = QColor::fromRgb(getImage().pixel(QPoint(i, j)));
        values.append(color.value());
      }
    } else {
      int d = (p2.y() > p1.y())? 1: -1;
      qreal k = ((qreal)w) / h;
      for (int j = p1.y(); j != p2.y(); j += d) {
        int dy = j - p1.y();
        int dx = (int)(dy * k + 0.5);
        int i = p1.x() + dx;
        QColor color = QColor::fromRgb(getImage().pixel(QPoint(i, j)));
        values.append(color.value());
      }
    }
  }
  return values;
}

const QVector<uchar> FormImageLineRegion::RectValues() const
{
  QPoint pointTopLeft(width() - 1, height() - 1);
  QPoint pointRightBottom(0, 0);
  for (int i = 0; i < mLinePoints.size() - 1; i++) {
    QPoint p = ToSource(mLinePoints.at(i));
    pointTopLeft.rx() = qMin(pointTopLeft.x(), p.x());
    pointTopLeft.ry() = qMin(pointTopLeft.y(), p.y());
    pointRightBottom.rx() = qMax(pointRightBottom.x(), p.x());
    pointRightBottom.ry() = qMax(pointRightBottom.y(), p.y());
  }
  pointRightBottom.rx() = qMin(pointRightBottom.x(), width() - 1);
  pointRightBottom.ry() = qMin(pointRightBottom.y(), height() - 1);
  QSize size(pointRightBottom.x() - pointTopLeft.x() + 1, pointRightBottom.y() - pointTopLeft.y() + 1);

  QVector<uchar> values;
  if (size.isEmpty()) {
    return values;
  }
  values.reserve(size.width() * size.height());
  for (int j = pointTopLeft.y(); j <= pointRightBottom.y(); j++) {
    for (int i = pointTopLeft.x(); i <= pointRightBottom.x(); i++) {
      QColor color = QColor::fromRgb(getImage().pixel(QPoint(i, j)));
      values.append(color.value());
    }
  }
  return values;
}

void FormImageLineRegion::MoveLineX(int x)
{
  if (mLinePoints.size() < 2) {
    return;
  }

  int d = mLinePoints.at(1).x() - mLinePoints.at(0).x();
  mLinePoints[0].setX(x - 0.5*width()/getScale());
  mLinePoints[1].setX(x + d - 0.5*width()/getScale());
}

void FormImageLineRegion::MoveLineY(int y)
{
  if (mLinePoints.size() < 2) {
    return;
  }

  mLinePoints[0].setY(y - 0.5*height()/getScale());
  mLinePoints[1].setY(y - 0.5*height()/getScale());
}

void FormImageLineRegion::SetPoint(const QPointF& p)
{
  switch (mMode) {
  case FormImageLineView::eSelect:
    break;

  case FormImageLineView::eLine:
    if (mLinePoints.size() > 1) {
      mLinePoints.first() = mLinePoints.takeLast();
    }
    mLinePoints.append(p);
    break;

  case FormImageLineView::eRectangle:
    if (mLinePoints.size() >= 1) {
      if (mLinePoints.size() > 1) {
        mLinePoints.first() = mLinePoints.takeLast();
        mLinePoints.remove(1, 2);
      }
      const QPointF& point1 = mLinePoints.first();
      const QPointF& point2 = p;
      mLinePoints.append(QPointF(point2.x(), point1.y()));
      mLinePoints.append(QPointF(point1.x(), point2.y()));
      mLinePoints.append(point2);
    } else {
      mLinePoints.append(p);
    }
    break;
  }

  update();
  emit LineChanged();
}

QPointF FormImageLineRegion::ToScreen(const QPointF& p) const
{
  return QPointF(0.5*width() + (p.x() + 0.5) * getScale(), 0.5*height() + (p.y() + 0.5) * getScale());
}

QPointF FormImageLineRegion::FromScreen(const QPointF& p) const
{
  return QPointF((p.x() - 0.5*width()) / getScale() - 0.5, (p.y() - 0.5*height()) / getScale() - 0.5);
}

QPoint FormImageLineRegion::ToSource(const QPointF& p) const
{
  return QPointF(0.5*width()/getScale() + p.x(), 0.5*height()/getScale() + p.y()).toPoint();
}

QPointF FormImageLineRegion::FromSource(const QPoint& p) const
{
  return QPointF(p.x() - 0.5*width()/getScale(), p.y() - 0.5*height()/getScale());
}


FormImageLineRegion::FormImageLineRegion(QWidget* parent)
  : FormImageRegion(parent)
  , mMode(FormImageLineView::eSelect), mCurrentPoint(-1)
{
}

