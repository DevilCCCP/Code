#include <QWheelEvent>

#include "QScrollAreaZoom.h"


void QScrollAreaZoom::wheelEvent(QWheelEvent* event)
{
  if (event->modifiers().testFlag(Qt::ControlModifier)) {
    event->ignore();
    return;
  }

  QScrollArea::wheelEvent(event);
}


QScrollAreaZoom::QScrollAreaZoom(QWidget* parent)
  : QScrollArea(parent)
{
}

