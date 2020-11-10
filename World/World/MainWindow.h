#pragma once

#include <QFutureWatcher>
#include <QFileDialog>
#include <QTimer>

#include <Lib/CoreUi/MainWindow2.h>


DefineClassS(Core);

namespace Ui {
class MainWindow;
}

class MainWindow: public MainWindow2
{
  Ui::MainWindow*        ui;
  CoreS                  mCore;

  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();
};
