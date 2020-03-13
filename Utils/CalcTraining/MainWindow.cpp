#include <QMessageBox>
#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>
#include <QShowEvent>
#include <QCloseEvent>
#include <QDir>
#include <QTimer>
#include <QDateTime>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "DialogLogin.h"
#include "MulInfoModel.h"


const int kPlaySeries = 10;
const int kSolveTime = 15 * 1000;
const int kShowTime = 2 * 1000;

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
  , mPlaying(false), mStopping(false)
  , mResultModel(nullptr)
  , mPlayTimer(new QTimer(this))
{
  ui->setupUi(this);

  mPlayTimer->setSingleShot(false);
  mPlayTimer->setInterval(20);
  setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), qApp->desktop()->availableGeometry()));

  connect(this, &MainWindow::Login, this, &MainWindow::OnLogin, Qt::QueuedConnection);
  connect(mPlayTimer, &QTimer::timeout, this, &MainWindow::OnTimeout);
}

MainWindow::~MainWindow()
{
  delete ui;
}


void MainWindow::showEvent(QShowEvent* event)
{
  emit Login();

  QMainWindow::showEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  if (mPlaying) {
    Stop();
    event->ignore();
    return;
  }

  QMainWindow::closeEvent(event);
}

bool MainWindow::Init()
{
  QDir appDir(QCoreApplication::instance()->applicationDirPath());
  appDir.cd("Var");
  QString filename = appDir.absoluteFilePath(QString("%1.ini").arg(mUser));
  mUserInfo = new QSettings(filename, QSettings::IniFormat, this);
  mUserInfo->beginReadArray("Number");

  mResult.resize(9*9);
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      int index = i * j;
      mUserInfo->setArrayIndex(index);
      LoadResult(&mResult[index]);
    }
  }
  mUserInfo->endArray();
  if (!QFile::exists(filename)) {
    Save();
  }
  ReloadModel();
  return true;
}

void MainWindow::LoadResult(MulInfo* mulInfo)
{
  mulInfo->Try  = mUserInfo->value("Try", 0).toInt();
  mulInfo->Ok   = mUserInfo->value("Ok", 0).toInt();
  mulInfo->Fail = mUserInfo->value("Fail", 0).toInt();
  mulInfo->Last = mUserInfo->value("Last", 0).toInt();
  mulInfo->Pass = mUserInfo->value("Pass", false).toBool();
}

void MainWindow::SaveResult(const MulInfo* mulInfo)
{
  mUserInfo->setValue("Try", mulInfo->Try);
  mUserInfo->setValue("Ok", mulInfo->Ok);
  mUserInfo->setValue("Fail", mulInfo->Fail);
  mUserInfo->setValue("Last", mulInfo->Last);
  mUserInfo->setValue("Pass", mulInfo->Pass);
}

void MainWindow::SetResultOk(MulInfo* mulInfo)
{
  mulInfo->Try++;
  mulInfo->Ok++;
  mulInfo->Last++;
  mulInfo->Pass = mulInfo->Last > 5 + qMin(2 * mulInfo->Fail, 5);
}

void MainWindow::SetResultFail(MulInfo* mulInfo)
{
  mulInfo->Try++;
  mulInfo->Fail++;
  mulInfo->Last = 0;
  mulInfo->Pass = false;
}

bool MainWindow::Save()
{
  QDir appDir(QCoreApplication::instance()->applicationDirPath());
  appDir.cd("Var");
  mUserInfo = new QSettings(appDir.absoluteFilePath(QString("%1.ini").arg(mUser)), QSettings::IniFormat, this);
  mUserInfo->beginReadArray("Number");

  mResult.resize(9*9);
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      int index = i * j;
      mUserInfo->setArrayIndex(index);
      SaveResult(&mResult[index]);
    }
  }
  mUserInfo->endArray();
  mUserInfo->sync();
  return true;
}

void MainWindow::Play()
{
  qsrand(QDateTime::currentMSecsSinceEpoch());
  mPlaySeries = 0;
  QVector<int> possibleValues;
  for (int i = 0; i < 9 * 9; i++) {
    const MulInfo& info = mResult.at(i);
    if (!info.Pass) {
      possibleValues.append(i);
    }
  }

  mPlaySeries = qMin(possibleValues.size(), kPlaySeries);

  mPlayPlan.clear();
  for (int i = 0; i < mPlaySeries; i++) {
    int index = qrand() % possibleValues.size();
    mPlayPlan.append(possibleValues.at(index));
    possibleValues[index] = possibleValues[possibleValues.size() - 1];
    possibleValues.resize(possibleValues.size() - 1);
  }
  if (mPlaySeries == 0) {
    QMessageBox::information(this, "Внимание", "Ваши результаты зачтены, больше нечего решать", QMessageBox::Ok);
    return;
  }

  mPlaying = true;
  ui->stackedWidgetMain->setCurrentIndex(1);
  PlayNext();

  UpdateActions();
}

void MainWindow::Stop()
{
  if (mPlaying) {
    mStopping = true;
  }

  UpdateActions();
}

void MainWindow::PlayNext()
{
  --mPlaySeries;
  if (mStopping || mPlaySeries < 0) {
    PlayHalt();
    return;
  }
  ui->widgetTime->setVisible(true);
  ui->widgetRight->setVisible(false);
  ui->widgetFail->setVisible(false);
  mPlayIndex = mPlayPlan[mPlaySeries];
  ui->labelFirst->setText(QString::number(1 + mPlayIndex / 9));
  ui->labelLast->setText(QString::number(1 + mPlayIndex % 9));
  ui->lineEditSolve->setText("");
  mPlayElapsed.start();
  mPlayTimer->start();
}

void MainWindow::PlayDone()
{
  if (mPlayIndex < 0) {
    return;
  }

  QString resultText = ui->lineEditSolve->text();
  int result = resultText.toInt();
  if (result == (1 + mPlayIndex / 9) * (1 + mPlayIndex % 9)) {
    SetResultOk(&mResult[mPlayIndex]);
    ui->widgetRight->setVisible(true);
  } else {
    SetResultFail(&mResult[mPlayIndex]);
    ui->widgetFail->setVisible(true);
  }
  mPlayIndex = -1;
  ui->widgetTime->setVisible(false);
  mPlayElapsed.start();
  mPlayTimer->start();
}

void MainWindow::PlayHalt()
{
  mPlaying  = false;
  mStopping = false;
  mPlayTimer->stop();

  UpdateActions();
  ReloadModel();
  ui->stackedWidgetMain->setCurrentIndex(0);
  if (!Save()) {
    QMessageBox::warning(this, "Ошибка", "Невозможно сохранить данные, изменения не применятся", QMessageBox::Ok);
  }
}

void MainWindow::UpdateActions()
{
  ui->actionStart->setEnabled(!mPlaying);
  ui->actionStop->setEnabled(mPlaying && !mStopping);
  ui->actionExit->setEnabled(!mPlaying);
}

void MainWindow::ReloadModel()
{
  if (mResultModel) {
    mResultModel->deleteLater();
  }
  mResultModel = new MulInfoModel(mResult, this);
  ui->tableViewResult->setModel(mResultModel);
}

void MainWindow::OnLogin()
{
  DialogLogin* dialogLogin = new DialogLogin(this);
  int res = dialogLogin->exec();
  if (res == QDialog::Accepted && !dialogLogin->Name().isEmpty()) {
    mUser = dialogLogin->Name();
    if (!Init()) {
      QMessageBox::warning(this, "Ошибка", "Невозможно загрузить данные, программа будет закрыта", QMessageBox::Ok);
      close();
    }
  } else {
    close();
  }
}

void MainWindow::OnTimeout()
{
  if (mPlayIndex < 0) {
    if (mPlayElapsed.elapsed() > kShowTime) {
      PlayNext();
    }
    return;
  }
  int timeout = kSolveTime - mPlayElapsed.elapsed();
  if (timeout > 0) {
    ui->labelTimer->setText(QString("%1").arg(timeout * 0.001, 0, 'f', 1));
  } else {
    PlayDone();
  }
}

void MainWindow::on_actionExit_triggered()
{
  close();
}

void MainWindow::on_actionStart_triggered()
{
  Play();
}

void MainWindow::on_actionStop_triggered()
{
  Stop();
}

void MainWindow::on_lineEditSolve_returnPressed()
{
  PlayDone();
}

void MainWindow::on_pushButtonPlayOk_clicked()
{
  PlayDone();
}
