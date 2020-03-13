#pragma once

#include <QWidget>
#include <QStringList>

#include <Lib/Include/Schedule.h>


class ScheduleWidget: public QWidget
{
  QStringList    mInfo;
  QList<QColor>  mInfoColorList;
  int            mPrecisionSec;
  Schedule       mSchedule;
  Schedule       mEditSchedule;

  enum EditAction {
    eNoEdit,
    ePointClick,
    eTwoPoints,
//    eCreatingLine,
//    eMovingLine,
//    eChangingBegin,
//    eChangingEnd
  };
  EditAction     mEditAction;
  int            mEditTime;
  int            mEditObject;
  int            mCurrentTime;
  int            mCurrentObject;
  qreal          mCurrentTimePos;

  /// Drawning
  int            mAxePosY;
  QRect          mTimeRect;
  QRect          mTimeExRect;
  int            mTimeCount;
  int            mGraphPosX;
  int            mGraphPosY;
  int            mGraphEndX;

  Q_OBJECT

public:
  void SetInfo(const QStringList& _Info, const QList<QColor>& _InfoColorList);
  void SetSchedule(const Schedule& _Schedule);
  const Schedule& GetSchedule() const { return mSchedule; }
  void SetPrecision(int _PrecisionSec);

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) override;
//  /*override */virtual void resizeEvent(QResizeEvent* event) override;

  /*override */virtual void mouseMoveEvent(QMouseEvent* event) override;
  /*override */virtual void mousePressEvent(QMouseEvent* event) override;
  /*override */virtual void mouseReleaseEvent(QMouseEvent* event) override;
  /*override */virtual void leaveEvent(QEvent* event) override;

private:
  void AddPeriod(int object, int time1, int time2);
  void CutPeriod(int time1, int time2);

  void CalcDrawPositions(QPainter* painter);
  void DrawAxe(QPainter* painter);
  void DrawInfo(QPainter* painter);
  void DrawGraph(QPainter* painter);
  void DrawGraphPoint(QPainter* painter);
  void DrawGraphLine(QPainter* painter);

signals:
  void ScheduleChanged();
  void ChangeCurrentObject(int object);
  void ChangeCurrentTime(int secs);

public:
  ScheduleWidget(QWidget* parent = 0);
};
