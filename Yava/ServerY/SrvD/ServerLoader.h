#pragma once

#include <QList>

#include <Lib/Dispatcher/ModuleLoaderO.h>


class ServerLoader: public ModuleLoaderO
{
protected:
  /*override */virtual void SetDbScheme() Q_DECL_OVERRIDE;

  /*override */virtual bool IsNeedIp() Q_DECL_OVERRIDE;

public:
  ServerLoader(SettingsAS& _Settings);
};

