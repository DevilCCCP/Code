#pragma once

#include <QVector>
#include <QPoint>

#include <Lib/CoreUi/FormImageRegion.h>
#include "FormImageLineView.h"


class FormImageLineRegion: public FormImageRegion
{
  FormImageLineView::EMode       mMode;
  PROPERTY_GET(QVector<QPointF>, LinePoints)
  int                            mCurrentPoint;

  Q_OBJECT

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

  /*override */virtual void mouseDoubleClickEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

public:
  void SetNone();
  void SetLine(const QVector<QPointF>& linePoints);
  void SetRegion(const QVector<QPointF>& linePoints);
  const QVector<QPoint> LinePoints() const;
  const QList<uchar> LineValues() const;
  const QVector<uchar> RectValues() const;

  void MoveLineX(int x);
  void MoveLineY(int y);

private:
  QPointF ToScreen(const QPointF& p) const;
  QPointF FromScreen(const QPointF& p) const;
  QPoint  ToSource(const QPointF& p) const;
  QPointF FromSource(const QPoint& p) const;

signals:
  void LineChanged();

public:
  FormImageLineRegion(QWidget* parent = 0);
};
