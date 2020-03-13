#pragma once

#include <QWidget>

#include <Lib/Db/Db.h>

#include "LogSchema.h"


class ObjectLogWidget: public QWidget
{
  LogSchema          mLogSchema;
  ObjectSchema       mObjectSchema;
  QDateTime          mFromTime;
  QDateTime          mToTime;
  QList<ObjectItemS> mObjectList;
  ObjectLogPeriodMap mObjectLogPeriodMap;

  ObjectItemS        mObject;
  int                mHue;
  int                mSaturation;
  int                mValue;
  qreal              mWorkHeight;
  int                mWorkIndex;
  QRectF             mProcessRect;
  QRectF             mThreadRect;
  QRectF             mWorkRect;
  QColor             mMainColor;
  QColor             mBorderColor;
  QColor             mTextColor;
  QColor             mErrorColor;

  struct WorkPeriodInfo {
    QRectF PeriodRect;
    qreal  Circles;
    qreal  WorkTime;
    int    LongestWork;
  };
  struct WorkInfo {
    QRectF                      WorkRect;
    QMap<qreal, WorkPeriodInfo> PeriodInfo;
  };

  typedef QMap<qreal, WorkInfo> WorkInfoMap;
  WorkInfoMap         mWorkInfoMap;
  WorkInfo*           mWorkInfo;

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

public:
  void SetLogSchema(const LogSchema& _LogSchema);
  void SetTimePeriod(const QDateTime& fromTime, const QDateTime& toTime);
  void SetObjectLog(const QList<ObjectItemS>& objectList, const QVector<ObjectLogS>& objectLogList);

private:
  void PaintObject(QPainter* painter, ThreadLogPeriodMap* threadLogPeriodMap);
  void PaintThread(QPainter* painter, const QString& threadName, WorkLogPeriodMap* workLogPeriodMap);
  void PaintWork(QPainter* painter, LogPeriod* logPeriod);

public:
  explicit ObjectLogWidget(QWidget* parent = 0);
  ~ObjectLogWidget();
};
