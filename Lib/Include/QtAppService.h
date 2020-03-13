#pragma once

#include <qsystemdetection.h>
#include <QCoreApplication>
#include <QString>
#include <QDir>

#include <Lib/Include/QtLog.h>
#include <Lib/Include/Common.h>
#include <Lib/Dispatcher/Dispatcher.h>

#ifdef Q_OS_WIN32
#include "Win/WinWerfault.h"
#endif


void QtMustInitializeQapp_()
{
}

bool ServiceInit(QString& _ServiceName, QString& _ServiceViewname, QString& _ServiceDescription);
DispatcherS CreateServiceDispatcher();

static QString gServiceName;
static QString gServiceViewname;
static QString gServiceDescription;

int main(int argc, char* argv[])
{
  QCoreApplication q(argc, argv);

  if (!ServiceInit(gServiceName, gServiceViewname, gServiceDescription)) {
    return -1;
  }

  QDir::setCurrent(QCoreApplication::applicationDirPath());
//#ifdef QT_NO_DEBUG
//  Log.SetFileLogging();
//#endif
  Log.Info(QString("Run in %1").arg(QDir::currentPath()));

#if QT_VERSION >= 0x050000
  qInstallMessageHandler(LogMessageHandler);
#else
  qInstallMsgHandler(LogMessageHandler);
#endif
#ifdef Q_OS_WIN32
  TurnOffWerFault();
  qApp->addLibraryPath(qApp->applicationDirPath() + "/plugins/");

  auto oldState = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
  if (oldState == 0) {
    Log.Error("SetThreadExecutionState execution fail");
  }
#endif
  Log.Info("Qt initialized");

  int ret = 0;
  try {
    DispatcherS devil = CreateServiceDispatcher();
    ret = devil? devil->RunService(gServiceName, gServiceViewname, gServiceDescription, argc, argv): -1;
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

