#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectState.h>
#include <Lib/Db/Event.h>
#include <Lib/Db/Files.h>
#include <Lib/Log/Log.h>
#include <Lib/Common/Format.h>

#include "Core.h"


bool Core::ReloadSchema()
{
  mDb.getObjectTypeTable()->Reload();
  if (!mDb.getObjectTypeTable()->Open()) {
    if (!mLoadError) {
      Log.Warning("Load object_type fail");
      mLoadError = true;
    }
    return false;
  }

  mDb.getObjectStateValuesTable()->Reload();
  if (!mDb.getObjectStateValuesTable()->Open()) {
    if (!mLoadError) {
      Log.Warning("Load object_state_values fail");
      mLoadError = true;
    }
    return false;
  }

  mDb.getObjectTable()->Reload();
  if (!mDb.getObjectTable()->Open()) {
    if (!mLoadError) {
      Log.Warning("Load object fail");
      mLoadError = true;
    }
    return false;
  }
  if (!mDb.getObjectTable()->ReloadConnections()) {
    if (!mLoadError) {
      Log.Warning("Load object_connection fail");
      mLoadError = true;
    }
    return false;
  }

  if (!GetTypeId("srv", mServerTypeId, &mServerTypeName)) {
    Log.Warning(QString("Get 'srv' type fail"));
    mLoadError = true;
    return false;
  }
  GetTypeId("sch", mScheduleTypeId);

  mDb.getEventTypeTable()->Reload();
  mDb.getEventTable()->Reload();
  return true;
}

bool Core::GetTypeId(const QString& abbr, int& id, QString* name)
{
  if (const ObjectTypeItem* typeItem = static_cast<const ObjectTypeItem*>(mDb.getObjectTypeTable()->GetItemByName(abbr))) {
    id = typeItem->Id;
    if (name) {
      *name = typeItem->Descr;
    }
    return true;
  } else {
    id = 0;
    return false;
  }
}


Core::Core(Db &_Db)
  : mDb(_Db)
  , mLoadError(false)
{
  Q_INIT_RESOURCE(Monitoring);
}

