#include <Lib/Include/QtAppDaemon.h>
#include <Lib/Dispatcher/Dispatcher.h>
#include <Lib/Dispatcher/Service.h>
#include <Lib/Settings/FileSettings.h>
#include <Misc/ModuleNames.h>

#include "ServerLoader.h"


bool ServiceInit(QString& _ServiceName, QString& _ServiceViewname, QString& _ServiceDescription)
{
  _ServiceName        = kServerDaemon;
  _ServiceViewname    = kServerDaemon;
  _ServiceDescription = kServerDaemon;
  return true;
}

int ServiceMain(Service& service, int argc, char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  SettingsAS settings(new FileSettings());
  if (!settings.staticCast<FileSettings>()->OpenLocal()) {
    Log.Fatal("No settings");
    return -1;
  }

  return service.RunDaemon<ServerLoader>(kServerDaemon, settings);
}


