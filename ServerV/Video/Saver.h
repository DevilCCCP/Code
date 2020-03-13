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
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "Repo saver"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "R"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;

protected:
  /*override */virtual bool ProcessFrame() Q_DECL_OVERRIDE;

public:
  Saver(SettingsAS &_StorageSettings);
};

