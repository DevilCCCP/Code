#include <QDesktopWidget>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QToolTip>

#include <LibV/Include/HystFast.h>

#include "GraphLabel.h"


const int kMaxAxesPoints = 8;
const int kAxeWidth = 80;
const int kAxeFontHeight = 20;

GraphLabel::GraphLabel(QWidget* parent)
  : QLabel(parent)
{
  mWidth = QDesktopWidget().availableGeometry().width();
  mHeight = QDesktopWidget().availableGeometry().height();
}


void GraphLabel::SetLineValues(const QVector<uchar>& values, const QVector<uchar>& marks)
{
  if (values.size() <= 1) {
    return;
  }

  mSourceValues = values;
  mSourceMarks  = marks;
  Draw();
}

void GraphLabel::mouseMoveEvent(QMouseEvent* event)
{
  QWidget::mouseMoveEvent(event);

  for (int i = 0; i < mGraphPoints.size(); i++) {
    QPoint d = event->pos() - mGraphPoints.at(i);
    if (d.manhattanLength() < 5) {
      QToolTip::showText(mapToGlobal(event->pos()), QString("Point: %1, Value: %2, Mark: %3")
                         .arg(i).arg(mSourceValues.value(i)).arg(mSourceMarks.value(i)), this);
    }
  }
}

void GraphLabel::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);

  mWidth = event->size().width();
  mHeight = event->size().height() - 1;
  Draw();
}

void GraphLabel::Draw()
{
  int minValue = 255;
  int maxValue = 0;
  HystFast hyst;
  foreach (const uchar& v, mSourceValues) {
    hyst.Inc((int)v);
    minValue = qMin(minValue, (int)v);
    maxValue = qMax(maxValue, (int)v);
  }
  minValue = qMax(minValue - 5, 0);
  maxValue = qMin(maxValue + 5, 255);

  int maxMark = 1;
  foreach (const uchar& m, mSourceMarks) {
    maxMark = qMax(maxMark, (int)m);
  }

  QPixmap pixmap(mWidth, mHeight);
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);
  {
    QLinearGradient gradient(mWidth/2, 0, mWidth/2, mHeight);
    gradient.setColorAt(0, QColor(minValue, minValue, minValue));
    gradient.setColorAt(1, QColor(maxValue, maxValue, maxValue));
    painter.setBrush(QBrush(gradient));
    painter.drawPolygon(QPolygon(QRect(0, 0, mWidth, mHeight)));
  }

  int delta = maxValue - minValue;
  int thresh = 10;
  while (delta / thresh > kMaxAxesPoints) {
    thresh += thresh;
  }
  QList<int> axePoints;
  axePoints.append(minValue);
  for (int i = ((minValue + thresh/2) / thresh + 1) * thresh; i < maxValue - thresh/2; i += thresh) {
    axePoints.append(i);
  }
  axePoints.append(maxValue);

  int clientX = kAxeWidth;
  int clientWidth = mWidth - 2 * kAxeWidth;
  if (clientWidth <= 0) {
    setPixmap(QPixmap());
    return;
  }
  for (int i = 0; i < 2; i++) {
    int x = (i == 0)? 2: mWidth - kAxeWidth + 2;
    int xLine = (i == 0)? kAxeWidth - 2: mWidth - kAxeWidth + 2;
    int hzAlign = (i == 0)? Qt::AlignRight: Qt::AlignLeft;

    painter.setPen(QColor(200, 0, 0));
    painter.drawLine(QPoint(xLine, 0), QPoint(xLine, mHeight));
    foreach (int j, axePoints) {
      int h = (j - minValue) * mHeight / (maxValue - minValue);
      painter.drawLine(QPoint(xLine - 2, h), QPoint(xLine + 2, h));
    }

    QFont fnt(font().family(), kAxeFontHeight/2, 0);
    fnt.setBold(true);
    painter.setFont(fnt);
    foreach (int j, axePoints) {
      if (j < 127) {
        painter.setPen(QColor(0, 230, 230));
      } else {
        painter.setPen(QColor(0, 120, 120));
      }
      int h = (j - minValue) * mHeight / (maxValue - minValue);
      if (h > mHeight / 2) {
        h -= kAxeFontHeight;
      }
      int vertAlign = (h < mHeight / 2)? Qt::AlignTop: Qt::AlignBottom;
      painter.drawText(x, h, kAxeWidth - 4, kAxeFontHeight, hzAlign | vertAlign, QString::number(j));
    }
  }
  int blackValue      = hyst.GetValue(100) + kHystFastLength/2;
  int whiteValue      = hyst.GetValue(900) + kHystFastLength/2;
  int blackThreshold = (2*blackValue + whiteValue)/3;
  int whiteThreshold = (blackValue + 2*whiteValue)/3;
  int h0 = (blackValue - minValue) * mHeight / (maxValue - minValue);
  painter.drawLine(QPoint(kAxeWidth - 2, h0), QPoint(mWidth - kAxeWidth + 2, h0));
  int h1 = (blackThreshold - minValue) * mHeight / (maxValue - minValue);
  painter.drawLine(QPoint(kAxeWidth - 2, h1), QPoint(mWidth - kAxeWidth + 2, h1));
  int h2 = (whiteThreshold - minValue) * mHeight / (maxValue - minValue);
  painter.drawLine(QPoint(kAxeWidth - 2, h2), QPoint(mWidth - kAxeWidth + 2, h2));
  int h3 = (whiteValue - minValue) * mHeight / (maxValue - minValue);
  painter.drawLine(QPoint(kAxeWidth - 2, h3), QPoint(mWidth - kAxeWidth + 2, h3));

  painter.setPen(QColor(200, 0, 0));
  painter.drawText(clientX, 0, clientWidth, mHeight, Qt::AlignBottom | Qt::AlignRight, QString::number(maxMark));

  mGraphPoints.clear();
  for (int i = 0; i < mSourceValues.size(); i++) {
    int x1 = clientX + i * clientWidth / (mSourceValues.size() - 1);
    int y1 = (mSourceValues.at(i) - minValue) * mHeight / (maxValue - minValue);
    mGraphPoints.append(QPoint(x1, y1));
  }

  painter.setPen(QColor(0, 230, 230));
  for (int i = 0; i < mGraphPoints.size() - 1; i++) {
    painter.drawLine(QPoint(mGraphPoints.at(i).x(), mGraphPoints.at(i).y()), QPoint(mGraphPoints.at(i+1).x(), mGraphPoints.at(i+1).y()));
  }

  for (int i = 0; i < mGraphPoints.size(); i++) {
    static QColor nomarkColor(0, 120, 120);
    QColor col = nomarkColor;
    if (i < mSourceMarks.size() && mSourceMarks.at(i) > 0) {
      int val = mSourceMarks.at(i) * 255 / maxMark;
      col = QColor(val, 255 - val, 0);
    }
    QPen pen(QBrush(col), 6);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.drawPoint(QPoint(mGraphPoints.at(i).x(), mGraphPoints.at(i).y()));
  }

  setPixmap(pixmap);
}
