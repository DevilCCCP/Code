#pragma once

#include <QList>
#include <QLabel>


class GraphLabel: public QLabel
{
  QVector<uchar>  mSourceValues;
  QVector<uchar>  mSourceMarks;
  int             mWidth;
  int             mHeight;
  QVector<QPoint> mGraphPoints;

public:
  GraphLabel(QWidget* parent = 0);

public:
  void SetLineValues(const QVector<uchar>& values, const QVector<uchar>& marks);

protected:
  virtual void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
//  virtual void mousePressEvent(QMouseEvent* ev) Q_DECL_OVERRIDE;
//  virtual void mouseReleaseEvent(QMouseEvent* ev) Q_DECL_OVERRIDE;
//  virtual void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;
  virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
  void Draw();
};
