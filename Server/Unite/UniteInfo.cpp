#include <Lib/Db/ObjectType.h>
#include <Lib/Db/Event.h>
#include <Lib/Db/Files.h>
#include <Lib/Db/Variables.h>
#include <Lib/Crypto/InnerCrypt.h>

#include "UniteInfo.h"


bool UniteInfo::Init()
{
  mObjectType->Reload();
  if (const NamedItem* item = mObjectType->GetItemByName("srv")) {
    mServerTypeId = item->Id;
  } else {
    return false;
  }
  if (const NamedItem* item = mObjectType->GetItemByName("sr_")) {
    mUniteServerTypeId = item->Id;
  } else {
    return false;
  }
  if (const NamedItem* item = mObjectType->GetItemByName("cam")) {
    mCameraTypeId = item->Id;
  } else {
    return false;
  }
  if (const NamedItem* item = mObjectType->GetItemByName("aca")) {
    mCamera2TypeId = item->Id;
  } else {
    return false;
  }
  if (const NamedItem* item = mObjectType->GetItemByName("ca_")) {
    mUniteCameraTypeId = item->Id;
  } else {
    return false;
  }
//  if (!mObjectTable->GetDefaultTemplateObjectId("sr_", &mUniteServerId)) {
//    return false;
//  }
  return true;
}

bool UniteInfo::TranslateObject(const ObjectItemS& object)
{
  if (object->Type == mServerTypeId) {
    object->Type = mUniteServerTypeId;
    return true;
//  } else if (object->Type == mCameraTypeId) {
//    object->Type = mUniteCameraTypeId;
//    return true;
//  } else if (object->Type == mCamera2TypeId) {
//    object->Type = mUniteCameraTypeId;
//    return true;
  }
  return true;
}

bool UniteInfo::IsTranslatedObject(const ObjectItemS& object)
{
  return object->Type == mUniteServerTypeId || object->Type == mUniteCameraTypeId;
}

bool UniteInfo::ValidateId(const QString& objUuid, bool& valid, int* id)
{
  QMutexLocker lock(&mMutexDb);
  ObjectItemS obj;
  if (!mObjectTable->GetObjectByGuid(objUuid, obj)) {
    return false;
  }
  valid = !obj.isNull();
  if (id && valid) {
    *id = obj->Id;
  }
  return true;
}


UniteInfo::UniteInfo(const Overseer* _Overseer, const Db& _Db)
  : mOverseer(_Overseer), mDb(_Db), mObjectTable(new ObjectTable(_Db))
  , mObjectType(new ObjectType(_Db)), mEventTable(new EventTable(_Db))
  , mVariablesTable(new VariablesTable(_Db))
  , mInnerCrypt(new InnerCrypt())
{
}

