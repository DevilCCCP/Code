#pragma once

#include <QWidget>
#include <QStringList>
#include <QDateTime>
#include <QVector>
#include <QList>


struct TimePoint {
  QDateTime TimeFrom;
  QDateTime TimeTo;
  qreal     Value;
};
typedef QVector<TimePoint> GraphUnit;
typedef QList<GraphUnit> Graph;

class GraphWidget: public QWidget
{
  QStringList    mInfo;
  QList<QColor>  mInfoColorList;
  QString        mValueScale;
  int            mPrecisionSec;
  Graph          mGraph;

  QDateTime      mGraphBegin;
  QDateTime      mGraphEnd;
  int            mGraphPeriodSecs;
  QDateTime      mTimeBegin;
  QDateTime      mTimeEnd;
  qreal          mTimeZoom;
  qreal          mTimeZoomExp;
  int            mTimePosSecs;
  int            mTimePeriodSecs;
  int            mTimePeriodScaleSecs;

  qreal          mValueMin;
  qreal          mValueMax;
  qreal          mValueAbsoluteMin;
  qreal          mValueAbsoluteMax;

  QDateTime      mCurrentTime;
  qreal          mCurrentTimePos;
  qreal          mCurrentValue;
  qreal          mCurrentValuePos;

  /// Drawning
  int            mAxePosY;
  QRect          mTimeRect;
  QRect          mValueRect;
  int            mTimeCount;
  int            mGraphPosX;
  int            mGraphPosY;
  int            mGraphEndX;

  int            mValuePrecission;
  qreal          mValueRescale;
  QString        mValuePrefix;

  bool           mTimeMoving;
  qreal          mTimeMovingBase;
  int            mTimePosMovingBase;

  Q_OBJECT

public:
  void SetAutoInfo(const QStringList& _Info, const QString& _ValueScale);
  void SetInfo(const QStringList& _Info, const QList<QColor>& _InfoColorList, const QString& _ValueScale);
  void SetValueBounds(qreal _ValueMin, qreal _ValueMax);
  void SetGraph(const Graph& _Graph);
  const Graph& GetGraph() const { return mGraph; }
  void SetPrecision(int _PrecisionSec);

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) override;
//  /*override */virtual void resizeEvent(QResizeEvent* event) override;

  /*override */virtual void mouseMoveEvent(QMouseEvent* event) override;
  /*override */virtual void mousePressEvent(QMouseEvent* event) override;
  /*override */virtual void mouseReleaseEvent(QMouseEvent* event) override;
  /*override */virtual void leaveEvent(QEvent* event) override;


  /*override */virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
  /*override */virtual void wheelEvent(QWheelEvent* event) override;

private:
  void CalcDrawPositions(QPainter* painter);
  void DrawAxe(QPainter* painter);
  void DrawInfo(QPainter* painter);
  void DrawGraph(QPainter* painter);

signals:
  void TimeSelected(const QDateTime& time);

public:
  GraphWidget(QWidget* parent = 0);
};
