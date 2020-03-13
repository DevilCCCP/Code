#include "DockWidget2.h"


DockWidget2::DockWidget2(QWidget* parent, Qt::WindowFlags flags)
  : QDockWidget(parent, flags)
{
}


void DockWidget2::resizeEvent(QResizeEvent* event)
{
  QDockWidget::resizeEvent(event);

  emit OnWindowChanged();
}

void DockWidget2::moveEvent(QMoveEvent* event)
{
  QDockWidget::moveEvent(event);

  emit OnWindowChanged();
}
