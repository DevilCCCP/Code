#pragma once

#include <QLineEdit>


class QLineEdit2: public QLineEdit
{
  Q_OBJECT

  /*override */virtual void focusOutEvent(QFocusEvent* event);

signals:
  void FocusLost();

public:
  QLineEdit2(QWidget* parent = 0);
};

