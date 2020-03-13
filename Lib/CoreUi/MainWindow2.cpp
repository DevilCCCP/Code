#include <QtGlobal>
#include <QStandardPaths>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QSettings>

#include "MainWindow2.h"


MainWindow2::MainWindow2(QWidget* parent)
  : QMainWindow(parent)
  , mRestored(false)
{
  Q_INIT_RESOURCE(Ui);

  QString iniFilePath = QDir(QStandardPaths::writableLocation(
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
                               QStandardPaths::AppDataLocation
#else
                               QStandardPaths::DataLocation
#endif
                               )).absoluteFilePath("main_wnd.ini");

  mSettings = new QSettings(iniFilePath, QSettings::IniFormat, this);
  mSettings->setIniCodec("UTF-8");

  mSaveStateTimer = new QTimer(this);
  mSaveStateTimer->setSingleShot(true);
  connect(mSaveStateTimer, &QTimer::timeout, this, &MainWindow2::SaveWindowState);
}

bool MainWindow2::Restore()
{
  bool ok = this->restoreGeometry(QByteArray::fromBase64(mSettings->value("MainWindowGeometry", QByteArray()).toByteArray()));
  this->restoreState(QByteArray::fromBase64(mSettings->value("MainWindowState", QByteArray()).toByteArray()));

  mRestored = true;
  return ok;
}


bool MainWindow2::RegisterSaveWidget(QWidget* widget)
{
  mSaveWidgets.append(widget);
  return widget->restoreGeometry(QByteArray::fromBase64(mSettings->value(widget->objectName(), QByteArray()).toByteArray()));
}

bool MainWindow2::RegisterSaveSplitter(QSplitter* splitter)
{
  connect(splitter, &QSplitter::splitterMoved, this, &MainWindow2::ScheduleSplitterSave);
  mSaveSplitters.append(splitter);
  return splitter->restoreState(QByteArray::fromBase64(mSettings->value(splitter->objectName(), QByteArray()).toByteArray()));
}

void MainWindow2::ScheduleSplitterSave(int pos, int ind)
{
  Q_UNUSED(pos);
  Q_UNUSED(ind);

  ScheduleStateSave();
}

void MainWindow2::ScheduleStateSave()
{
  if (mRestored) {
    mSaveStateTimer->start(500);
  }
}

void MainWindow2::SaveWindowState()
{
  mSettings->setValue("MainWindowState", this->saveState().toBase64());
  mSettings->setValue("MainWindowGeometry", this->saveGeometry().toBase64());
  for (auto itr = mSaveSplitters.begin(); itr != mSaveSplitters.end(); itr++) {
    const QSplitter* splitter = *itr;
    mSettings->setValue(splitter->objectName(), splitter->saveState().toBase64());
  }
  for (auto itr = mSaveWidgets.begin(); itr != mSaveWidgets.end(); itr++) {
    const QWidget* widget = *itr;
    mSettings->setValue(widget->objectName(), widget->saveGeometry().toBase64());
  }
  mSettings->sync();
}


void MainWindow2::resizeEvent(QResizeEvent* event)
{
  QMainWindow::resizeEvent(event);

  ScheduleStateSave();
}

void MainWindow2::moveEvent(QMoveEvent* event)
{
  QMainWindow::moveEvent(event);

  ScheduleStateSave();
}



