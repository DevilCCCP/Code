#include <QPaintEvent>
#include <QPainter>

#include "WidgetImageR.h"


void WidgetImageR::paintEvent(QPaintEvent* event)
{
  QWidgetB::paintEvent(event);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QSize imageSize = GetImageSize();
  if (imageSize.width() < 1 || imageSize.height() < 1) {
    return;
  }

  for (int i = 0; i < mRectList.size(); i++) {
    if (mColorList.size() > i) {
      painter.setPen(QPen(QBrush(mColorList.at(i)), mRectWidth));
    }
    const QRect rect = mRectList.at(i);
    qreal kw = (qreal)width() / imageSize.width();
    qreal kh = (qreal)height() / imageSize.height();
    painter.drawRect(kw * rect.x(), kh * rect.y(), kw * rect.width(), kh * rect.height());
  }
}

void WidgetImageR::SetRectColor(const QColor& color)
{
  mColorList.clear();
  mColorList.append(color);
}

void WidgetImageR::SetRectWidth(int width)
{
  mRectWidth = width;
}

void WidgetImageR::SetImageRect(const QRect& rect)
{
  mRectList.clear();
  mRectList.append(rect);

  update();
}

void WidgetImageR::SetImageRect(const QRect& rect, const QColor& color)
{
  mColorList.clear();
  mColorList.append(color);

  SetImageRect(rect);
}

void WidgetImageR::SetImageRectList(const QList<QRect>& rectList)
{
  mRectList = rectList;

  update();
}

void WidgetImageR::SetImageRectList(const QList<QRect>& rectList, const QColor& color)
{
  mColorList.clear();
  mColorList.append(color);

  SetImageRectList(rectList);
}

void WidgetImageR::SetImageRectList(const QList<QRect>& rectList, const QList<QColor>& colorList)
{
  mColorList = colorList;

  SetImageRectList(rectList);
}


WidgetImageR::WidgetImageR(QWidget* parent, Qt::WindowFlags f)
  : QWidgetB(parent, f)
  , mRectWidth(1)
{
}

