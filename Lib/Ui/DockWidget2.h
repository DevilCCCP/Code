#pragma once

#include <QDockWidget>


class DockWidget2: public QDockWidget
{
  Q_OBJECT

public:
  DockWidget2(QWidget* parent = 0, Qt::WindowFlags flags = 0);

protected:
  /*override */virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void moveEvent(QMoveEvent* event) Q_DECL_OVERRIDE;

signals:
  void OnWindowChanged();
};
