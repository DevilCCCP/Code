#pragma once

#include <QMainWindow>
#include <QElapsedTimer>
#include <QFutureWatcher>

#include <Lib/Include/Common.h>

#include "Calc.h"


DefineClassS(Db);
class QProcess;
class QTimer;

namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow
{
  Ui::MainWindow*       ui;
  DbS                   mDb;

  Calc                  mCalc;
  qreal                 mCirclePerformance;
  QElapsedTimer         mWorkTimer;
  int                   mWorkers;

  QList<QProcess*>      mWorkProcesses;
  QTimer*               mWatchDbTimer;
  qint64                mJobId;

  Q_OBJECT

public:
  explicit MainWindow(const DbS& _Db, QWidget* parent = 0);
  ~MainWindow();

private:
  void CalcCircle();
  void CalcTotal();

private:
  void OnFinished();
  void OnTimeout();

private slots:
  void on_spinBoxCalc_editingFinished();
  void on_doubleSpinBoxTime_editingFinished();
  void on_pushButtonTestThread_clicked();
  void on_pushButtonTestProcess_clicked();
  void on_pushButtonTestSingle_clicked();
};
