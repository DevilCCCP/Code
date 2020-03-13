#include "stdafx.h"

#include <Misc/ModuleNames.h>

#include "ServerLoader.h"


void ServerLoader::SetDbScheme()
{
  RegisterDbModule("cam", kVideoAExe, 0);
  RegisterDbModule("rep", kStoreCreatorExe, 1);
}

bool ServerLoader::IsNeedIp()
{
  return true;
}


ServerLoader::ServerLoader(SettingsAS &_Settings)
  : ModuleLoaderO(_Settings, kUpdaterSrvExe, "srv", 20000)
{
}
