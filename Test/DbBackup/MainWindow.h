#pragma once

#include <QMainWindow>


namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow
{
  Ui::MainWindow* ui;

  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();
};
