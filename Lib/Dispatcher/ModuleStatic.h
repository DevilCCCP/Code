#pragma once

#include <Lib/Include/Common.h>

#include "ModuleLoaderD.h"


DefineClassS(ModuleStatic);

class ModuleStatic
{
  PROPERTY_GET_SET(int,          Id)
  PROPERTY_GET_SET(QString,      Path)
  PROPERTY_GET_SET(QStringList,  Params)
  PROPERTY_GET_SET(int,          StateFlag)
  ;

public:
  ModuleStatic()
    : mId(0), mStateFlag(eAllState)
  { }
  virtual ~ModuleStatic()
  { }
};
