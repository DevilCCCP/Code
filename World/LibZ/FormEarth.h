#pragma once

#include <QWidget>
#include <QVector>
#include <QPointF>

#include "EarthLandscape.h"


namespace Ui {
class FormEarth;
}

class FormEarth: public QWidget
{
  Ui::FormEarth*   ui;

  bool             mShowAxes;
  bool             mMinScale;
  qreal            mDefaultCamTheta;
  qreal            mDefaultCamPhi;
  qreal            mDefaultCamH;
  QColor           mMainColor;
  QColor           mAxeColor;

  EarthLandscape   mLandscape;
  EarthLandscape   mObjects;

  qreal            mCamTheta;
  qreal            mCamPhi;
  qreal            mCamH;
  int              mZoomDelta;
  QPointF          mCamAnchor;
  QPoint           mMoveAnchor;
  QPoint           mMoved;
  bool             mMoving;

  QPointF          mCenter;
  qreal            mPlaneScale;
  qreal            mPlaneRadius;
  QVector<QPointF> mPlaneBorder;
  qreal            mZ0;
  qreal            mThetaD;
  qreal            mPhiD;
  qreal            mAxeThetaD;
  qreal            mAxePhiD;

  QPixmap          mBackBuffer;
  bool             mUpdated;

  Q_OBJECT

public:
  explicit FormEarth(QWidget* parent = 0);
  ~FormEarth();

public:
  void SetShowAxes(bool show) { mShowAxes = show; update(); }
  void SetMainColor(const QColor& mainColor) { mMainColor = mainColor; update(); }
  void SetDefaultCamTheta(qreal value) { mDefaultCamTheta = value; }
  void SetDefaultCamPhi(qreal value) { mDefaultCamPhi = value; }
  void SetDefaultCamH(qreal value) { mDefaultCamH = value; }

  void SetLandscape(const EarthLandscape& landscape) { mLandscape = landscape; update(); }

protected:
  /*override */virtual void resizeEvent(QResizeEvent* event) override;

  /*override */virtual void mouseMoveEvent(QMouseEvent* event) override;
  /*override */virtual void mousePressEvent(QMouseEvent* event) override;
  /*override */virtual void mouseReleaseEvent(QMouseEvent* event) override;
  /*override */virtual void wheelEvent(QWheelEvent* event) override;

  /*override */virtual void paintEvent(QPaintEvent* event) override;

private:
  void Update();
  void UpdatePrepare(int w, int h);
  void UpdateValue(int& value, int newValue);
  void UpdateValue(qreal& value, qreal newValue);

  void Draw();
  void DrawBackground(QPainter* painter);
  void DrawLandscape(QPainter* painter, const EarthLandscape& landscape);
  void DrawPlate(QPainter* painter, const QVector<QPointF>& border);
  void PlateConnectBorder(int fromIndex, int toIndex, QVector<QPointF>& sborder);
  int FindBorderIndex(const QPointF& p);
  void DrawAxes(QPainter* painter);

  bool TranslateToPlane(const QPointF& p, QPointF& ps);
  bool TranslateToPlaneRotate(const QPointF& p, QPointF& ps, QPoint& rotate);

  void Zoom(int k);
  void MovePhi(qreal newCamPhi);

private slots:
  void on_actionZoomIn_triggered();
  void on_actionZoomOut_triggered();
  void on_actionZoomDefault_triggered();
  void on_actionMoveLeft_triggered();
  void on_actionMoveRight_triggered();
  void on_actionMoveUp_triggered();
  void on_actionMoveDown_triggered();
  void on_toolButtonMoveDebug_clicked();
};
