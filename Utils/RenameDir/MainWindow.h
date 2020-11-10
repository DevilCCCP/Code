#pragma once

#include <QMainWindow>

#include "RenameWorker.h"


namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow
{
  Ui::MainWindow* ui;

  RenameWorker*   mRenameWorker;

  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

private:
  void OnWorkFinished();
  void OnWorkProgress(int percent);

private slots:
  void on_actionBrowseFolder_triggered();
  void on_checkBoxRename_toggled(bool checked);
  void on_checkBoxNumbers_toggled(bool checked);
  void on_checkBoxSecondNumber_toggled(bool checked);
  void on_pushButtonStart_clicked();
  void on_checkBoxResizeNumber_toggled(bool checked);
};
