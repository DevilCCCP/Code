#include <QPaintEvent>
#include <QPainter>

#include "QWidgetB.h"


void QWidgetB::paintEvent(QPaintEvent* event)
{
  if (mBackImage.isNull()) {
    QWidget::paintEvent(event);
  }

  QRectF targetRect = event->rect();
  QRectF fullRect = geometry();
  qreal kx = mBackImage.width() / fullRect.width();
  qreal ky = mBackImage.height() / fullRect.height();
  QRectF sourceRect;
  sourceRect.setLeft(kx * targetRect.left());
  sourceRect.setRight(kx * targetRect.right());
  sourceRect.setTop(ky * targetRect.top());
  sourceRect.setBottom(ky * targetRect.bottom());
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.drawImage(targetRect, mBackImage, sourceRect);
}

void QWidgetB::resizeEvent(QResizeEvent* event)
{
  update();

  QWidget::resizeEvent(event);
}

void QWidgetB::SetBackImage(const QImage& _BackImage)
{
  mBackImage = _BackImage;

  update();
}


QWidgetB::QWidgetB(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
}

QWidgetB::~QWidgetB()
{
}
