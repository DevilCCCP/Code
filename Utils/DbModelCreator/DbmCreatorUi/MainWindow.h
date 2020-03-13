#pragma once

#include <QMainWindow>

#include "MainWindow2.h"
#include "../Core/Core.h"


namespace Ui {
class MainWindow;
}

class MainWindow: public MainWindow2
{
  Ui::MainWindow* ui;

  Core            mCore;
  QString         mSourceDirectory;
  QString         mSourceModelDirectory;
  QString         mSqlDirectory;

  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

private slots:
  void on_actionGenerate_triggered();
  void on_toolButtonSourcePaste_clicked();
  void on_toolButtonHeaderCopy_clicked();
  void on_toolButtonSourceCopy_clicked();
  void on_toolButtonModelHeaderCopy_clicked();
  void on_toolButtonModelSourceCopy_clicked();
  void on_actionSave_triggered();
  void on_actionSaveModel_triggered();
  void on_actionPath_triggered();
  void on_actionPathModel_triggered();
  void on_actionBatch_triggered();
  void on_actionBatchModel_triggered();
};

