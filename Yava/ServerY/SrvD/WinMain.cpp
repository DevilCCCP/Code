#include "stdafx.h"

#include <QSharedPointer>

#include <Lib/Include/QtAppCon.h>
#include <Lib/Dispatcher/Dispatcher.h>
#include <Lib/Settings/FileSettings.h>
#include <Misc/ModuleNames.h>

#include "ServerLoader.h"


int qmain(int argc, char* argv[])
{
  FileSettingsS settings(new FileSettings());
  if (!settings->OpenLocal()) {
    Log.Fatal("No settings");
    return -1;
  }

  const QString _Name = QString::fromUtf8("yava_srvd");
  const QString _Viewname = QString::fromUtf8("СНА Сервер");
  const QString _Description = QString::fromUtf8("Сервис СНА, осуществляющий контроль работы компонент видеосервера");
  DispatcherDaemon<ServerLoader> devil(kServerDaemon, settings.staticCast<SettingsA>());
#ifdef Q_OS_WIN32
  auto oldState = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
  if (oldState == NULL) {
    Log.Error("SetThreadExecutionState execution fail");
  }
#endif

  return devil.RunService(_Name, _Viewname, _Description, argc, argv);
}

