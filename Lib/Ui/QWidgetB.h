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
  QSize GetImageSize() { return mBackImage.isValid()? mBackImage.size(): 0; }

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

public slots:
  void SetBackImage(const QImage& _BackImage);

public:
  explicit QWidgetB(QWidget* parent = 0, Qt::WindowFlags f = 0);
  /*override */ virtual ~QWidgetB();
};
