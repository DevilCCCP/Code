#pragma once

#include <qsystemdetection.h>
#include <QCoreApplication>
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
  QCoreApplication q(argc, argv);
  Log.SetFileLogging();
  Log.Info("Begin");
  qInstallMessageHandler(LogMessageHandler);
#ifdef Q_OS_WIN32
  TurnOffWerFault();
  qApp->addLibraryPath(qApp->applicationDirPath() + "/plugins/");
#endif
  Log.Info("Qt initialized");
  QDir::setCurrent(QCoreApplication::applicationDirPath());
  Log.Info(QString("Run in %1").arg(QDir::currentPath()));
  LICENSE_MAIN(false);
  SetConsoleOutputCP(65001);

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
  qInstallMessageHandler(nullptr);
  if (ret) {
    exit(ret);
  }
  return 0;
}

