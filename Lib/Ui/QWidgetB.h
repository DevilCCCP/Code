#pragma once

#include <QWidget>
#include <QImage>


class QWidgetB: public QWidget
{
  QImage mBackImage;

public:
  enum FillStyle {
    eStretch
  };

public:
  QSize GetImageSize() { return !mBackImage.isNull()? mBackImage.size(): QSize(0, 0); }

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) override;
  /*override */virtual void resizeEvent(QResizeEvent* event) override;

public slots:
  void SetBackImage(const QImage& _BackImage);

public:
  explicit QWidgetB(QWidget* parent = 0, Qt::WindowFlags f = Qt::Widget);
  /*override */ virtual ~QWidgetB();
};
