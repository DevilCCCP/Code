#pragma once

#include <QWidget>
#include <QImage>


class QWidgetB: public QWidget
{
public:
  enum FillStyle {
    eStretch,
    eCopy,
  };

private:
  QImage    mBackImage;
  FillStyle mFillStyle;

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) override;
  /*override */virtual void resizeEvent(QResizeEvent* event) override;

public:
  void SetFillStyle(FillStyle _FillStyle);
  void SetBackImage(const QImage& _BackImage);

protected:
  void DrawRectStretch(const QRectF& rect);
  void DrawRectCopy(const QRectF& rect);

public:
  explicit QWidgetB(QWidget* parent = 0, Qt::WindowFlags f = 0);
  /*override */ virtual ~QWidgetB();
};
