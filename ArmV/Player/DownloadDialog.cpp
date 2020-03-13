#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

#include "DownloadDialog.h"
#include "ui_DownloadDialog.h"


void DownloadDialog::SetCameraId(int id)
{
  int index = 0;
  for (auto itr = mCameras.begin(); itr != mCameras.end(); itr++) {
    if (itr->first == id) {
      ui->comboBoxCameras->setCurrentIndex(index);
      return;
    }
    index++;
  }
}

int DownloadDialog::GetCameraId()
{
  return ui->comboBoxCameras->itemData(ui->comboBoxCameras->currentIndex()).toInt();
}

void DownloadDialog::SetSpeed(int speed)
{
  ui->spinBoxSpeed->setValue(speed);
}

int DownloadDialog::GetSpeed()
{
  return mSpeed;
}

void DownloadDialog::SetTimeStart(const QDateTime &timeStart)
{
  ui->dateTimeEditStart->setDateTime(timeStart);
}

QDateTime DownloadDialog::GetTimeStart()
{
  return mStartTime;
}

void DownloadDialog::SetTimeEnd(const QDateTime &timeEnd)
{
  ui->dateTimeEditFinish->setDateTime(timeEnd);
}

QDateTime DownloadDialog::GetTimeEnd()
{
  return mEndTime;
}

void DownloadDialog::SetPath(const QString &path)
{
  ui->lineEditPath->setText(path);
}

QString DownloadDialog::GetPath()
{
  return mPath;
}

bool DownloadDialog::GetPathAsDefault()
{
  return mPathAsDefault;
}

void DownloadDialog::accept()
{
  mSpeed = ui->spinBoxSpeed->value();
  mStartTime = ui->dateTimeEditStart->dateTime();
  mEndTime = ui->dateTimeEditFinish->dateTime();
  mPath = ui->lineEditPath->text();
  mPathAsDefault = ui->checkBoxDefault->checkState() == Qt::Checked;
  if (mStartTime > mEndTime) {
    QMessageBox::information(this, windowTitle()
                             , QString::fromUtf8("Временной промежуток должен быть положительным"));
    ui->dateTimeEditStart->setFocus();
  } else if (!QDir(mPath).exists()) {
    QMessageBox::information(this, windowTitle()
                             , QString::fromUtf8("Необходимо указать корректный пусть к папке для сохранения записи"));
    ui->lineEditPath->setFocus();
  } else {
    QDialog::accept();
  }
}

void DownloadDialog::on_pushButtonBrowse_clicked()
{
  QString dir = QFileDialog::getExistingDirectory(this
                                                  , QString::fromUtf8("Выбор пути для сохранения записи")
                                                  , ui->lineEditPath->text());
  if (!dir.isEmpty()) {
    ui->lineEditPath->setText(dir);
  }
}


DownloadDialog::DownloadDialog(QList<QPair<int, QString> > &_Cameras, QWidget *parent)
  : QDialog(parent), ui(new Ui::DownloadDialog)
  , mCameras(_Cameras)
{
  ui->setupUi(this);
  for (auto itr = mCameras.begin(); itr != mCameras.end(); itr++) {
    ui->comboBoxCameras->addItem(itr->second, itr->first);
  }
  ui->comboBoxCameras->setCurrentIndex(-1);
  setWindowFlags(Qt::WindowStaysOnTopHint);
}

DownloadDialog::~DownloadDialog()
{
  delete ui;
}

