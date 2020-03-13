#pragma once

#include <QList>
#include <QLabel>
#include <QSettings>


class ImageLabel: public QLabel
{
  QImage          mSourceImage;
  int             mWidth;
  int             mHeight;
  int             mScaleAngle;
  QPoint          mCenterPos;

  QList<QPointF>  mLinePoints;
  int             mCurrentPoint;

  Q_OBJECT

public:
  ImageLabel(QWidget* parent = 0);

public:
  const QList<uchar> LineValues() const;
  void SyncLine(QSettings* settings) const;
  void RestoreLine(QSettings* settings);

protected:
  virtual void mouseMoveEvent(QMouseEvent* ev) Q_DECL_OVERRIDE;
  virtual void mousePressEvent(QMouseEvent* ev) Q_DECL_OVERRIDE;
  virtual void mouseReleaseEvent(QMouseEvent* ev) Q_DECL_OVERRIDE;
  virtual void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;

public:
  void SetImage(const QImage& image);
  void SetX(int value);
  void SetY(int value);

private:
  QString LineVarname(int i) const;
  QPoint ToScreen(const QPointF& p) const;
  QPoint ToSource(const QPointF& p) const;
  QPointF ToPercent(const QPoint& p) const;

private:
  void UpdateImage();
  void DrawImage();

signals:
  void OnMove(int x, int y);
  void OnScale(int scale);
};
