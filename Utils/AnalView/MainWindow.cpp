#include <QDesktopWidget>
#include <QFileDialog>
#include <QDirIterator>
#include <QTimer>
#include <QFile>
#include <QSet>
#include <QtMath>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "SelectDlg.h"


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
  , mSettings(qApp->applicationDirPath() + "/.settings.ini", QSettings::IniFormat)
  , mScaled(true), mIndex(0), mInit(false)
  , mSaveStateTimer(new QTimer(this))
{
  ui->setupUi(this);

  if (!this->restoreGeometry(QByteArray::fromBase64(mSettings.value("MainWindowSz", QByteArray()).toByteArray()))) {
    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
  }
  this->restoreState(QByteArray::fromBase64(mSettings.value("MainWindow", QByteArray()).toByteArray()));
  ui->mainToolBar->insertWidget(ui->actionBrowse, ui->lineEditPath);
  //ui->mainToolBar->insertSeparator(ui->actionSelect);
  ui->lineEditPath->setText(mSettings.value("Path").toString());

  ui->actionScaled->setChecked(mScaled = true);

  mSaveStateTimer->setSingleShot(true);
  connect(mSaveStateTimer, &QTimer::timeout, this, &MainWindow::SaveWindowState);
  mInit = true;
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
  QMainWindow::resizeEvent(event);

  ScheduleStateSave();
}

void MainWindow::moveEvent(QMoveEvent* event)
{
  QMainWindow::moveEvent(event);

  ScheduleStateSave();
}

void MainWindow::ScheduleStateSave()
{
  if (mInit) {
    mSaveStateTimer->start(500);
  }
}

void MainWindow::SaveWindowState()
{
  mSettings.setValue("MainWindow", this->saveState().toBase64());
  mSettings.setValue("MainWindowSz", this->saveGeometry().toBase64());
  mSettings.sync();
}

void MainWindow::SelectPath()
{
  mPath = ui->lineEditPath->text();

  QSet<QString> names;
  for (QDirIterator itr(mPath); itr.hasNext(); ) {
    itr.next();
    QString filename = itr.fileName();
    if (filename.endsWith(".jpg")) {
      int sep = filename.indexOf(QChar('_'));
      if (sep > 0) {
        QString name = filename.mid(0, sep);
        names.insert(name);
      }
    }
  }

  QList<QString> namesList = names.toList();
  qSort(namesList);

  SelectDlg* dlg = new SelectDlg(this);
  dlg->SetLeft(namesList);
  int ret = dlg->exec();
  if (ret) {
    dlg->GetRight(mItems);
    ReloadView();
    DrawImage();
  }
  dlg->deleteLater();
}

void MainWindow::ReloadView()
{
  mItemsMap.clear();
  while (auto rmItem = ui->gridLayoutImages->takeAt(0)) {
    rmItem->widget()->deleteLater();
  }

  int rowCount = qMax((int)(qSqrt(0.5 * mItems.count()) + 0.99), 1);
  int columnCount = (mItems.count() + rowCount - 1) / rowCount;

  auto itr = mItems.begin();
  for (int j = 0; j < rowCount; j++) {
    for (int i = 0; i < columnCount; i++) {
      if (itr == mItems.end()) {
        return;
      }
      const QString& name = *itr;
      QLabel* label = new QLabel(name, this);
      mItemsMap[name] = label;
      ui->gridLayoutImages->addWidget(label, j, i, 1, 1);
      itr++;
    }
  }
}

void MainWindow::DrawImage()
{
  for (auto itr = mItems.begin(); itr != mItems.end(); itr++) {
    const QString& name = *itr;
    QString filename = QString("%1/%2_%3.jpg").arg(mPath).arg(name).arg(mIndex, 6, 10, QChar('0'));
    QLabel* label = mItemsMap[name];
    if (QFile::exists(filename)) {
      QPixmap pixmap(filename);
      label->setPixmap(pixmap);
      label->setScaledContents(mScaled);
    } else {
      label->setText(name);
    }
  }
}

void MainWindow::on_spinBox_valueChanged(int value)
{
  ui->horizontalSlider->setMinimum(value);
}


void MainWindow::on_spinBox_2_valueChanged(int value)
{
  ui->horizontalSlider->setMaximum(value);
}

void MainWindow::on_spinBoxIndex_valueChanged(int value)
{
  mIndex = value;
  DrawImage();
}

void MainWindow::on_actionBrowse_triggered()
{
  QString path = QFileDialog::getExistingDirectory(this, "Select path", ui->lineEditPath->text()
                                                   , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!path.isEmpty()) {
    ui->lineEditPath->setText(path);
  }
}

void MainWindow::on_actionSelect_triggered()
{
  mSettings.setValue("Path", ui->lineEditPath->text());
  mSettings.sync();

  SelectPath();
}

void MainWindow::on_actionClear_triggered()
{
  mPath = ui->lineEditPath->text();

  QDir dir(mPath);
  QStringList list = dir.entryList(QStringList() << "*.jpg", QDir::NoDot | QDir::NoDotAndDotDot | QDir::Files);
  for (auto itr = list.begin(); itr != list.end(); itr++) {
    const QString& filename = *itr;
    dir.remove(filename);
  }
}

void MainWindow::on_actionScaled_triggered(bool checked)
{
  mScaled = checked;
  DrawImage();
}
