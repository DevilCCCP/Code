#pragma once

#include <QDialog>


namespace Ui {
class DialogAbout;
}

class DialogAbout: public QDialog
{
  Ui::DialogAbout* ui;

  Q_OBJECT

public:
  explicit DialogAbout(QWidget* parent = 0);
  ~DialogAbout();
};
