#pragma once

#include <qsystemdetection.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QDir>

#include <Lib/Include/QtLog.h>
#include <Lib/Include/Common.h>
#include <Lib/Include/License.h>

#ifdef Q_OS_WIN32
#include "Win/WinWerfault.h"
#endif


void QtMustInitializeQapp_()
{
}

int qmain(int argc, char* argv[]);

int main(int argc, char* argv[])
{
  Log.Info("Begin");
  QApplication q(argc, argv);
#if QT_VERSION >= 0x050000
  qInstallMessageHandler(LogMessageHandler);
#else
  qInstallMsgHandler(LogMessageHandler);
#endif
#ifdef Q_OS_WIN32
  TurnOffWerFault();
  qApp->addLibraryPath(qApp->applicationDirPath() + "/plugins/");
#endif
  Log.Info("Qt initialized");
  QDir::setCurrent(QCoreApplication::applicationDirPath());
  Log.Info(QString("Run in %1").arg(QDir::currentPath()));
  qApp->desktop();
  LICENSE_MAIN(true);

  int ret = 0;
  try {
    ret = qmain(argc, argv);
    Log.Info(QString("Exit (code: %1)").arg(ret));
  } catch (FatalException&) {
    Log.Info("Exit app due to fatal");
    ret = 1;
  } catch (...) {
    Log.Info("Exit app due to unhandled exception");
    ret = 2;
  }
#if QT_VERSION >= 0x050000
  qInstallMessageHandler(nullptr);
#else
  qInstallMsgHandler(nullptr);
#endif
  if (ret) {
    exit(ret);
  }
  return 0;
}

