#include <QtConcurrent>
#include <QTimer>

#include <Lib/Db/Db.h>
#include <Lib/Settings/FileSettings.h>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "MultiThreadCalc.h"


MainWindow::MainWindow(const DbS& _Db, QWidget* parent)
  : QMainWindow(parent), ui(new Ui::MainWindow), mDb(_Db)
  , mWorkers(0)
  , mWatchDbTimer(new QTimer(this))
{
  ui->setupUi(this);

  ui->progressBarProcess->setVisible(false);

  CalcCircle();
  CalcTotal();

  connect(mWatchDbTimer, &QTimer::timeout, this, &MainWindow::OnTimeout);
}

MainWindow::~MainWindow()
{
  while (mWorkProcesses.size() > 0) {
    QProcess* proc = mWorkProcesses.takeLast();
    proc->kill();
  }
  delete ui;
}


void MainWindow::CalcCircle()
{
  qint64 ms = (qint64)(ui->doubleSpinBoxTime->value() * 1000.0);
  ms = qMax(ms, (qint64)1);
  mCalc.CalcTime(0, ms);
  qint64 circles = mCalc.WorkCircles();
  ui->spinBoxCircles->setMaximum(circles);
  ui->spinBoxCircles->setValue(circles);
  mCirclePerformance = (qreal)mCalc.WorkMs() / mCalc.WorkCircles();
}

void MainWindow::CalcTotal()
{
  qreal time = ui->spinBoxCalc->value() * (ui->spinBoxCircles->value() * mCirclePerformance * 0.001);
  ui->doubleSpinBoxResultTime->setMaximum(time);
  ui->doubleSpinBoxResultTime->setValue(time);
}

void MainWindow::OnFinished()
{
  QFutureWatcher<void>* watcher = dynamic_cast<QFutureWatcher<void>*>(sender());
  if (watcher) {
    mWorkers--;
    watcher->deleteLater();
  }

  if (mWorkers == 0) {
    ui->doubleSpinBoxResultThread->setMaximum(mWorkTimer.elapsed() * 0.001);
    ui->doubleSpinBoxResultThread->setValue(mWorkTimer.elapsed() * 0.001);
    ui->groupBoxMultiThread->setEnabled(true);
  }
}

void MainWindow::OnTimeout()
{
  auto q = mDb->MakeQuery();
  q->prepare(QString("SELECT done + fail, iter_end FROM job WHERE _id = %1;").arg(mJobId));
  if (!mDb->ExecuteQuery(q) || !q->next()) {
    return;
  }

  int done = q->value(0).toInt();
  int end  = q->value(1).toInt();
  if (done >= end) {
    ui->doubleSpinBoxResultProcess->setMaximum(mWorkTimer.elapsed() * 0.001);
    ui->doubleSpinBoxResultProcess->setValue(mWorkTimer.elapsed() * 0.001);
    mWatchDbTimer->stop();
    ui->progressBarProcess->setVisible(false);
  } else {
    ui->progressBarProcess->setValue(100 * done / end);
  }
}

void MainWindow::on_spinBoxCalc_editingFinished()
{
  CalcTotal();
}

void MainWindow::on_doubleSpinBoxTime_editingFinished()
{
  CalcCircle();
  CalcTotal();
}

void MainWindow::on_pushButtonTestThread_clicked()
{
  qint64 circles = ui->spinBoxCircles->value();
  int totalThreads = ui->spinBoxThreads->value();
  int totalCalcs = ui->spinBoxCalc->value();
  int lastCalc = 0;
  mWorkTimer.start();
  for (mWorkers = 0; mWorkers < totalThreads; mWorkers++) {
    MultiThreadCalc* mcalc = new MultiThreadCalc(this);
    int nextCalc = totalCalcs * (mWorkers + 1) / totalThreads;
    QFutureWatcher<void>* watcher(new QFutureWatcher<void>(this));
    connect(watcher, &QFutureWatcher<void>::finished, this, &MainWindow::OnFinished);
    QFuture<void> future = QtConcurrent::run(mcalc, &MultiThreadCalc::Calc, circles, lastCalc, nextCalc);
    watcher->setFuture(future);
    lastCalc = nextCalc;
  }
  ui->groupBoxMultiThread->setEnabled(false);
}

void MainWindow::on_pushButtonTestProcess_clicked()
{
  if (!mDb) {
    mDb.reset(new Db());
    mDb->OpenDefault();
  }
  if (!mDb->Connect()) {
    return;
  }
  {
    auto q = mDb->MakeQuery();
    q->prepare(QString("DELETE FROM job;"));
    mDb->ExecuteNonQuery(q);
  }

  qint64 circles = ui->spinBoxCircles->value();
  int totalProcesses = ui->spinBoxProcesses->value();
  int totalCalcs = ui->spinBoxCalc->value();

  while (mWorkProcesses.size() < totalProcesses) {
    QProcess* proc = new QProcess(this);
    proc->setProgram(qApp->applicationFilePath());
    proc->setArguments(QStringList() << "worker" << QString::number(mWorkProcesses.size() + 1));
    proc->start();
    mWorkProcesses.append(proc);
  }
  while (mWorkProcesses.size() > totalProcesses) {
    QProcess* proc = mWorkProcesses.takeLast();
    proc->kill();
  }
  for (int i = 0; i < mWorkProcesses.size(); i++) {
    QProcess* proc = mWorkProcesses[i];
    proc->waitForStarted();
  }

  ui->progressBarProcess->setVisible(true);
  mWatchDbTimer->start(10);
  mWorkTimer.start();
  {
    QString params = QString("%1").arg(circles);
    auto q = mDb->MakeQuery();
    q->prepare(QString("INSERT INTO job(name, is_active, iter_end, data) VALUES ('test_job', true, ?, ?) RETURNING _id;"));
    q->addBindValue(totalCalcs);
    q->addBindValue(params.toLatin1());
    if (!mDb->ExecuteQuery(q) || !q->next()) {
      return;
    }
    mJobId = q->value(0).toLongLong();
  }
}

void MainWindow::on_pushButtonTestSingle_clicked()
{
  mWorkTimer.start();
  int count = ui->spinBoxCalc->value();
  qint64 circles = ui->spinBoxCircles->value();
  for (int i = 0; i < count; i++) {
    mCalc.CalcCircles(i, circles);
  }
  ui->doubleSpinBoxResultSingle->setMaximum(mWorkTimer.elapsed() * 0.001);
  ui->doubleSpinBoxResultSingle->setValue(mWorkTimer.elapsed() * 0.001);
}
