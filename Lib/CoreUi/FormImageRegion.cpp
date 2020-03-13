#include <QPainter>

#include "FormImageRegion.h"


void FormImageRegion::paintEvent(QPaintEvent* event)
{
  Q_UNUSED(event);

  QPainter painter(this);
  painter.setBrush(QBrush(Qt::black));
  painter.drawRect(this->rect());
  painter.drawImage(rect(), mImage, mImage.rect());
}

void FormImageRegion::SetImage(const QImage& image)
{
  mImage = image;

  Resize();
  update();
}

void FormImageRegion::SetScale(qreal scale)
{
  if (mScale != scale) {
    mScale = scale;

    Resize();
    update();
  }
}

void FormImageRegion::Resize()
{
  if (mImage.isNull()) {
    return;
  }

  resize((int)(mImage.width() * mScale + 0.5), (int)(mImage.height() * mScale + 0.5));
}


FormImageRegion::FormImageRegion(QWidget* parent)
  : QWidget(parent)
  , mScale(1)
{
  resize(0, 0);
}

