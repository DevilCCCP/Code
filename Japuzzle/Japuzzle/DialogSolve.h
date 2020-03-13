#pragma once

#include <QDialog>


namespace Ui {
class DialogSolve;
}

class DialogSolve: public QDialog
{
  Ui::DialogSolve* ui;

  Q_OBJECT

public:
  explicit DialogSolve(QWidget* parent = 0);
  ~DialogSolve();

public:
  void ChangeCount(int value, int maximum);

private slots:
  void on_pushButtonCancel_clicked();
};
