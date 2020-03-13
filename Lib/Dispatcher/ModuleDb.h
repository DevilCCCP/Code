#pragma once

#include <Lib/Include/Common.h>


DefineClassS(ModuleDb);

class ModuleDb
{
  PROPERTY_GET_SET(int,          Id)
  PROPERTY_GET_SET(QString,      Path)
  PROPERTY_GET_SET(QStringList,  Params)

  PROPERTY_GET_SET(int,          Revision)
  PROPERTY_GET_SET(int,          State)
  PROPERTY_GET_SET(QString,      Uri)

  PROPERTY_GET_SET(int,          Port)
  PROPERTY_GET_SET(bool,         Active)
  ;
protected:
  virtual bool Eq(const ModuleDb& other)
  {
    return mPath == other.mPath && mParams == other.mParams
        && mRevision == other.mRevision && mState == other.mState && mUri == other.mUri
        && mActive == other.mActive;
  }

public:
  bool operator==(const ModuleDb& other) { return Eq(other); }
  bool operator!=(const ModuleDb& other) { return !Eq(other); }

public:
  ModuleDb()
    : mId(0), mPort(0), mActive(true)
  { }
  virtual ~ModuleDb()
  { }
};
