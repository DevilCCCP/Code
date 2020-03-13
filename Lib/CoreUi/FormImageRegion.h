#pragma once

#include <QWidget>
#include <QPoint>

#include <Lib/Include/Common.h>


class FormImageRegion: public QWidget
{
  PROPERTY_GET(QImage,  Image)
  PROPERTY_GET(qreal,   Scale)

  Q_OBJECT

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

//  /*override */virtual void mouseDoubleClickEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
//  /*override */virtual void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
//  /*override */virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
//  /*override */virtual void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

public:
  void SetImage(const QImage& image);
  void SetScale(qreal scale);

private:
  void Resize();

signals:
  void Moved(int x, int y);

public:
  explicit FormImageRegion(QWidget* parent = 0);
};
