#pragma once

#include <QMap>
#include <QString>

#include <Lib/Dispatcher/Dispatcher.h>
#include <Lib/Include/License_h.h>

class SettingMLoader: public ModuleLoaderB
{
  QMap<int, QPair<QString, QString> > mServices;

protected:
  /*override */virtual bool UpdateModules() override;

public:
  SettingMLoader(SettingsAS& _Settings);
  /*override */virtual ~SettingMLoader();
};
