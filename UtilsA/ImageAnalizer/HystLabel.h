#pragma once

#include <QList>
#include <QLabel>

#include <LibA/Analyser/Hyst.h>


class HystLabel: public QLabel
{
  Hyst            mHyst;
  int             mWidth;
  int             mHeight;

public:
  HystLabel(QWidget* parent = 0);

public:
  void SetHyst(const Hyst& _Hyst);
  void SetHystFast(const HystFast& _Hyst);

protected:
  virtual void mouseMoveEvent(QMouseEvent* ev) Q_DECL_OVERRIDE;
//  virtual void mousePressEvent(QMouseEvent* ev) Q_DECL_OVERRIDE;
//  virtual void mouseReleaseEvent(QMouseEvent* ev) Q_DECL_OVERRIDE;
//  virtual void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;
  virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
  void Draw();
};
