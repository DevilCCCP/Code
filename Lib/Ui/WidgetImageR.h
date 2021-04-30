#pragma once

#include <QList>
#include <QRect>
#include <QColor>

#include "QWidgetB.h"


class WidgetImageR: public QWidgetB
{
  QList<QRect>  mRectList;
  QList<QColor> mColorList;
  int           mRectWidth;

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

public:
  void SetRectColor(const QColor& color);
  void SetRectWidth(int width);

public slots:
  void SetImageRect(const QRect& rect);
  void SetImageRect(const QRect& rect, const QColor& color);
  void SetImageRectList(const QList<QRect>& rectList);
  void SetImageRectList(const QList<QRect>& rectList, const QColor& color);
  void SetImageRectList(const QList<QRect>& rectList, const QList<QColor>& colorList);

public:
  explicit WidgetImageR(QWidget* parent = 0, Qt::WindowFlags f = 0);
};
