#pragma once

#include <QWidget>
#include <QPixmap>


class PreviewWidget: public QWidget
{
  QPixmap mBackground;
  QPixmap mPreview;
  QRect   mLocation;

  Q_OBJECT

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

public:
  void Setup();
  void SetLocation(const QRect& _Location);
  void UpdateFull();
  QSize DefaultSize() const;

private:
  void DrawPreview();
  void DrawLocation();

public:
  PreviewWidget(QWidget* parent = 0);
};
