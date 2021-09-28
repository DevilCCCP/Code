#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QStandardPaths>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QShowEvent>

#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent, Qt::WindowStaysOnTopHint), ui(new Ui::MainWindow)
  , mBuildProcess(nullptr), mAppProcess(nullptr), mCurrentId(-1), mInit(false)
{
  ui->setupUi(this);

  QString settinsFilename = QApplication::applicationDirPath() + "/.settings";
  ui->statusBar->showMessage("Settings file: " + settinsFilename);
  mSettings = new QSettings(settinsFilename, QSettings::IniFormat, this);
  ui->lineEditProj->setText(mSettings->value("Proj", "Vica").toString());
  ui->lineEditSourcePath->setText(mSettings->value("Source", "C:\\!Code\\!Code").toString());
  ui->lineEditParams->setText(mSettings->value("Params", "--id=%1|--uri=tcp:::20100|-d|-t").toString());
  ui->lineEditMsvsPath->setText(mSettings->value("Msvs", "C:\\Programs\\VS 10").toString());
  restoreGeometry(mSettings->value("Window", size()).toByteArray());

  mListModel = new QStringListModel(this);
  mListData = mSettings->value("List", QStringList()).toStringList();
  mListModel->setStringList(QStringList() << mListData << "<new>");
  ui->listViewMain->setModel(mListModel);
  connect(mListModel, &QStringListModel::dataChanged, this, &MainWindow::OnListDataChanged);

  auto sm = ui->listViewMain->selectionModel();
  connect(sm, &QItemSelectionModel::currentChanged, this, &MainWindow::OnListCurrentChanged);
  SetRunEnable(false);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
  SaveGeometry();

  QMainWindow::resizeEvent(event);
}

void MainWindow::moveEvent(QMoveEvent* event)
{
  SaveGeometry();

  QMainWindow::moveEvent(event);
}

void MainWindow::showEvent(QShowEvent* event)
{
  QMainWindow::showEvent(event);

  mInit = true;
}

void MainWindow::SaveGeometry()
{
  if (mInit) {
    mSettings->setValue("Window", saveGeometry());
    mSettings->sync();
    ui->statusBar->showMessage("Save window geometry", 3000);
  }
}

void MainWindow::SetRunEnable(bool enable)
{
  if (mCurrentId < 0 || mCurrentId >= mListData.size()) {
    enable = false;
  }

  ui->actionRun->setEnabled(enable);
  ui->actionStop->setEnabled(mAppProcess);
}

void MainWindow::OnBuildFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  ui->statusBar->showMessage("Build finished", 10000);

  ui->progressBarBuild->setMaximum(1);
  if (exitStatus == QProcess::NormalExit) {
    ui->lineEditBuildStatus->setText(QString("Exit: %1").arg(exitCode));
    if (exitCode == 0) {
      ui->tabWidget->setCurrentIndex(0);
    }
  } else {
    ui->lineEditBuildStatus->setText("Failed");
  }
  SetRunEnable(true);
}

void MainWindow::OnAppFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitStatus == QProcess::NormalExit) {
    ui->statusBar->showMessage(QString("Application exit: %1").arg(exitCode), 10000);
  } else {
    ui->statusBar->showMessage("Application crushed", 10000);
  }
  mAppProcess->deleteLater();
  mAppProcess = nullptr;
  SetRunEnable(true);
}

void MainWindow::OnListDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
  Q_UNUSED(topLeft);
  Q_UNUSED(bottomRight);
  Q_UNUSED(roles);

  mListData = mListModel->stringList();
  if (!mListData.isEmpty() && mListData.last() == "<new>") {
    mListData.removeLast();
  }
  if (topLeft.isValid()) {
    int row = topLeft.row();
    if (row >= 0 && row < mListData.size() && mListData.at(row).isEmpty()) {
      mListData.removeAt(row);
    }
  }

  mSettings->setValue("List", mListData);
  mSettings->sync();

  mListModel->setStringList(QStringList() << mListData << "<new>");
}

void MainWindow::OnListCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
  Q_UNUSED(previous);

  if (current.isValid()) {
    mCurrentId = current.row();
  }

  SetRunEnable(true);
}

void MainWindow::on_lineEditProj_textChanged(const QString &text)
{
  mSettings->setValue("Proj", text);
  mSettings->sync();
}

void MainWindow::on_lineEditSourcePath_textChanged(const QString &text)
{
  mSettings->setValue("Source", text);
  mSettings->sync();
}

void MainWindow::on_lineEditParams_textChanged(const QString &text)
{
  mSettings->setValue("Params", text);
  mSettings->sync();
}

void MainWindow::on_lineEditMsvsPath_textChanged(const QString &text)
{
  mSettings->setValue("Msvs", text);
  mSettings->sync();
}

void MainWindow::on_actionBuild_triggered()
{
  QString msvcPath = ui->lineEditMsvsPath->text() + "\\Common7\\IDE\\devenv.exe";
  if (!QFile::exists(msvcPath)) {
    ui->tabWidget->setCurrentIndex(2);
    ui->lineEditMsvsPath->setFocus();
    QMessageBox::warning(this, "Build", "Visual studio path is incorrect");
    return;
  }

  if (mBuildProcess) {
    mBuildProcess->deleteLater();
  }

  mBuildProcess = new QProcess(this);
  mBuildProcess->setProgram(msvcPath);
  mBuildProcess->setArguments(QStringList()
                              << QString("%1\\%2\\%2.sln").arg(ui->lineEditSourcePath->text()).arg(ui->lineEditProj->text())
                              << "/build" << "Debug|Win32");
  ui->progressBarBuild->setMaximum(0);
  connect(mBuildProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &MainWindow::OnBuildFinished);
  ui->statusBar->showMessage("Build started");

  ui->tabWidget->setCurrentIndex(1);
  SetRunEnable(false);
  mBuildProcess->start();
}

void MainWindow::on_actionRun_triggered()
{
  QString appPath = QString("%1\\%2\\bin\\debug\\%3_va.exe").arg(ui->lineEditSourcePath->text())
      .arg(ui->lineEditProj->text()).arg(ui->lineEditProj->text().toLower());
  if (!QFile::exists(appPath)) {
    ui->tabWidget->setCurrentIndex(2);
    ui->lineEditSourcePath->setFocus();
    QMessageBox::warning(this, "Run", QString("Application not found at '%1'").arg(appPath));
    return;
  }

  if (mCurrentId < 0 || mCurrentId >= mListData.size()) {
    QMessageBox::warning(this, "Run", QString("Application id not selected (index: %1)").arg(mCurrentId));
    return;
  }

  if (mAppProcess) {
    mAppProcess->terminate();
    mAppProcess->deleteLater();
  }

  mAppProcess = new QProcess(this);
  mAppProcess->setProgram(appPath);
  mAppProcess->setArguments(QString(ui->lineEditParams->text()).arg(mListData.at(mCurrentId)).split('|'));
  ui->statusBar->showMessage("Run started");
  connect(mAppProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &MainWindow::OnAppFinished);

  ui->tabWidget->setCurrentIndex(0);
  mCurrentId++;
  auto sm = ui->listViewMain->selectionModel();
  sm->setCurrentIndex(sm->currentIndex().sibling(mCurrentId, 0), QItemSelectionModel::SelectCurrent);
  SetRunEnable(true);
  mAppProcess->start();
}

void MainWindow::on_actionStop_triggered()
{
  mAppProcess->terminate();
  mAppProcess->deleteLater();
  mAppProcess = nullptr;
  SetRunEnable(true);
}

void MainWindow::on_actionExit_triggered()
{
  close();
}
