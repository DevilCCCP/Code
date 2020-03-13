#include <Lib/Include/ModuleNames.h>

#include "UpDLoader.h"


bool UpDLoader::UpdateModules()
{
  return false;
}


UpDLoader::UpDLoader(const SettingsAS&)
  : ModuleLoaderA()
{
  AddModule(-1, kUpdateLoaderExe, QStringList(), QString());
}
