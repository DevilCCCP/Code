#include <QDesktopWidget>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QToolTip>

#include "HystLabel.h"


HystLabel::HystLabel(QWidget* parent)
  : QLabel(parent)
{
  mWidth = QDesktopWidget().availableGeometry().width();
  mHeight = QDesktopWidget().availableGeometry().height();
}


void HystLabel::SetHyst(const Hyst& _Hyst)
{
  mHyst = _Hyst;

  Draw();
}

void HystLabel::SetHystFast(const HystFast& _Hyst)
{
  mHyst.SetHystFast(_Hyst);

  Draw();
}

void HystLabel::mouseMoveEvent(QMouseEvent* event)
{
  QWidget::mouseMoveEvent(event);
}

void HystLabel::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);

  mWidth = event->size().width();
  mHeight = event->size().height();

  Draw();
}

void HystLabel::Draw()
{
  QPixmap pixmap(mWidth, mHeight);
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);
  {
    QLinearGradient gradient(mWidth/2, 0, mWidth/2, mHeight);
    gradient.setColorAt(0, QColor(0, 0, 0));
    gradient.setColorAt(1, QColor(255, 255, 255));
    painter.setBrush(QBrush(gradient));
    painter.drawPolygon(QPolygon(QRect(0, 0, mWidth, mHeight)));
  }

  int maxValue = 1;
  for (int i = 0; i < mHyst.Length(); i++) {
    maxValue = qMax(maxValue, mHyst.GetHyst(i));
  }

  painter.setPen(QColor(0, 200, 200));
  QVector<int> cutValues;
  cutValues.append(50);
  cutValues.append(500);
  cutValues.append(950);
  for (int i = 0; i < cutValues.size(); i++) {
    int cutValue = cutValues.at(i);
    int h = (height() - 1) * mHyst.GetValue(cutValue) / (mHyst.Length() - 1);
    h = qBound(1, h, height() - 2);
    painter.drawLine(QPoint(width() * i / cutValues.size(), h), QPoint(width() * (i+1) / cutValues.size(), h));
  }

  painter.setPen(QColor(200, 0, 0));
  int lastHeight = -1;
  int lastValue = -1;
  for (int i = 0; i < mHyst.Length(); i++) {
    int value = width() * mHyst.GetHyst(i) / maxValue;
    int h = (height() - 1) * i / (mHyst.Length() - 1);
    if (lastHeight >= 0) {
      painter.drawLine(QPoint(lastValue, lastHeight), QPoint(value, h));
    }
    lastHeight = h;
    lastValue = value;
  }

  setPixmap(pixmap);
}
