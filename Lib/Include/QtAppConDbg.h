#pragma once

#include <qsystemdetection.h>
#include <QCoreApplication>
#include <QDir>

#include <Lib/Include/QtLog.h>
#include <Lib/Include/Common.h>
#include <Lib/Include/License.h>
#include <Lib/Log/Log.h>

#ifdef Q_OS_WIN32
#include "Win/WinWerfault.h"
#endif

void QtMustInitializeQapp_()
{
}

int qmain(int argc, char* argv[]);

int main(int argc, char* argv[])
{
  QDir dir;
  dir.mkpath("C:/Temp");
  Log.SetFile("C:/Temp/boot.log");
  QCoreApplication q(argc, argv);
  Log.Info("QCoreApplication done");
#if QT_VERSION >= 0x050000
  qInstallMessageHandler(LogMessageHandler);
#else
  qInstallMsgHandler(LogMessageHandler);
#endif
#ifdef Q_OS_WIN32
  Log.Info("TurnOffWerFault\n");
  TurnOffWerFault();
  Log.Info("qApp->addLibraryPath\n");
  qApp->addLibraryPath(qApp->applicationDirPath() + "/plugins/");
#endif
  Log.Info("QDir::setCurrent\n");
  QDir::setCurrent(QCoreApplication::applicationDirPath());
  Log.Info("Run in " + QDir::currentPath().toUtf8() + "\n");
  bool noTest = false;
  for (int i = 0; i < argc; i++) {
    if (QString(argv[i]) == "--skip-license") {
      noTest = true;
    }
  }
  Log.Info("Test license\n");
  if (!noTest) {
    LICENSE_MAIN(false);
  }
  Log.Info("Test license done\n");

  try {
    int ret = qmain(argc, argv);
    Log.Info(QString("Exit (code: %1)").arg(ret));
    if (ret) {
      exit(ret);
    }
    return ret;
  } catch (FatalException&) {
    Log.Info("Exit app due to fatal");
    return -1001;
  } catch (...) {
    Log.Info("Exit app due to unhandled exception");
    return -1002;
  }
}

