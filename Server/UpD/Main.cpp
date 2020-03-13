#include <QSharedPointer>

#include <Lib/Include/QtAppService.h>
#include <Lib/Dispatcher/Dispatcher.h>
#include <Lib/Settings/FileSettings.h>
#include <Local/ModuleNames.h>

#include "UpDLoader.h"


bool ServiceInit(QString& _ServiceName, QString& _ServiceViewname, QString& _ServiceDescription)
{
  _ServiceName        = kUpdateDaemon;
  _ServiceViewname    = GetUpdateViewName();
  _ServiceDescription = GetUpdateDescription();
  return true;
}

DispatcherS CreateServiceDispatcher()
{
  return DispatcherS(new DispatcherDaemon<UpDLoader>(kUpdateDaemon, SettingsAS()));
}
