#include <QDateTime>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QToolTip>

#include <Lib/Common/FormatTr.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectLog.h>

#include "ObjectLogWidget.h"


const qreal kPenWidth = 2.0;
const qreal kProcessLineWidth = 2.0;
const qreal kThreadLineWidth = 1.0;
const qreal kLogWidthSecs = 5 * 60;

ObjectLogWidget::ObjectLogWidget(QWidget* parent)
  : QWidget(parent)
{
  setMouseTracking(true);
}

ObjectLogWidget::~ObjectLogWidget()
{
}


void ObjectLogWidget::paintEvent(QPaintEvent* event)
{
  Q_UNUSED(event);

  mWorkInfoMap.clear();
  if (mObjectLogPeriodMap.isEmpty()) {
    return;
  }
  int totalIndexSize = 0;
  for (auto itr = mObjectLogPeriodMap.begin(); itr != mObjectLogPeriodMap.end(); itr++) {
    const ThreadLogPeriodMap& threadLogPeriodMap = itr.value();
    for (auto itr = threadLogPeriodMap.begin(); itr != threadLogPeriodMap.end(); itr++) {
      const WorkLogPeriodMap& workLogPeriodMap = itr.value();
      totalIndexSize += workLogPeriodMap.size();
    }
  }
  mWorkIndex = 0;

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  mWorkHeight = height() / mObjectLogPeriodMap.size();
  qreal totalWidth = width() - kProcessLineWidth;
  qreal totalHeight = height() - kProcessLineWidth;
  mWorkHeight = totalHeight / totalIndexSize - kProcessLineWidth;
  for (int i = 0; i < mObjectList.size(); i++) {
    mObject = mObjectList.at(i);
    ThreadLogPeriodMap* threadLogPeriodMap = &mObjectLogPeriodMap[mObject->Id];
    mHue = 0 + (2*i + 1) * 300 / (2 * mObjectList.size());
    mSaturation = 200;
    mValue = 150;
    int processIndexSize = 0;
    for (auto itr = threadLogPeriodMap->begin(); itr != threadLogPeriodMap->end(); itr++) {
      const WorkLogPeriodMap& workLogPeriodMap = itr.value();
      processIndexSize += workLogPeriodMap.size();
    }
    mProcessRect = QRectF(kProcessLineWidth * 0.5
                       , kProcessLineWidth * 0.5 + (mWorkHeight + kProcessLineWidth) * mWorkIndex
                       , totalWidth, mWorkHeight * processIndexSize);
    PaintObject(&painter, threadLogPeriodMap);
  }
}

void ObjectLogWidget::mouseMoveEvent(QMouseEvent* event)
{
  QPointF point = event->localPos();
  auto itrWork = mWorkInfoMap.lowerBound(point.y());
  if (itrWork != mWorkInfoMap.end()) {
    const WorkInfo& workInfo = itrWork.value();
    if (point.y() > workInfo.WorkRect.top()) {
      auto itrPeriod = workInfo.PeriodInfo.lowerBound(point.x());
      if (itrPeriod != workInfo.PeriodInfo.end()) {
        const WorkPeriodInfo& workPeriodInfo = itrPeriod.value();
        if (point.x() > workPeriodInfo.PeriodRect.left()) {
          QString info = tr("Circles: %1 p/s, Proc: %2%").arg(workPeriodInfo.Circles, 0, 'f', 1).arg(workPeriodInfo.WorkTime, 0, 'f', 2);
          if (workPeriodInfo.LongestWork > 100) {
            info.append(tr(", Long work: %1").arg(FormatTimeTr(workPeriodInfo.LongestWork)));
          }
          info.append(tr("\nTime: %1").arg(workPeriodInfo.Timestamp.toString("dd MMM hh:mm:ss")));
          QToolTip::showText(mapToGlobal(point.toPoint()), info, this, workInfo.WorkRect.toRect(), 60*1000);
          return;
        }
      }
    }
  }
//  QToolTip::hideText();
}

void ObjectLogWidget::SetLogSchema(const LogSchema& _LogSchema)
{
  mLogSchema = _LogSchema;
}

void ObjectLogWidget::SetTimePeriod(const QDateTime& fromTime, const QDateTime& toTime)
{
  mFromTime = fromTime;
  mToTime   = toTime;
}

void ObjectLogWidget::SetObjectLog(const QList<ObjectItemS>& objectList, const QVector<ObjectLogS>& objectLogList)
{
  mObjectList = objectList;

  mObjectSchema.clear();
  mObjectLogPeriodMap.clear();
  foreach (const LogUnitSchema& logUnitSchema, mLogSchema) {
    foreach (const ObjectItemS& object, objectList) {
      if (object->Type == logUnitSchema.ObjectTypeId) {
        mObjectSchema.append(LogObjectSchema(object->Id, logUnitSchema.ThreadName, logUnitSchema.WorkName, logUnitSchema.ViewName));
        mObjectLogPeriodMap[object->Id][logUnitSchema.ThreadName][logUnitSchema.WorkName] = QVector<ObjectLogS>();
      }
    }
  }

  foreach (const ObjectLogS& log, objectLogList) {
    auto itr1 = mObjectLogPeriodMap.find(log->ObjectId);
    if (itr1 == mObjectLogPeriodMap.end()) {
      continue;
    }
    ThreadLogPeriodMap* threadLogPeriodMap = &itr1.value();
    auto itr2 = threadLogPeriodMap->find(log->ThreadName);
    if (itr2 == threadLogPeriodMap->end()) {
      continue;
    }
    WorkLogPeriodMap* workLogPeriodMap = &itr2.value();
    auto itr3 = workLogPeriodMap->find(log->WorkName);
    if (itr3 == workLogPeriodMap->end()) {
      continue;
    }
    LogPeriod* logPeriod = &itr3.value();
    logPeriod->append(log);
  }
}

void ObjectLogWidget::PaintObject(QPainter* painter, ThreadLogPeriodMap* threadLogPeriodMap)
{
  mMainColor.setHsv(mHue, mSaturation, mValue);
  mBorderColor.setHsv(mHue, mSaturation, 100);
  mTextColor.setHsv((mHue + 180) % 360, mSaturation, mValue);
  mErrorColor.setHsv(0, 200, 200);
  mErrorColor.setAlpha(100);

//  painter->setPen(QPen(QBrush(mBorderColor), kProcessLineWidth));
//  painter->setBrush(QBrush(mMainColor));
//  painter->drawRect(mProcessRect);

  qreal totalWidth = mProcessRect.width() - kThreadLineWidth;
  int index = 0;
  for (auto itr = threadLogPeriodMap->begin(); itr != threadLogPeriodMap->end(); itr++) {
    WorkLogPeriodMap* workLogPeriodMap = &itr.value();
    mThreadRect = QRectF(mProcessRect.left() + kThreadLineWidth * 0.5
                         , mProcessRect.top() + kThreadLineWidth * 0.5 + (mWorkHeight + kThreadLineWidth) * index
                         , totalWidth, mWorkHeight * workLogPeriodMap->size());
    PaintThread(painter, itr.key(), workLogPeriodMap);
    index += workLogPeriodMap->size();
  }

  painter->setPen(QPen(QBrush(mTextColor), kPenWidth));
  painter->drawText(mProcessRect.adjusted(kProcessLineWidth, kProcessLineWidth, -kProcessLineWidth, -kProcessLineWidth)
                    , Qt::AlignLeft | Qt::AlignTop, mObject->Name);
}

void ObjectLogWidget::PaintThread(QPainter* painter, const QString& threadName, WorkLogPeriodMap* workLogPeriodMap)
{
  qreal totalWidth = mThreadRect.width() - kThreadLineWidth;
  int index = 0;
  for (auto itr = workLogPeriodMap->begin(); itr != workLogPeriodMap->end(); itr++, index++) {
    LogPeriod* logPeriod = &itr.value();
    mWorkRect = QRectF(mThreadRect.left() + kThreadLineWidth * 0.5
                         , mThreadRect.top() + kThreadLineWidth * 0.5 + (mWorkHeight + kThreadLineWidth) * index
                         , totalWidth, mWorkHeight);

    painter->setPen(QPen(QBrush(mBorderColor), kProcessLineWidth));
    painter->setBrush(QBrush(mMainColor));
    painter->drawRect(mWorkRect);

    QString workName = itr.key();
    mWorkRect.adjust(0.5 * kThreadLineWidth, 0.5 * kThreadLineWidth, -0.5 * kThreadLineWidth, -0.5 * kThreadLineWidth);
    PaintWork(painter, logPeriod);

    painter->setPen(QPen(QBrush(mTextColor), kPenWidth));
    foreach (const LogUnitSchema& logUnitSchema, mLogSchema) {
      if (logUnitSchema.ObjectTypeId == mObject->Type && logUnitSchema.ThreadName == threadName && logUnitSchema.WorkName == workName) {
        painter->drawText(mWorkRect, Qt::AlignLeft | Qt::AlignBottom, logUnitSchema.ViewName);
      }
    }

    mWorkIndex++;
  }
}

void ObjectLogWidget::PaintWork(QPainter* painter, LogPeriod* logPeriod)
{
  if (logPeriod->isEmpty()) {
    painter->setPen(QPen(QBrush(mBorderColor), kProcessLineWidth));
    painter->setBrush(QBrush(mErrorColor));
    painter->drawRect(mWorkRect);
    return;
  }

  WorkInfo workInfo;
  workInfo.WorkRect = mWorkRect;
  auto itrWorkInfoMap = mWorkInfoMap.insert(mWorkRect.bottom(), workInfo);
  mWorkInfo = &itrWorkInfoMap.value();

  qreal maxCircles = 1;
  qreal maxWorkTime = 0.5;
  int   maxLongestWork = 50;
  foreach (const ObjectLogS& log, *logPeriod) {
    if (log->TotalTime <= 0) {
      continue;
    }
    qreal k = 1.0 / log->TotalTime;
    maxCircles = qMax(maxCircles, 1000*k*log->Circles);
    maxWorkTime = qMax(maxWorkTime, 100*k*log->WorkTime);
    maxLongestWork = qMax(maxLongestWork, log->LongestWork);
  }
  qreal topCircles = 1.5 * maxCircles;
  qreal topWorkTime = 1.25 * maxWorkTime;
  qreal topLongestWork = 2.0 * maxLongestWork;
  QColor colorCircles;
  QColor colorWorkTime;
  QColor colorLongestWork;
  colorCircles.setHsv((mHue + 120) % 360, mSaturation, mValue);
  colorWorkTime.setHsv((mHue + 60) % 360, mSaturation, mValue);
  colorWorkTime.setAlpha(200);
  colorLongestWork.setHsv((mHue + 240) % 360, mSaturation, mValue);

  QDateTime firstTime = logPeriod->first()->PeriodStart;
  QDateTime startTime = mFromTime;
  QDateTime nextTime = firstTime > mFromTime.addSecs(kLogWidthSecs)? mFromTime: logPeriod->first()->PeriodEnd;
  qint64 timeLength = mFromTime.msecsTo(mToTime);
  foreach (const ObjectLogS& log, *logPeriod) {
    if (log->TotalTime <= 0) {
      continue;
    }
    if (log->PeriodStart > nextTime) {
      qint64 timeLeft = mFromTime.msecsTo(nextTime);
      qint64 timeRight = mFromTime.msecsTo(log->PeriodStart);
      qreal left = mWorkRect.left() + mWorkRect.width() * timeLeft / timeLength;
      qreal right = mWorkRect.left() + mWorkRect.width() * timeRight / timeLength;
      QRectF missRect(left, mWorkRect.top(), right - left, mWorkRect.height());
      painter->setPen(QPen(QBrush(mErrorColor), 0));
      painter->setBrush(QBrush(mErrorColor));
      painter->drawRect(missRect);
    }

    qint64 timeLeft = mFromTime.msecsTo(log->PeriodStart);
    qint64 timeRight = mFromTime.msecsTo(log->PeriodEnd);
    qreal left = mWorkRect.left() + mWorkRect.width() * timeLeft / timeLength;
    qreal right = mWorkRect.left() + mWorkRect.width() * timeRight / timeLength;
    qreal k = 1.0 / log->TotalTime;
    qreal valueCircles = qMax(1000.0*k*log->Circles, 0.0);
    qreal valueWorkTime = qMax(100.0*k*log->WorkTime, 0.0);
    int   valueLongestWork = qMax(log->LongestWork, 0);
    qreal pointCircles = (topCircles - valueCircles) / topCircles * mWorkRect.height();
    qreal pointWorkTime = (topWorkTime - valueWorkTime) / topWorkTime * mWorkRect.height();
    qreal pointLongestWork = (topLongestWork - valueLongestWork) / topLongestWork * mWorkRect.height();
    QRectF rectWorkTime(left, mWorkRect.top() + pointWorkTime, right - left, mWorkRect.height() - pointWorkTime);
    painter->setPen(QPen(Qt::NoPen));
    painter->setBrush(QBrush(colorWorkTime));
    painter->drawRect(rectWorkTime);
    if (valueLongestWork > 100) {
      painter->setPen(QPen(QBrush(colorLongestWork), 1));
      painter->drawLine(QPointF(left, mWorkRect.top() + pointLongestWork), QPointF(right, mWorkRect.top() + pointLongestWork));
    }
    painter->setPen(QPen(QBrush(colorCircles), 1));
    painter->drawLine(QPointF(left, mWorkRect.top() + pointCircles), QPointF(right, mWorkRect.top() + pointCircles));

    WorkPeriodInfo workPeriodInfo;
    workPeriodInfo.PeriodRect = QRectF(left, mWorkRect.top(), right - left, mWorkRect.height());
    workPeriodInfo.Circles = valueCircles;
    workPeriodInfo.WorkTime = valueWorkTime;
    workPeriodInfo.LongestWork = valueLongestWork;
    workPeriodInfo.Timestamp = log->PeriodStart;
    mWorkInfo->PeriodInfo[right] = workPeriodInfo;

    if (log->PeriodStart > startTime) {
      startTime = log->PeriodStart;
      nextTime = log->PeriodEnd;
    }
  }
  if (nextTime.addSecs(2*kLogWidthSecs) < mToTime) {
    qint64 timeLeft = mFromTime.msecsTo(nextTime);
    qreal left = mWorkRect.left() + mWorkRect.width() * timeLeft / timeLength;
    QRectF missRect(left, mWorkRect.top(), mWorkRect.right() - left, mWorkRect.height());
    painter->setPen(QPen(QBrush(mErrorColor), 0));
    painter->setBrush(QBrush(mErrorColor));
    painter->drawRect(missRect);
  }

  painter->setPen(QPen(QBrush(mTextColor), kPenWidth));
  QString maxText(tr("%1p/s %2").arg(maxCircles, 0, 'f', 1).arg(maxWorkTime, 0, 'f', 2));
  if (maxLongestWork > 100) {
    maxText.append(QString(" (%1)").arg(FormatTimeTr(maxLongestWork)));
  }
  painter->drawText(mWorkRect, Qt::AlignRight | Qt::AlignTop, maxText);
//  painter->drawText(mWorkRect, Qt::AlignRight | Qt::AlignBottom, QString("%1p/s %2").arg(maxCircles, 0, 'f', 2));
}
