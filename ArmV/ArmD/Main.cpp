#include <QSharedPointer>
#include <qsystemdetection.h>
#ifdef Q_OS_WIN32
#include <Windows.h>
#endif

#include <Lib/Include/QtAppCon.h>
#include <Lib/Dispatcher/Dispatcher.h>
#include <Lib/Settings/FileSettings.h>
#include <Local/ModuleNames.h>

#include "ArmLoader.h"


int qmain(int argc, char* argv[])
{
#ifdef QT_NO_DEBUG
  Log.SetFileLogging();
#endif

  FileSettingsS settings(new FileSettings());
  if (!settings->OpenLocal()) {
    Log.Fatal("No settings");
    return -1;
  }

  const QString _Name = QString::fromUtf8(kArmDaemon);
  const QString& _Viewname = GetArmViewName();
  const QString& _Description = GetArmDescription();
  DispatcherDaemon<ArmLoader> devil(kArmDaemon, settings.staticCast<SettingsA>());
#ifdef Q_OS_WIN32
  auto oldState = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
  if (oldState == 0) {
    Log.Error("SetThreadExecutionState execution fail");
  }
#endif
  return devil.RunService(_Name, _Viewname, _Description, argc, argv);
}

