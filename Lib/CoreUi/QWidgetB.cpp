#include <QPaintEvent>
#include <QPainter>

#include "QWidgetB.h"


void QWidgetB::paintEvent(QPaintEvent* event)
{
  if (mBackImage.isNull()) {
    QWidget::paintEvent(event);
  }

  switch (mFillStyle) {
  case eStretch: DrawRectStretch(event->rect()); break;
  case eCopy   : DrawRectCopy(event->rect()); break;
  }
}

void QWidgetB::resizeEvent(QResizeEvent* event)
{
  switch (mFillStyle) {
  case eStretch: update(); break;
  case eCopy   : break;
  }

  QWidget::resizeEvent(event);
}

void QWidgetB::SetFillStyle(QWidgetB::FillStyle _FillStyle)
{
  mFillStyle = _FillStyle;
}

void QWidgetB::SetBackImage(const QImage& _BackImage)
{
  mBackImage = _BackImage;
}

void QWidgetB::DrawRectStretch(const QRectF& rect)
{
  QRectF fullRect = geometry();
  qreal kx = mBackImage.width() / fullRect.width();
  qreal ky = mBackImage.height() / fullRect.height();
  QRectF sourceRect;
  sourceRect.setLeft(kx * rect.left());
  sourceRect.setRight(kx * rect.right());
  sourceRect.setTop(ky * rect.top());
  sourceRect.setBottom(ky * rect.bottom());
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.drawImage(rect, mBackImage, sourceRect);
}

void QWidgetB::DrawRectCopy(const QRectF& rect)
{
  int l = (int)(rect.left() / mBackImage.width());
  int r = (int)(rect.right() / mBackImage.width());
  int t = (int)(rect.top() / mBackImage.height());
  int b = (int)(rect.bottom() / mBackImage.height());
  for (int j = t; j <= b; j++) {
    qreal dt = qMax(rect.top(), (qreal)(mBackImage.height() * j));
    qreal db = qMin(rect.bottom(), (qreal)(mBackImage.height() * (j + 1)));
    qreal st = dt - (qreal)(mBackImage.height() * j);
    qreal sb = db - (qreal)(mBackImage.height() * j);
    for (int i = l; i <= r; i++) {
      qreal dl = qMax(rect.left(), (qreal)(mBackImage.width() * i));
      qreal dr = qMin(rect.right(), (qreal)(mBackImage.width() * (i + 1)));
      qreal sl = dl - (qreal)(mBackImage.width() * i);
      qreal sr = dr - (qreal)(mBackImage.width() * i);
      QPainter painter(this);
      painter.setRenderHint(QPainter::Antialiasing);
      painter.drawImage(QRectF(dl, dt, dr - dl, db - dt), mBackImage, QRectF(sl, st, sr - sl, sb - st));
    }
  }
}


QWidgetB::QWidgetB(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
  , mFillStyle(eStretch)
{
}

QWidgetB::~QWidgetB()
{
}
