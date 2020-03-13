#include <QFocusEvent>

#include "QLineEdit2.h"


void QLineEdit2::focusOutEvent(QFocusEvent* event)
{
  emit FocusLost();

  QLineEdit::focusOutEvent(event);
}


QLineEdit2::QLineEdit2(QWidget* parent)
  : QLineEdit(parent)
{
}

