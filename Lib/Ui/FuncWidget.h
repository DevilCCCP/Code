#pragma once

#include <QWidget>
#include <QStringList>
#include <QDateTime>
#include <QVector>
#include <QList>


class FuncWidget: public QWidget
{
  QString          mAxeX;
  QString          mAxeY;
  QString          mAxeXValue;
  QString          mAxeYValue;
  bool             mShowMedian;
  bool             mShowAxeX;
  bool             mShowAxeY;

  QVector<QPointF> mValues;
  qreal            mValueXRangeMin;
  qreal            mValueXRangeMax;
  qreal            mValueXMin;
  qreal            mValueXMax;
  qreal            mValueYMin;
  qreal            mValueYMax;
  qreal            mValueYMedian;

  /// Drawning
  QColor           mBackgroundColor;
  QColor           mFuncColor;
  QColor           mAxeColor;
  QRect            mAxeXNameRect;
  QRect            mAxeYNameRect;
  QRect            mAxeXTextRect;
  QRect            mAxeYTextRect;
  int              mAxePosX;
  int              mAxePosY;

  int              mValueXPrecission;
  int              mValueYPrecission;
  qreal            mValueXRescale;
  qreal            mValueYRescale;
  QString          mValueXPrefix;
  QString          mValueYPrefix;

public:
  void SetAxesNames(const QString& _AxeX, const QString& _AxeY, const QString& _AxeXValue, const QString& _AxeYValue);
  void SetShowMedian(bool _ShowMedian);
  void SetShowAxes(bool _ShowAxeX, bool _ShowAxeY);
  void SetBackgroundColor(const QColor& _BackgroundColor);
  void SetFuncColor(const QColor& _FuncColor);
  void SetAxeColor(const QColor& _AxeColor);
  void SetXRange(qreal minX, qreal maxX);

public:
  void SetValues(const QVector<QPointF>& values);

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) override;

private:
  void SetValueScales();
  void SetValueScale(qreal value, int& precission, qreal& rescale, QString& prefix);

  void CalcDrawPositions(QPainter* painter);
  void DrawAxe(QPainter* painter);
  void DrawInfo(QPainter* painter);
  void DrawFunc(QPainter* painter);

signals:
  void TimeSelected(const QDateTime& time);

public:
  FuncWidget(QWidget* parent = 0);
};
