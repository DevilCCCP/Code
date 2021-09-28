#pragma once

#include <LibV/Include/Frame.h>
#include <LibV/Include/ConveyorV.h>
#include <Lib/Include/License_h.h>


DefineClassS(Db);
DefineClassS(SettingsA);
DefineClassS(Storage);
DefineClassS(Saver);

class Saver: public ConveyorV
{
  SettingsAS    mStorageSettings;
  StorageS      mStorage;
  bool          mWriteError;

  LICENSE_HEADER;

public:
  /*override */virtual const char* Name() override { return "Repo saver"; }
  /*override */virtual const char* ShortName() override { return "R"; }
protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual void DoRelease() override;

protected:
  /*override */virtual bool ProcessFrame() override;

public:
  Saver(SettingsAS &_StorageSettings);
};

