#include <QMouseEvent>
#include <QWheelEvent>
#include <QPixmap>
#include <QPainter>
#include <QColor>

#include "ImageLabel.h"


const int kMaxWidth = 4800;
const int kMaxHeight = 3600;
const int kMinWidth = 320;
const int kMinHeight = 240;

ImageLabel::ImageLabel(QWidget* parent)
  : QLabel(parent)
  , mCurrentPoint(-1)
{
}


const QList<uchar> ImageLabel::LineValues() const
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
        QColor color = QColor::fromRgb(mSourceImage.pixel(QPoint(i, j)));
        values.append(color.value());
      }
    } else {
      int d = (p2.y() > p1.y())? 1: -1;
      qreal k = ((qreal)w) / h;
      for (int j = p1.y(); j != p2.y(); j += d) {
        int dy = j - p1.y();
        int dx = (int)(dy * k + 0.5);
        int i = p1.x() + dx;
        QColor color = QColor::fromRgb(mSourceImage.pixel(QPoint(i, j)));
        values.append(color.value());
      }
    }
  }
  return values;
}

void ImageLabel::SyncLine(QSettings* settings) const
{
  for (int i = 0; i < mLinePoints.size(); i++) {
    settings->setValue(LineVarname(i), mLinePoints.at(i));
  }
  for (int i = mLinePoints.size(); !settings->value(LineVarname(i)).isNull(); i++) {
    settings->setValue(LineVarname(i), QVariant());
  }
  settings->sync();
}

void ImageLabel::RestoreLine(QSettings* settings)
{
  mLinePoints.clear();
  for (int i = 0; ; i++) {
    QVariant v = settings->value(LineVarname(i));
    if (v.isNull()) {
      break;
    }
    QPointF p = v.toPointF();
    mLinePoints.append(p);
  }
}

void ImageLabel::mouseMoveEvent(QMouseEvent* ev)
{
  if (mCurrentPoint >= 0 && mCurrentPoint < mLinePoints.size()) {
    mLinePoints[mCurrentPoint] = ToPercent(ev->pos());
    DrawImage();
  }
}

void ImageLabel::mousePressEvent(QMouseEvent* ev)
{
  if (ev->button() == Qt::LeftButton) {
    const QPoint& pos = ev->pos();
    for (int i = 0; i < mLinePoints.size(); i++) {
      QPoint r = ToScreen(mLinePoints.at(i)) - pos;
      if (r.manhattanLength() < 5) {
        mCurrentPoint = i;
        DrawImage();
        return;
      }
    }

    mLinePoints.clear();
    mLinePoints.append(ToPercent(pos));
    mLinePoints.append(ToPercent(pos));
    mCurrentPoint = 1;
    DrawImage();
  }
}

void ImageLabel::mouseReleaseEvent(QMouseEvent* ev)
{
  if (mCurrentPoint >= 0 && mCurrentPoint < mLinePoints.size()) {
    mLinePoints[mCurrentPoint] = ToPercent(ev->pos());
  }
  mCurrentPoint = -1;
  DrawImage();
}

void ImageLabel::wheelEvent(QWheelEvent* event)
{
  if (event->modifiers() & Qt::ControlModifier) {
    mScaleAngle += event->angleDelta().y();
    UpdateImage();
  } else {
    QLabel::wheelEvent(event);
  }
}

void ImageLabel::SetImage(const QImage& image)
{
  mSourceImage = image;
  mScaleAngle = 0;
  UpdateImage();
}

void ImageLabel::SetX(int value)
{
  mCenterPos.setX(value - mWidth/2);
}

void ImageLabel::SetY(int value)
{
  mCenterPos.setY(value - mHeight/2);
}

QString ImageLabel::LineVarname(int i) const
{
  return QString("Line %1").arg(i);
}

QPoint ImageLabel::ToScreen(const QPointF& p) const
{
  return QPoint(p.x() * mWidth, p.y() * mHeight);
}

QPoint ImageLabel::ToSource(const QPointF& p) const
{
  return QPoint(p.x() * mSourceImage.width(), p.y() * mSourceImage.height());
}

QPointF ImageLabel::ToPercent(const QPoint& p) const
{
  return QPointF(((qreal)p.x()) / mWidth, ((qreal)p.y()) / mHeight);
}

void ImageLabel::UpdateImage()
{
  const int kScaleMax = 9;
  static const int gScale[kScaleMax] = { 1, 2, 3, 4, 6, 8, 16, 24, 32 };
  if (mScaleAngle >= 0) {
    if (mScaleAngle/120 >= kScaleMax) {
      return;
    }
    int num = gScale[mScaleAngle/120];
    mWidth = mSourceImage.width() * num;
    mHeight = mSourceImage.height() * num;
    if (num > 1 && (mWidth > kMaxWidth || mHeight > kMaxHeight)) {
      return;
    }
    emit OnScale(num);
  } else {
    if ((-mScaleAngle)/120 >= kScaleMax) {
      return;
    }
    int denum = gScale[(-mScaleAngle)/120];
    mWidth = mSourceImage.width() / denum;
    mHeight = mSourceImage.height() / denum;
    if (denum > 2 && (mWidth < kMinWidth || mHeight < kMinHeight)) {
      return;
    }
    emit OnScale(-denum);
  }

  DrawImage();

  emit OnMove(mCenterPos.x() + mWidth/2, mCenterPos.y() + mHeight/2);
}

void ImageLabel::DrawImage()
{
  if (mWidth <= 0 || mHeight <= 0) {
    return;
  }

  QPixmap pixmap(mWidth, mHeight);
  QPainter painter(&pixmap);
  painter.drawImage(QRect(0, 0, mWidth, mHeight), mSourceImage);
  for (int i = 0; i < mLinePoints.size() - 1; i++) {
    QPoint p1 = ToScreen(mLinePoints.at(i));
    QPoint p2 = ToScreen(mLinePoints.at(i + 1));
    QLinearGradient gradient(p1, p2);
    gradient.setColorAt(0, QColor(71, 212, 255));
    gradient.setColorAt(1, QColor(27, 158, 198));

    QPen pen(QBrush(gradient), 3);
    painter.setPen(pen);
    painter.drawLine(p1, p2);
  }

  for (int i = 0; i < mLinePoints.size(); i++) {
    QPoint p1 = ToScreen(mLinePoints.at(i));
    QPen pen(QBrush(QColor(19, 121, 121)), i == mCurrentPoint? 8: 6);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);

    painter.drawPoint(p1);
  }
  setPixmap(pixmap);
}
