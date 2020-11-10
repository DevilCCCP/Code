#include <QTimeZone>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <QtMath>

#include "FuncWidget.h"


const qreal kAxeWidth = 2;
const qreal kAxeTickWidth = 2;
const int kTextMargin = 4;
const int kAxeTickHeight = 3;
const qreal kFuncPointWidth = 2;
const qreal kFuncLineWidth = 1;
const qreal kFuncMedianWidth = 0.5;

void FuncWidget::SetAxesNames(const QString& _AxeX, const QString& _AxeY, const QString& _AxeXValue, const QString& _AxeYValue)
{
  mAxeX = _AxeX;
  mAxeY = _AxeY;
  mAxeXValue = _AxeXValue;
  mAxeYValue = _AxeYValue;

  update();
}

void FuncWidget::SetShowMedian(bool _ShowMedian)
{
  mShowMedian = _ShowMedian;

  update();
}

void FuncWidget::SetShowAxes(bool _ShowAxeX, bool _ShowAxeY)
{
  mShowAxeX = _ShowAxeX;
  mShowAxeY = _ShowAxeY;

  update();
}

void FuncWidget::SetBackgroundColor(const QColor& _BackgroundColor)
{
  mBackgroundColor = _BackgroundColor;

  update();
}

void FuncWidget::SetFuncColor(const QColor& _FuncColor)
{
  mFuncColor = _FuncColor;

  update();
}

void FuncWidget::SetAxeColor(const QColor& _AxeColor)
{
  mAxeColor = _AxeColor;

  update();
}

void FuncWidget::SetXRange(qreal minX, qreal maxX)
{
  mValueXRangeMin = minX;
  mValueXRangeMax = maxX;

  update();
}

void FuncWidget::SetValues(const QVector<QPointF>& values)
{
  mValues = values;

  if (mValueXRangeMin < mValueXRangeMax) {
    mValueXMin = mValueXRangeMin;
    mValueXMax = mValueXRangeMax;
  } else if (!values.isEmpty()) {
    mValueXMin = values.first().x();
    mValueXMax = values.first().x();
  } else {
    mValueXMin = mValueXMax = 0;
  }

  if (!values.isEmpty()) {
    mValueYMin = values.first().y();
    mValueYMax = values.first().y();
    if (mValueXRangeMin < mValueXRangeMax) {
      mValueXMin = mValueXRangeMin;
      mValueXMax = mValueXRangeMax;
    } else {
      mValueXMin = values.first().x();
      mValueXMax = values.first().x();
    }
    QVector<qreal> valueY;
    for (const QPointF& p: values) {
      mValueYMin = qMin(mValueYMin, p.y());
      mValueYMax = qMax(mValueYMax, p.y());
      mValueXMin = qMin(mValueXMin, p.x());
      mValueXMax = qMax(mValueXMax, p.x());
      valueY.push_back(p.y());
    }

    std::sort(valueY.begin(), valueY.end());
    mValueYMedian = valueY.at(valueY.size()/2);
  } else {
    mValueYMin = mValueYMax = mValueYMedian = 0;
  }

  SetValueScales();

  update();
}

void FuncWidget::paintEvent(QPaintEvent* event)
{
  Q_UNUSED(event);

  if (width() <= 0 || height() <= 0) {
    return;
  }

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  CalcDrawPositions(&painter);
  painter.fillRect(0, 0, width(), height(), mBackgroundColor);
  DrawAxe(&painter);
  DrawInfo(&painter);
  DrawFunc(&painter);
}

void FuncWidget::SetValueScales()
{
  if (mValues.isEmpty()) {
    mValueXPrecission = mValueYPrecission = 0;
    mValueXRescale = mValueYRescale = 1;
    mValueXPrefix = mValueYPrefix = QString("");
    return;
  }

  qreal x = qMax(qAbs(mValueXMin), qAbs(mValueXMax));
  qreal y = qMax(qAbs(mValueYMin), qAbs(mValueYMax));
  SetValueScale(x, mValueXPrecission, mValueXRescale, mValueXPrefix);
  SetValueScale(y, mValueYPrecission, mValueYRescale, mValueYPrefix);
}

void FuncWidget::SetValueScale(qreal value, int& precission, qreal& rescale, QString& prefix)
{
  if (value >= 1000000000) {
    prefix  = QString("Г");
    rescale = 0.000000001;
  } else if (value >= 1000000) {
    prefix  = QString("М");
    rescale = 0.000001;
  } else if (value >= 1000) {
    prefix  = QString("К");
    rescale = 0.001;
  } else if (value >= 1) {
    prefix  = QString("");
    rescale = 1;
  } else if (value >= 0.001) {
    prefix  = QString("м");
    rescale = 1000.0;
  } else if (value >= 0.000001) {
    prefix  = QString("мк");
    rescale = 1000000.0;
  } else {
    prefix  = QString("");
    rescale = 1;
  }
  value *= rescale;

  if (value > 1000) {
    precission = 0;
  } else if (value > 100) {
    precission = 1;
  } else if (value > 10) {
    precission = 2;
  } else {
    precission = 3;
  }
}

void FuncWidget::CalcDrawPositions(QPainter* painter)
{
  const QString testValueX(QString("-999.99 Г%0").arg(mAxeXValue));
  const QString testValueY(QString("-999.99 Г%0").arg(mAxeYValue));
  QFontMetrics fontMetrics = painter->fontMetrics();
  mAxeXTextRect = fontMetrics.boundingRect(testValueX);
  mAxeYTextRect = fontMetrics.boundingRect(testValueY);
  mAxeXNameRect = fontMetrics.boundingRect(mAxeX);
  mAxeYNameRect = fontMetrics.boundingRect(mAxeY);

  mAxePosX = width() - (mShowAxeY? (mAxeYTextRect.width() + 2 * kTextMargin): 0);
  mAxePosY = height() - (mShowAxeX? (mAxeXTextRect.height() + 2 * kTextMargin): 0);
}

void FuncWidget::DrawAxe(QPainter* painter)
{
  painter->setPen(QPen(QBrush(mAxeColor), kAxeWidth));
  if (mShowAxeX) {
    painter->drawLine(0, mAxePosY, mAxePosX, mAxePosY);
  }
  if (mShowAxeY) {
    painter->drawLine(mAxePosX, 0, mAxePosX, mAxePosY);
  }
}

void FuncWidget::DrawInfo(QPainter* painter)
{
  painter->setPen(QPen(QBrush(mAxeColor), kAxeTickWidth));

  if (!mValues.isEmpty()) {
    if (mShowAxeY) {
      int valuesCount = mAxePosY / (mAxeYTextRect.height() + 2*kTextMargin);
      QVector<qreal> drawValues;
      drawValues.append(mValueYMin);
      if (mValueYMax > mValueYMin) {
        drawValues.append(mValueYMax);
      }
      if (qAbs(mValueYMedian - mValueYMin) > (mValueYMax - mValueYMin) / valuesCount
          && qAbs(mValueYMedian - mValueYMax) > (mValueYMax - mValueYMin) / valuesCount) {
        drawValues.append(mValueYMedian);
      }
      for (int i = 0; i < drawValues.size(); i++) {
        qreal value = drawValues.at(i);
        int y = mAxePosY/2;
        if (mValueYMax > mValueYMin) {
          y = (mValueYMax - value) / (mValueYMax - mValueYMin) * mAxePosY;
        }
        value *= mValueYRescale;
        painter->drawLine(mAxePosX - kAxeTickHeight, y, mAxePosX + kAxeTickHeight, y);
        QString valueText = QString("%1%2%3").arg(value, 0, 'f', mValueYPrecission).arg(mValueYPrefix).arg(mAxeYValue);
        if (y < mAxeYTextRect.height()) {
          QRect valueTickRect(mAxePosX + kTextMargin, y, mAxeYTextRect.width(), mAxeYTextRect.height());
          painter->drawText(valueTickRect, Qt::AlignLeft | Qt::AlignTop, valueText);
        } else {
          QRect valueTickRect(mAxePosX + kTextMargin, mAxePosY - mAxeYTextRect.height(), mAxeYTextRect.width(), mAxeYTextRect.height());
          painter->drawText(valueTickRect, Qt::AlignLeft | Qt::AlignBottom, valueText);
        }
      }
    }

    if (mShowAxeX) {
      int valuesCount = 1;
      if (mValueXMax > mValueXMin) {
        valuesCount = qMax(2, mAxePosX / (mAxeXTextRect.width() + 2*kTextMargin));
      }
      for (int i = 0; i < valuesCount; i++) {
        qreal value = mValueXMin + (mValueXMax - mValueXMin) * i / (valuesCount - 1);
        int x = 0;
        if (mValueXMax > mValueXMin) {
          x = (value - mValueXMin) / (mValueXMax - mValueXMin) * mAxePosX;
        }
        value *= mValueXRescale;
        painter->drawLine(x, mAxePosY - kAxeTickHeight, x, mAxePosY + kAxeTickHeight);
        QString valueText = QString("%1%2%3").arg(value, 0, 'f', mValueXPrecission).arg(mValueXPrefix).arg(mAxeXValue);
        if (x < mAxeXTextRect.width()) {
          QRect valueTickRect(x, mAxePosY + kTextMargin, mAxeXTextRect.width(), mAxeXTextRect.height());
          painter->drawText(valueTickRect, Qt::AlignTop | Qt::AlignLeft, valueText);
        } else {
          QRect valueTickRect(mAxePosX - mAxeXTextRect.width(), mAxePosY + kTextMargin, mAxeXTextRect.width(), mAxeXTextRect.height());
          painter->drawText(valueTickRect, Qt::AlignTop | Qt::AlignRight, valueText);
        }
      }
    }
  }

  QRect axeXNameRect(kTextMargin, mAxePosY - (mAxeXNameRect.height() + kTextMargin), mAxeXNameRect.width(), mAxeXNameRect.height());
  painter->drawText(axeXNameRect, Qt::AlignLeft | Qt::AlignBottom, mAxeX);
  QRect axeYNameRect(mAxePosX - (mAxeYNameRect.width() + kTextMargin), kTextMargin, mAxeYNameRect.width(), mAxeYNameRect.height());
  painter->drawText(axeYNameRect, Qt::AlignRight | Qt::AlignTop, mAxeY);
}

void FuncWidget::DrawFunc(QPainter* painter)
{
  painter->setClipRect(0, 0, mAxePosX, mAxePosY);
  if (mShowMedian) {
    painter->setPen(QPen(QBrush(mFuncColor), kFuncMedianWidth));
    if (mValueYMax > mValueYMin) {
      int y = (mValueYMax - mValueYMedian) / (mValueYMax - mValueYMin) * mAxePosY;
      painter->drawLine(0, y, mAxePosX, y);
    }
  }

  painter->setPen(QPen(QBrush(mFuncColor), kFuncLineWidth));
  if (mValueXMax > mValueXMin) {
    for (int i = 1; i < mValues.size(); i++) {
      const QPointF& p1 = mValues.at(i - 1);
      const QPointF& p2 = mValues.at(i);
      int x1 = (p1.x() - mValueXMin) / (mValueXMax - mValueXMin) * mAxePosX;
      int x2 = (p2.x() - mValueXMin) / (mValueXMax - mValueXMin) * mAxePosX;
      int y1 = mAxePosY/2;
      int y2 = mAxePosY/2;
      if (mValueYMax > mValueYMin) {
        y1 = (mValueYMax - p1.y()) / (mValueYMax - mValueYMin) * mAxePosY;
        y2 = (mValueYMax - p2.y()) / (mValueYMax - mValueYMin) * mAxePosY;
      }
      painter->drawLine(x1, y1, x2, y2);
    }
  }

  painter->setPen(QPen(QBrush(mFuncColor), kFuncPointWidth));
  for (int i = 0; i < mValues.size(); i++) {
    const QPointF& p = mValues.at(i);
    int x = 0;
    if (mValueXMax > mValueXMin) {
      x = (p.x() - mValueXMin) / (mValueXMax - mValueXMin) * mAxePosX;
    }
    int y = mAxePosY/2;
    if (mValueYMax > mValueYMin) {
      y = (mValueYMax - p.y()) / (mValueYMax - mValueYMin) * mAxePosY;
    }
    painter->drawPoint(x, y);
  }
  painter->setClipping(false);
}


FuncWidget::FuncWidget(QWidget* parent)
  : QWidget(parent)
  , mAxeX("X"), mAxeY("Y"), mShowMedian(false), mShowAxeX(true), mShowAxeY(true)
  , mValueXMin(0), mValueXMax(0)
  , mBackgroundColor(Qt::transparent), mFuncColor(Qt::darkGray), mAxeColor(Qt::blue), mAxePosX(0), mAxePosY(0)
{
}
