#pragma once

#include <qsystemdetection.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QDir>

#include <Lib/Include/QtLog.h>
#include <Lib/Include/Common.h>
#include <Lib/Include/License.h>
#include <Lib/Db/Db.h>
#include <Lib/Updater/UpInfo.h>

#ifdef Q_OS_WIN32
#include "Win/WinWerfault.h"
#endif


void QtMustInitializeQapp_()
{
}

int qmain(int argc, char* argv[]);

int TestLicVerbose()
{
  LICENSE_MAIN(true);
  return 0;
}

const QString GetProgramName();

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

  if (int errCode = TestLicVerbose()) {
    QMessageBox::critical(qApp->desktop(), GetProgramName()
#ifdef LANG_EN
                          , QString::fromUtf8("Fail to start at license checking"));
#else
                          , QString::fromUtf8("Невозможно запустить из-за ошибки проверки лицензии"));
#endif
    return errCode;
  }

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

#ifdef NO_UPDATER
int ConnectPrimaryDb(Db& db)
{
#else
int ConnectPrimaryDb(Db& db, UpInfo* upInfo)
{
  if (upInfo) {
    upInfo->Attach();
  }
#endif
  if (!db.OpenDefault()) {
    QMessageBox::critical(qApp->desktop(), GetProgramName()
#ifdef LANG_EN
                          , QString::fromUtf8("Fail to start at DB connection"));
#else
                          , QString::fromUtf8("Невозможно подключиться к базе данных, возможно необходима переустановка"));
#endif
    return -2002;
  }

  if (!db.Connect()) {
    Log.Warning(QString("Connect to db fail"));
    for (int i = 0; i < 30; i++) {
#ifndef NO_UPDATER
      if (upInfo && upInfo->Check()){
        return 1;
      }
#endif
      if (!db.Connect() && i >= 29) {
        Log.Fatal(QString("Connect to db fail all tries"));
        QMessageBox::critical(qApp->desktop(), GetProgramName()
#ifdef LANG_EN
                              , QString::fromUtf8("Fail to start, DB unavailable"));
#else
                              , QString::fromUtf8("База данных недоступна, запуск невозможен"));
#endif
        return -2003;
      }
    }
  }

  return 0;
}
