#pragma once

#include <QWidget>
#include <QImage>


namespace Ui {
class FormImageView;
}
class FormImageRegion;

class FormImageView: public QWidget
{
  Ui::FormImageView* ui;
  FormImageRegion*   mImageRegion;

  bool               mBestScale;
  qreal              mScale;
  QPoint             mScaleCenter;
  QPoint             mMousePressPos;
  QPoint             mMousePressCenter;
  bool               mManual;

  Q_OBJECT

public:
  explicit FormImageView(QWidget* parent = 0);
  ~FormImageView();

protected:
  /*override */virtual void resizeEvent(QResizeEvent* event) override;
//  /*override */virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
  /*override */virtual void mouseMoveEvent(QMouseEvent* event) override;
  /*override */virtual void mousePressEvent(QMouseEvent* event) override;
  /*override */virtual void mouseReleaseEvent(QMouseEvent* event) override;
  /*override */virtual void wheelEvent(QWheelEvent* event) override;

protected:
  void SetImageRegion(FormImageRegion* _ImageRegion);

public:
  void SetImage(const QImage& image);
  void SetScale(int value);
  void SetBestScale();

private:
  void SelectScale(int value);
  void CalcBestScale();
  void ApplyScale();

private:
  void OnHorizontalMoved(int value);
  void OnVerticalMoved(int value);

private slots:
  void on_horizontalSliderScale_sliderMoved(int value);
  void on_actionScaleIn_triggered();
  void on_actionScaleOut_triggered();
  void on_actionScaleHome_triggered();
  void on_actionScaleBest_toggled(bool on);
};
