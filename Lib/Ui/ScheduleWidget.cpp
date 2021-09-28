#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QToolTip>

#include "ScheduleWidget.h"


const int kTextMargin = 4;
const int kTimelineTickHeight = 4;

void ScheduleWidget::SetInfo(const QStringList& _Info, const QList<QColor>& _InfoColorList)
{
  mInfo = _Info;
  mInfoColorList = _InfoColorList;
  if (mSchedule.size() != mInfo.size()) {
    bool noSchedule = mSchedule.isEmpty();
    mSchedule.resize(mInfo.size());
    if (!noSchedule) {
      emit ScheduleChanged();
    }
  }
}

void ScheduleWidget::SetSchedule(const Schedule& _Schedule)
{
  mSchedule = _Schedule;

  update();
}

void ScheduleWidget::SetPrecision(int _PrecisionSec)
{
  mPrecisionSec = qMax(1, _PrecisionSec);

  update();
}

void ScheduleWidget::paintEvent(QPaintEvent* event)
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

void ScheduleWidget::mouseMoveEvent(QMouseEvent* event)
{
  qreal posX = event->localPos().x();

  int newTime = -1;
  qreal newTimePos = -1;
  if (posX >= mGraphPosX) {
    int timePoint = qRound((24.0 * 60 * 60) / mPrecisionSec * (posX - mGraphPosX) / (mGraphEndX - mGraphPosX));
    timePoint = qBound(0, timePoint, (24 * 60 * 60) / mPrecisionSec);
    newTime = timePoint * mPrecisionSec;
    newTimePos = (mGraphEndX - mGraphPosX) * (qreal)newTime / (24.0 * 60 * 60);
  }

  if (newTime != mCurrentTime) {
    mCurrentTime = newTime;
    mCurrentTimePos = newTimePos;

    emit ChangeCurrentTime(mCurrentTime);

    update();
  }

  qreal posY = event->localPos().y();
  int newCurrentObject = (mCurrentTime >= 0 && posY >= 0 && posY < mGraphPosY)? (int)(mInfo.size() * posY / mGraphPosY): -1;
  if (newCurrentObject != mCurrentObject) {
    mCurrentObject = newCurrentObject;

    emit ChangeCurrentObject(mCurrentObject);

    update();
  }
}

void ScheduleWidget::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    if (mEditAction == eNoEdit) {
      if (mCurrentTime >= 0 && mCurrentObject >= 0) {
        mEditAction = ePointClick;
        mEditTime = mCurrentTime;
        mEditObject = mCurrentObject;

        update();
      }
    } else if (mEditAction == eTwoPoints) {
      if (mCurrentTime >= 0 && mCurrentObject >= 0 && mCurrentTime != mEditTime && mCurrentObject == mEditObject) {
        AddPeriod(mCurrentObject, mCurrentTime, mEditTime);
      } else {
        mEditAction = eNoEdit;
      }
      update();
    }
  } else if (mEditAction != eNoEdit) {
    mEditAction = eNoEdit;

    update();
  }
}

void ScheduleWidget::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    if (mEditAction == ePointClick) {
      if (mCurrentTime == mEditTime && mCurrentObject == mEditObject) {
        mEditAction = eTwoPoints;

        update();
        return;
      }
    }
  }

  if (mEditAction != eNoEdit) {
    mEditAction = eNoEdit;

    update();
  }
}

void ScheduleWidget::leaveEvent(QEvent* event)
{
  Q_UNUSED(event);

  if (mCurrentTime != -1) {
    mCurrentTime = -1;
    mCurrentTimePos = -1;
    mCurrentObject = -1;
    mEditAction = eNoEdit;

    emit ChangeCurrentTime(mCurrentTime);
    emit ChangeCurrentObject(mCurrentObject);

    update();
  }
}

void ScheduleWidget::AddPeriod(int object, int time1, int time2)
{
  if (object < 0 || object >= mSchedule.size()) {
    return;
  }

  if (time1 > time2) {
    qSwap(time1, time2);
  }

  CutPeriod(time1, time2);

  TimePeriodList* timeList = &mSchedule[object];
  auto insertItr = timeList->begin();
  for (; insertItr != timeList->end(); insertItr++) {
    TimePeriod* timePeriod = &*insertItr;
    if (time1 == timePeriod->EndSec) {
      timePeriod->EndSec = time2;
      auto nextInsertItr = insertItr;
      nextInsertItr++;
      if (nextInsertItr != timeList->end()) {
        TimePeriod* nextTimePeriod = &*nextInsertItr;
        if (nextTimePeriod->BeginSec == timePeriod->EndSec) {
          timePeriod->EndSec = nextTimePeriod->EndSec;
          timeList->erase(nextInsertItr);
        }
      }
      break;
    } else if (time2 == timePeriod->BeginSec) {
      timePeriod->BeginSec = time1;
      break;
    } else if (time2 < timePeriod->BeginSec) {
      break;
    }
  }
  if (insertItr != timeList->end()) {
    if (time2 < insertItr->BeginSec) {
      timeList->insert(insertItr, TimePeriod(time1, time2));
    }
  } else {
    timeList->append(TimePeriod(time1, time2));
  }

  emit ScheduleChanged();
}

void ScheduleWidget::CutPeriod(int time1, int time2)
{
  if (time1 > time2) {
    qSwap(time1, time2);
  }

  for (int i = 0; i < mSchedule.size(); i++) {
    TimePeriodList* timeList = &mSchedule[i];
    for (auto itr = timeList->begin(); itr != timeList->end(); ) {
      TimePeriod* timePeriod = &*itr;
      if (timePeriod->BeginSec >= time2) {
        break;
      } else if (timePeriod->EndSec <= time1) {
        itr++;
        continue;
      } else {
        if (timePeriod->BeginSec < time1) {
          if (timePeriod->EndSec > time2) {
            itr = timeList->insert(itr, TimePeriod(timePeriod->BeginSec, time1));
            itr++;
            timePeriod = &*itr;
            timePeriod->BeginSec = time2;
          } else {
            timePeriod->EndSec = time1;
          }
          itr++;
        } else if (timePeriod->EndSec > time2) {
          timePeriod->BeginSec = time2;
          break;
        } else {
          itr = timeList->erase(itr);
        }
      }
    }
  }
}

void ScheduleWidget::CalcDrawPositions(QPainter* painter)
{
  const QString testTime("00:00");
  QFontMetrics fontMetrics = painter->fontMetrics();
  mTimeRect = fontMetrics.boundingRect(testTime);

  mGraphPosX = 0;
  for (const QString& name: mInfo) {
    QFontMetrics fontMetrics = painter->fontMetrics();
    mGraphPosX = qMax(mGraphPosX, fontMetrics.boundingRect(name).width() + 2 * kTextMargin);
  }
  mGraphEndX = width() - kTextMargin;

  mGraphPosY = height() - 2 * (mTimeRect.height() + 2 * kTextMargin);
  mAxePosY = height() - (mTimeRect.height() + 2 * kTextMargin);
  int timeWidth = (mTimeRect.width() + 2 * kTextMargin) * 2;
  mTimeCount = (mGraphEndX - mGraphPosX) / timeWidth;
  if (mTimeCount > 48) {
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

void ScheduleWidget::DrawAxe(QPainter* painter)
{
  painter->setPen(QPen(QBrush(QColor(Qt::black)), 2));
  painter->drawLine(mGraphPosX, mAxePosY, mGraphEndX, mAxePosY);
  for (int i = 1; i < 2*mTimeCount; i += 2) {
    int x = mGraphPosX + (mGraphEndX - mGraphPosX) * i / (2*mTimeCount);
    painter->drawLine(x, mAxePosY - kTimelineTickHeight/3, x, mAxePosY + kTimelineTickHeight/3);
  }
  for (int i = 0; i <= mTimeCount; i++) {
    int x = mGraphPosX + (mGraphEndX - mGraphPosX) * i / mTimeCount;
    int minutes = (24 * 60) * i / mTimeCount;
    painter->drawLine(x, mAxePosY - kTimelineTickHeight/2, x, mAxePosY + kTimelineTickHeight/2);
    QString timeText = QString("%1:%2").arg(minutes / 60, 2, 10, QChar('0')).arg(minutes % 60, 2, 10, QChar('0'));
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
    painter->drawLine(QPointF(mGraphPosX + mCurrentTimePos, mAxePosY - kTimelineTickHeight/3), QPointF(mGraphPosX + mCurrentTimePos, mAxePosY));

    int x = mGraphPosX + (mGraphEndX - mGraphPosX) * mCurrentTime / (24 * 60 * 60);
    int minutes = mCurrentTime / 60;
    QString timeText = QString("%1:%2").arg(minutes / 60, 2, 10, QChar('0')).arg(minutes % 60, 2, 10, QChar('0'));
    if (x < mGraphEndX - mTimeRect.width()) {
      QRect timeTickRect(x + kTextMargin, mAxePosY - kTextMargin - mTimeRect.height(), mTimeRect.width(), mTimeRect.height());
      painter->drawText(timeTickRect, Qt::AlignLeft | Qt::AlignBottom, timeText);
    } else {
      QRect timeTickRect(mGraphEndX - kTextMargin - mTimeRect.width(), mAxePosY - kTextMargin - mTimeRect.height(), mTimeRect.width(), mTimeRect.height());
      painter->drawText(timeTickRect, Qt::AlignRight | Qt::AlignBottom, timeText);
    }
  }
}

void ScheduleWidget::DrawInfo(QPainter* painter)
{
  QFont font = painter->font();
  for (int i = 0; i < mInfo.size(); i++) {
    int y = mGraphPosY * i / mInfo.size();
    QRect infoRect(kTextMargin, y, mGraphPosX - 2 * kTextMargin, mGraphPosY / mInfo.size());
    painter->setPen(mInfoColorList.at(i));
    font.setBold(i == mCurrentObject);
    painter->setFont(font);
    painter->drawText(infoRect, Qt::AlignRight | Qt::AlignVCenter, mInfo.at(i));
  }
}

void ScheduleWidget::DrawGraph(QPainter* painter)
{
  for (int i = 0; i < mInfo.size(); i++) {
    qreal y = mGraphPosY * (i + 0.5) / mInfo.size();
    painter->setPen(QPen(QBrush(mInfoColorList.at(i)), 0.5));
    painter->drawLine(QPointF(mGraphPosX, y), QPointF(mGraphEndX, y));

    const TimePeriodList& timeList = mSchedule.value(i);
    for (auto itr = timeList.constBegin(); itr != timeList.constEnd(); itr++) {
      const TimePeriod& timePeriod = *itr;

      int x1 = (mGraphEndX - mGraphPosX) * (qreal)timePeriod.BeginSec / (24.0 * 60 * 60);
      int x2 = (mGraphEndX - mGraphPosX) * (qreal)timePeriod.EndSec / (24.0 * 60 * 60);
      painter->setPen(QPen(QBrush(mInfoColorList.at(i)), 2.5));
      painter->drawLine(QPointF(mGraphPosX + x1, y), QPointF(mGraphPosX + x2, y));

      painter->setPen(QPen(QBrush(mInfoColorList.at(i)), 4.0));
      painter->drawPoint(QPointF(mGraphPosX + x1, y));
      painter->drawPoint(QPointF(mGraphPosX + x2, y));
    }
  }

  switch (mEditAction) {
  case eNoEdit: break;
  case ePointClick:
    if (mEditObject >= 0 && mEditTime >= 0) {
      DrawGraphPoint(painter);
    }
    break;
  case eTwoPoints:
    if (mEditObject >= 0 && mEditTime >= 0) {
      if (mCurrentObject == mEditObject && mCurrentTime != mEditTime) {
        DrawGraphLine(painter);
      } else {
        DrawGraphPoint(painter);
      }
    }
    break;
  }
}

void ScheduleWidget::DrawGraphPoint(QPainter* painter)
{
  qreal y = mGraphPosY * (mEditObject + 0.5) / mInfo.size();
  int x1 = (mGraphEndX - mGraphPosX) * (qreal)mEditTime / (24.0 * 60 * 60);
  QPen pointPen(QBrush(mInfoColorList.at(mEditObject)), 8.0);
  pointPen.setCapStyle(Qt::RoundCap);
  painter->setPen(pointPen);
  painter->drawPoint(QPointF(mGraphPosX + x1, y));
}

void ScheduleWidget::DrawGraphLine(QPainter* painter)
{
  if (mCurrentObject != mEditObject) {
    return;
  }

  qreal y = mGraphPosY * (mEditObject + 0.5) / mInfo.size();
  int x1 = (mGraphEndX - mGraphPosX) * (qreal)mEditTime / (24.0 * 60 * 60);
  int x2 = (mGraphEndX - mGraphPosX) * (qreal)mCurrentTime / (24.0 * 60 * 60);
  QPen pointPen(QBrush(mInfoColorList.at(mEditObject)), 8.0);
  pointPen.setCapStyle(Qt::RoundCap);
  painter->setPen(pointPen);
  painter->drawLine(QPointF(mGraphPosX + x1, y), QPointF(mGraphPosX + x2, y));
}

ScheduleWidget::ScheduleWidget(QWidget* parent)
  : QWidget(parent)
  , mPrecisionSec(60)
  , mEditAction(eNoEdit), mEditTime(0), mEditObject(0), mCurrentTime(-1), mCurrentObject(-1), mCurrentTimePos(-1)
  , mAxePosY(0), mTimeRect(0, 0, 40, 20), mTimeCount(2), mGraphPosX(0), mGraphPosY(0)
{
  setMouseTracking(true);
}
