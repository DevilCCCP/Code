#include <QTimeZone>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <QtMath>

#include "GraphWidget.h"


const int kPrecisePixel = 2;
const int kTextMargin = 4;
const int kTimelineTickHeight = 3;
const int kValueTickWidth = 3;

void GraphWidget::SetAutoInfo(const QStringList& _Info, const QString& _ValueScale)
{
  mInfo = _Info;
  QList<QColor> colorList;

  for (int i = 0; i < mInfo.size(); i++) {
    int hue = 0 + (2*i + 1) * 300 / (2 * mInfo.size());
    int saturation = 200;
    int value = 150;
    QColor infoColor;
    infoColor.setHsv(hue, saturation, value);
    colorList.append(infoColor);
  }

  SetInfo(_Info, colorList, _ValueScale);
}

void GraphWidget::SetInfo(const QStringList& _Info, const QList<QColor>& _InfoColorList, const QString& _ValueScale)
{
  mInfo = _Info;
  mInfoColorList = _InfoColorList;
  mValueScale = _ValueScale;
}

void GraphWidget::SetValueBounds(qreal _ValueMin, qreal _ValueMax)
{
  mValueAbsoluteMin = qMin(_ValueMin, _ValueMax);
  mValueAbsoluteMax = qMax(_ValueMin, _ValueMax);

  update();
}

void GraphWidget::SetGraph(const Graph& _Graph)
{
  mGraph = _Graph;

  mTimeZoom = 1.0;
  mTimeZoomExp = 1.0;
  mTimePosSecs = 0;

  mCurrentTime = QDateTime();
  mCurrentTimePos = -1;
  mCurrentValue = mValueAbsoluteMin - 1;
  mCurrentValuePos = -1;

  mGraphBegin = QDateTime();
  mGraphEnd = QDateTime();

  mValueMin = mValueAbsoluteMin;
  mValueMax = mValueAbsoluteMax;
  for (const GraphUnit& graphUnit: mGraph) {
    for (const TimePoint& timePoint: graphUnit) {
      if (mGraphBegin.isValid()) {
        mGraphBegin = qMin(mGraphBegin, timePoint.TimeFrom);
      } else {
        mGraphBegin = timePoint.TimeFrom;
      }
      if (mGraphEnd.isValid()) {
        mGraphEnd = qMax(mGraphEnd, timePoint.TimeTo);
      } else {
        mGraphEnd = timePoint.TimeTo;
      }
      mValueMin = qMin(mValueMin, timePoint.Value);
      mValueMax = qMax(mValueMax, timePoint.Value);
    }
  }

  qreal valueLong = qMax(qAbs(mValueMin), qAbs(mValueMax));
  if (valueLong >= 1000000000) {
    mValuePrefix  = QString("Г");
    mValueRescale = 0.000000001;
  } else if (valueLong >= 1000000) {
    mValuePrefix  = QString("М");
    mValueRescale = 0.000001;
  } else if (valueLong >= 1000) {
    mValuePrefix  = QString("К");
    mValueRescale = 0.001;
  } else if (valueLong >= 1) {
    mValuePrefix  = QString("");
    mValueRescale = 1;
  } else if (valueLong >= 0.001) {
    mValuePrefix  = QString("м");
    mValueRescale = 1000.0;
  } else if (valueLong >= 0.000001) {
    mValuePrefix  = QString("мк");
    mValueRescale = 1000000.0;
  } else {
    mValuePrefix  = QString("");
    mValueRescale = 1;
  }
  valueLong *= mValueRescale;

  if (valueLong > 1000) {
    mValuePrecission = 0;
  } else if (valueLong > 100) {
    mValuePrecission = 2;
  } else if (valueLong > 10) {
    mValuePrecission = 3;
  } else {
    mValuePrecission = 4;
  }

  mGraphPeriodSecs = (mGraphBegin.isValid() && mGraphEnd.isValid())? (mGraphEnd.toMSecsSinceEpoch() - mGraphBegin.toMSecsSinceEpoch()) / 1000: 1;
  if (mGraphPeriodSecs >= 7 * 24 * 60 * 60) {
    mTimePeriodScaleSecs = 24 * 60 * 60;
  } else if (mGraphPeriodSecs >= 4 * 24 * 60 * 60) {
    mTimePeriodScaleSecs = 12 * 60 * 60;
  } else if (mGraphPeriodSecs >= 24 * 60 * 60) {
    mTimePeriodScaleSecs = 60 * 60;
  } else if (mGraphPeriodSecs >= 60 * 60) {
    mTimePeriodScaleSecs = 5 * 60;
  } else {
    mTimePeriodScaleSecs = 60;
  }

  if (mGraphBegin.isValid() && mGraphEnd.isValid()) {
    int offsetUtc = mGraphBegin.timeZone().offsetFromUtc(mGraphBegin);
    qint64 timeBeginSecs = mGraphBegin.toSecsSinceEpoch() + offsetUtc;
    qint64 timeEndSecs   = mGraphEnd.toSecsSinceEpoch() + offsetUtc;
    mTimeBegin = QDateTime::fromSecsSinceEpoch(((timeBeginSecs - mTimePeriodScaleSecs/2) / mTimePeriodScaleSecs) * mTimePeriodScaleSecs - offsetUtc);
    mTimeEnd   = QDateTime::fromSecsSinceEpoch(((timeEndSecs + 3*mTimePeriodScaleSecs/2 - 1) / mTimePeriodScaleSecs) * mTimePeriodScaleSecs - offsetUtc);
  } else {
    mTimeBegin = mTimeEnd = QDateTime();
  }
  mTimePeriodSecs = (mTimeBegin.isValid() && mTimeEnd.isValid())? (mTimeEnd.toMSecsSinceEpoch() - mTimeBegin.toMSecsSinceEpoch()) / 1000: 1;

  update();
}

void GraphWidget::SetPrecision(int _PrecisionSec)
{
  mPrecisionSec = qMax(1, _PrecisionSec);

  update();
}

void GraphWidget::paintEvent(QPaintEvent* event)
{
  Q_UNUSED(event);

  if (width() <= 0 || height() <= 0) {
    return;
  }

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  CalcDrawPositions(&painter);
  DrawAxe(&painter);
  DrawInfo(&painter);
  DrawGraph(&painter);
}

void GraphWidget::mouseMoveEvent(QMouseEvent* event)
{
  if (mTimePeriodSecs < 2) {
    return;
  }

  qreal posX = event->localPos().x();

  if (mTimeMoving) {
    mTimePosSecs = mTimePosMovingBase + (mTimeMovingBase - posX) / (mGraphEndX - mGraphPosX) * mTimePeriodSecs / mTimeZoom;
    mTimePosSecs = qBound(0, mTimePosSecs, qRound(mTimePeriodSecs - mTimePeriodSecs / mTimeZoom));

    update();
  }

  QDateTime newTime;
  qreal newTimePos = -1;
  if (posX >= mGraphPosX && posX < mGraphEndX + kTextMargin) {
    newTimePos = qMin(posX - mGraphPosX, (qreal)mGraphEndX - mGraphPosX);
    int newTimeSecs = qRound(mTimePeriodSecs / mTimeZoom * (posX - mGraphPosX) / (mGraphEndX - mGraphPosX)) + mTimePosSecs;
    newTimeSecs = qBound(0, newTimeSecs, mTimePeriodSecs);
    newTime = mTimeBegin.addSecs(newTimeSecs);
  }

  if (newTime != mCurrentTime) {
    mCurrentTime = newTime;
    mCurrentTimePos = newTimePos;

    update();
  }

  qreal posY = event->localPos().y();
  qreal newValue = mValueMin + ((mGraphPosY - posY) / (mGraphPosY - kTextMargin)) * (mValueMax - mValueMin);
  if (newValue < mValueMin || newValue > mValueMax) {
    newValue = mValueMin - 1;
  }

  if (newValue != mCurrentValue) {
    mCurrentValue = newValue;
    mCurrentValuePos = posY;

    update();
  }
}

void GraphWidget::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    mTimeMoving = true;
    mTimeMovingBase = event->localPos().x();
    mTimePosMovingBase = mTimePosSecs;
  } else {
    mTimeMoving = false;
  }

  setCursor(mTimeMoving? Qt::ClosedHandCursor: Qt::ArrowCursor);
}

void GraphWidget::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    mTimeMoving = false;
  }

  setCursor(mTimeMoving? Qt::ClosedHandCursor: Qt::ArrowCursor);
}

void GraphWidget::leaveEvent(QEvent* event)
{
  Q_UNUSED(event);

  if (mCurrentTime.isValid()) {
    mCurrentTime = QDateTime();
    mCurrentTimePos = -1;

    update();
  }

  if (mTimeMoving) {
    mTimeMoving = false;
    setCursor(Qt::ArrowCursor);
  }
}

void GraphWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
  qreal posX = event->localPos().x();
  if (posX >= mGraphPosX && posX < mGraphEndX + kTextMargin) {
    int timeSecs = qRound(mTimePeriodSecs / mTimeZoom * (posX - mGraphPosX) / (mGraphEndX - mGraphPosX)) + mTimePosSecs;
    timeSecs = qBound(0, timeSecs, mTimePeriodSecs);
    QDateTime clickTime = mTimeBegin.addSecs(timeSecs);

    emit TimeSelected(clickTime);
  }
}

void GraphWidget::wheelEvent(QWheelEvent* event)
{
  qreal timeCenter = mTimePosSecs + 0.5 * mTimePeriodSecs / mTimeZoom;
  mTimeZoomExp = qMax(1.0, mTimeZoomExp + event->angleDelta().y() / 120.0);
  mTimeZoom = qPow(2.0, mTimeZoomExp) * 0.5;
  mTimePosSecs = timeCenter - 0.5 * mTimePeriodSecs / mTimeZoom;
  mTimePosSecs = qBound(0, mTimePosSecs, mTimePeriodSecs - qRound(mTimePeriodSecs / mTimeZoom));

  update();

  event->accept();
}

void GraphWidget::CalcDrawPositions(QPainter* painter)
{
  const QString testTime("30.05.2020 00:00");
  const QString testValue("-999.99");
  QFontMetrics fontMetrics = painter->fontMetrics();
  mTimeRect  = fontMetrics.boundingRect(testTime);
  mValueRect = fontMetrics.boundingRect(mValueScale);
  QRect valueRect = fontMetrics.boundingRect(testValue);
  mValueRect.setWidth(qMax(mValueRect.width(), valueRect.width()));

  mGraphPosX = 0;
  for (const QString& name: mInfo) {
    QFontMetrics fontMetrics = painter->fontMetrics();
    mGraphPosX = qMax(mGraphPosX, fontMetrics.boundingRect(name).width() + 2 * kTextMargin);
  }
  mGraphEndX = width() - kTextMargin - mValueRect.width() - 2*kTextMargin;

  mGraphPosY = height() - 2 * (mTimeRect.height() + 2 * kTextMargin);
  mAxePosY = height() - (mTimeRect.height() + 2 * kTextMargin);
  int timeWidth = (mTimeRect.width() + 2 * kTextMargin) * 2;
  mTimeCount = (mGraphEndX - mGraphPosX) / timeWidth;
  if (!mTimeBegin.isValid() || !mTimeEnd.isValid()) {
    mTimeCount = -1;
  } else if (mTimeCount > 48) {
    mTimeCount = 48;
  } else if (mTimeCount > 24) {
    mTimeCount = 24;
  } else if (mTimeCount > 12) {
    mTimeCount = 12;
  } else if (mTimeCount > 6) {
    mTimeCount = 6;
  } else if (mTimeCount > 4) {
    mTimeCount = 4;
  } else {
    mTimeCount = 2;
  }
}

void GraphWidget::DrawAxe(QPainter* painter)
{
  painter->setPen(QPen(QBrush(QColor(Qt::black)), 2));
  painter->drawLine(mGraphPosX, mAxePosY, mGraphEndX, mAxePosY);
  for (int i = 1; i < 2*mTimeCount; i += 2) {
    int x = mGraphPosX + (mGraphEndX - mGraphPosX) * i / (2*mTimeCount);
    painter->drawLine(x, mAxePosY - kTimelineTickHeight, x, mAxePosY + kTimelineTickHeight);
  }
  painter->drawLine(mGraphEndX, kTextMargin, mGraphEndX, mAxePosY);
  painter->drawText(width() - mValueRect.width() - 2*kTextMargin, kTextMargin + mValueRect.height(), mValueRect.width() + kTextMargin, mValueRect.height()
                    , Qt::AlignRight | Qt::AlignTop, QString("%1%2").arg(mValuePrefix, mValueScale));

  for (int i = 0; i <= mTimeCount; i++) {
    int x = mGraphPosX + (mGraphEndX - mGraphPosX) * i / mTimeCount;
    QDateTime time = mTimeBegin.addSecs(mTimePosSecs + mTimePeriodSecs * i / mTimeCount / mTimeZoom);
    painter->drawLine(x, mAxePosY - kTimelineTickHeight, x, mAxePosY + kTimelineTickHeight);
    QString timeText = time.toString(mTimePeriodScaleSecs >= 24 * 60 * 60? "dd.MM.yyyy": "dd.MM.yyyy hh:mm");
    if (x < mGraphEndX - mTimeRect.width()) {
      QRect timeTickRect(x + kTextMargin, mAxePosY + kTextMargin, mTimeRect.width(), mTimeRect.height());
      painter->drawText(timeTickRect, Qt::AlignLeft | Qt::AlignTop, timeText);
    } else {
      QRect timeTickRect(x - kTextMargin - mTimeRect.width(), mAxePosY + kTextMargin, mTimeRect.width(), mTimeRect.height());
      painter->drawText(timeTickRect, Qt::AlignRight | Qt::AlignTop, timeText);
    }
  }

  if (mCurrentTimePos >= 0) {
    painter->setPen(QPen(QBrush(QColor(Qt::blue)), 3));
    painter->drawLine(QPointF(mGraphPosX + mCurrentTimePos, mAxePosY - kTimelineTickHeight), QPointF(mGraphPosX + mCurrentTimePos, mAxePosY));

    int x = qMax(mGraphPosX + kTextMargin, mGraphPosX + (int)mCurrentTimePos - mTimeRect.width()/2);
    QString timeText = mCurrentTime.toString("dd.MM.yyyy hh:mm");
    if (x < mGraphEndX - mTimeRect.width()) {
      QRect timeTickRect(x, mAxePosY - kTextMargin - mTimeRect.height(), mTimeRect.width(), mTimeRect.height());
      painter->drawText(timeTickRect, Qt::AlignLeft | Qt::AlignBottom, timeText);
    } else {
      QRect timeTickRect(mGraphEndX - kTextMargin - mTimeRect.width(), mAxePosY - kTextMargin - mTimeRect.height(), mTimeRect.width(), mTimeRect.height());
      painter->drawText(timeTickRect, Qt::AlignRight | Qt::AlignBottom, timeText);
    }
  }

  if (!mGraph.isEmpty()) {
    const int kValueCount = 3;
    painter->setPen(QPen(QBrush(QColor(Qt::black)), 2));
    for (int i = 0; i < kValueCount; i++) {
      int y = mGraphPosY - (mGraphPosY - kTextMargin) * i / (kValueCount - 1);
      qreal value = mValueMin + (mValueMax - mValueMin) * i / (kValueCount - 1);
      value *= mValueRescale;
      painter->drawLine(mGraphEndX - kValueTickWidth, y, mGraphEndX + kValueTickWidth, y);
      QString valueText = QString("%1").arg(value, 0, 'f', mValuePrecission);
      if (y < mGraphPosY - mValueRect.height()) {
        QRect valueTickRect(mGraphEndX + kTextMargin, y, mValueRect.width(), mValueRect.height());
        painter->drawText(valueTickRect, Qt::AlignLeft | Qt::AlignTop, valueText);
      } else {
        QRect valueTickRect(mGraphEndX + kTextMargin, mGraphPosY - mValueRect.height(), mValueRect.width(), mValueRect.height());
        painter->drawText(valueTickRect, Qt::AlignLeft | Qt::AlignBottom, valueText);
      }
    }

    if (mCurrentValue >= mValueMin) {
      painter->setPen(QPen(QBrush(QColor(Qt::blue)), 3));
      qreal value = mCurrentValue * mValueRescale;
      qreal y = mCurrentValuePos;
      painter->drawLine(mGraphEndX - kValueTickWidth, y, mGraphEndX + kValueTickWidth, y);
      QString valueText = QString("%1").arg(value, 0, 'f', mValuePrecission);
      QRect valueTickRect(mGraphEndX + kTextMargin, mGraphPosY + kTextMargin, mValueRect.width(), mValueRect.height());
      painter->drawText(valueTickRect, Qt::AlignLeft | Qt::AlignTop, valueText);
    }
  }
}

void GraphWidget::DrawInfo(QPainter* painter)
{
  for (int i = 0; i < mInfo.size(); i++) {
    int y = height() * i / mInfo.size();
    QRect infoRect(kTextMargin, y, mGraphPosX - 2 * kTextMargin, height() / mInfo.size());
    painter->setPen(mInfoColorList.at(i));
    painter->drawText(infoRect, Qt::AlignRight | Qt::AlignVCenter, mInfo.at(i));
  }
}

void GraphWidget::DrawGraph(QPainter* painter)
{
  painter->setClipRect(mGraphPosX, kTextMargin, mGraphEndX - mGraphPosX, mGraphPosY - kTextMargin);
  for (int i = 0; i < mInfo.size(); i++) {
    QBrush unitBrush = QBrush(mInfoColorList.at(i));

    painter->setPen(QPen(unitBrush, 1.0));
    const GraphUnit& graphUnit = mGraph.value(i);
    for (int j = 0; j < graphUnit.size(); j++) {
      const TimePoint& timePoint = graphUnit.at(j);
      qreal timeSecs1 = (timePoint.TimeFrom.toMSecsSinceEpoch() - mTimeBegin.toMSecsSinceEpoch()) / 1000;
      qreal x1 = mGraphPosX + (mGraphEndX - mGraphPosX) * (timeSecs1 - mTimePosSecs) / mTimePeriodSecs * mTimeZoom;
      qreal y1 = mGraphPosY - (mGraphPosY - kTextMargin) * (timePoint.Value - mValueMin) / (mValueMax - mValueMin);
      qreal timeSecs2 = (timePoint.TimeTo.toMSecsSinceEpoch() - mTimeBegin.toMSecsSinceEpoch()) / 1000;
      qreal x2 = mGraphPosX + (mGraphEndX - mGraphPosX) * (timeSecs2 - mTimePosSecs) / mTimePeriodSecs * mTimeZoom;
      qreal y2 = mGraphPosY - (mGraphPosY - kTextMargin) * (timePoint.Value - mValueMin) / (mValueMax - mValueMin);
      painter->drawLine(QPointF(x2, y2), QPointF(x1, y1));
    }
//    for (int j = 0; j < graphUnit.size(); j++) {
//      painter->setPen(QPen(unitBrush, 4.0));
//      const TimePoint& timePoint1 = graphUnit.at(j);
//      qreal timeSecs = (timePoint1.Time.toMSecsSinceEpoch() - mTimeBegin.toMSecsSinceEpoch()) / 1000;
//      qreal x1 = mGraphPosX + (mGraphEndX - mGraphPosX) * timeSecs / mTimePeriodSecs;
//      qreal y1 = mGraphPosY - (mGraphPosY - kTextMargin) * (timePoint1.Value - mValueMin) / (mValueMax - mValueMin);
//      painter->drawPoint(QPointF(x1, y1));

//      if (j > 0) {
//        painter->setPen(QPen(unitBrush, 2.5));
//        const TimePoint& timePoint2 = graphUnit.at(j - 1);
//        qreal x2 = mGraphPosX + (mGraphEndX - mGraphPosX) * timeSecs / mTimePeriodSecs;
//        qreal y2 = mGraphPosY - (mGraphPosY - kTextMargin) * (timePoint2.Value - mValueMin) / (mValueMax - mValueMin);
//        painter->drawLine(QPointF(x2, y2), QPointF(x1, y1));
//      }
//    }
  }
  painter->setClipping(false);
}


GraphWidget::GraphWidget(QWidget* parent)
  : QWidget(parent)
  , mPrecisionSec(60)
  , mGraphPeriodSecs(1), mTimeZoom(1.0), mTimeZoomExp(1.0), mTimePosSecs(0), mTimePeriodSecs(1), mTimePeriodScaleSecs(60)
  , mValueMin(0), mValueMax(1), mValueAbsoluteMin(0), mValueAbsoluteMax(1)
  , mCurrentTimePos(-1)
  , mAxePosY(0), mTimeRect(0, 0, 40, 20), mTimeCount(-1), mGraphPosX(0), mGraphPosY(0)
  , mTimeMoving(false), mTimeMovingBase(0)
{
  setMouseTracking(true);
}
