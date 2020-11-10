#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>

#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  QString iniFilePath = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).absoluteFilePath("main_wnd.ini");
  mSettings = new QSettings(iniFilePath, QSettings::IniFormat);
  mRegExpRecentList = mSettings->value("RegExpRecentList").toStringList();
  ui->comboBoxRegExp->addItems(mRegExpRecentList);

  ui->toolButtonBrowse->setDefaultAction(ui->actionBrowseFolder);

  ui->lineEditDirectory->setText(QDir::currentPath());
  ui->checkBoxRename->setChecked(true);
  ui->checkBoxNumbers->setChecked(false);
  on_checkBoxNumbers_toggled(false);
  ui->checkBoxSecondNumber->setChecked(false);
  on_checkBoxSecondNumber_toggled(false);
  ui->checkBoxResizeNumber->setChecked(true);
  ui->widgetProgress->setVisible(false);
  ui->checkBoxExit->setChecked(true);
}

MainWindow::~MainWindow()
{
  delete ui;
}


void MainWindow::OnWorkFinished()
{
  if (mRenameWorker) {
    mRenameWorker->deleteLater();
    mRenameWorker = nullptr;
  }

  ui->pushButtonStart->setEnabled(true);
  ui->widgetProgress->setVisible(false);

  if (ui->checkBoxExit->isChecked()) {
    close();
  }
}

void MainWindow::OnWorkProgress(int percent)
{
  ui->progressBarWork->setValue(percent);
}

void MainWindow::on_actionBrowseFolder_triggered()
{
  QString newPath = QFileDialog::getExistingDirectory(this, "Select starting directory", ui->lineEditDirectory->text());
  if (!newPath.isEmpty()) {
    ui->lineEditDirectory->setText(newPath);
  }
}

void MainWindow::on_checkBoxRename_toggled(bool checked)
{
  ui->widgetRename->setEnabled(checked);
}

void MainWindow::on_checkBoxNumbers_toggled(bool checked)
{
  ui->widgetNumbers->setEnabled(checked);
}

void MainWindow::on_checkBoxSecondNumber_toggled(bool checked)
{
  ui->widgetSecondNumber->setEnabled(checked);
}

void MainWindow::on_pushButtonStart_clicked()
{
  QString regExpText = ui->comboBoxRegExp->currentText();
  QString regExpReplace = ui->lineEditValue->text();
  QString regExpItem = QString("\"%1\"->\"%2\"").arg(regExpText, regExpReplace);
  if (ui->checkBoxRename->isChecked() && !mRegExpRecentList.contains(regExpItem)) {
    mRegExpRecentList.append(regExpItem);
    ui->comboBoxRegExp->addItem(regExpItem);
    mSettings->setValue("RegExpRecentList", mRegExpRecentList);
    mSettings->sync();
  }

  ui->pushButtonStart->setEnabled(false);
  ui->widgetProgress->setVisible(true);

  if (mRenameWorker) {
    return;
  }

  mRenameWorker = new RenameWorker(this);
  mRenameWorker->setPath(ui->lineEditDirectory->text());

  mRenameWorker->setRename(ui->checkBoxRename->isChecked());
  mRenameWorker->setRenameRegExp(ui->comboBoxRegExp->currentText());
  mRenameWorker->setRenameValue(ui->lineEditValue->text());

  mRenameWorker->setNumbers(ui->checkBoxNumbers->isChecked());
  mRenameWorker->setFirstNumber(ui->spinBoxFirstNumber->value());
  mRenameWorker->setSecondNumber(ui->checkBoxSecondNumber->isChecked()? ui->spinBoxSecondNumber->value(): 0);
  mRenameWorker->setResizeNumbers(ui->checkBoxResizeNumber->isChecked()? ui->spinBoxNumberResize->value(): 0);
  mRenameWorker->setSwapNumbers(ui->checkBoxSwap->isChecked());

  connect(mRenameWorker, &RenameWorker::finished, this, &MainWindow::OnWorkFinished);
  connect(mRenameWorker, &RenameWorker::Progress, this, &MainWindow::OnWorkProgress);

  mRenameWorker->Start();
}

void MainWindow::on_checkBoxResizeNumber_toggled(bool checked)
{
  ui->spinBoxNumberResize->setEnabled(checked);
}

void MainWindow::on_comboBoxRegExp_currentIndexChanged(const QString& text)
{
  QRegExp regExp("\"(.*)\"->\"(.*)\"");
  if (regExp.exactMatch(text)) {
    ui->comboBoxRegExp->setCurrentText(regExp.cap(1));
    ui->lineEditValue->setText(regExp.cap(2));
  }
}
