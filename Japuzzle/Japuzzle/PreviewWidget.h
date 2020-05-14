#pragma once

#include <QWidget>
#include <QPixmap>


class PreviewWidget: public QWidget
{
  QPixmap mBackground;
  QRect   mFullRect;
  QRect   mLocation;

  Q_OBJECT

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

  /*override */virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

public:
  void Setup();
  void SetLocation(const QRect& _Location);
  void UpdateFull();
  QSize DefaultSize() const;

private:
  void DrawPreview();
  void DrawPicture(QPainter* painter);
  void DrawLocation(QPainter* painter);

signals:
  void MoveToLocation(int i, int j);

public:
  PreviewWidget(QWidget* parent = 0);
};
