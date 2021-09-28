#pragma once

#include <QList>

#include <Lib/Dispatcher/ModuleLoaderA.h>


DefineClassS(SettingsA);

class UpDLoader: public ModuleLoaderA
{
protected:
  /*override */virtual bool UpdateModules() override;

public:
  UpDLoader(const SettingsAS&);
};

