#include <QMutexLocker>

#include <Lib/Db/ObjectType.h>
#include <Lib/Db/Event.h>
#include <Lib/Db/Files.h>
#include <Lib/Db/Variables.h>
#include <Lib/Crypto/InnerCrypt.h>

#include "UniteInfo.h"


bool UniteInfo::AddSyncObject(const Db& db, const QString& typeSync)
{
  db.getObjectTypeTable()->Load();
  const NamedItem* item = db.getObjectTypeTable()->GetItemByName(typeSync);
  if (item) {
    mUnitTypeIdMap[item->Id] = item->Id;
    mUnitedTypeIdList.insert(item->Id);
    return true;
  }
  return false;
}

bool UniteInfo::AddSyncObject(const Db& db, const QString& typeFrom, const QString& typeTo)
{
  db.getObjectTypeTable()->Load();
  const NamedItem* itemFrom = db.getObjectTypeTable()->GetItemByName(typeFrom);
  const NamedItem* itemTo   = db.getObjectTypeTable()->GetItemByName(typeTo);
  if (itemFrom && itemTo) {
    mUnitTypeIdMap[itemFrom->Id] = itemTo->Id;
    mUnitedTypeIdList.insert(itemTo->Id);
    return true;
  }
  return false;
}

const QStringList& UniteInfo::GetSyncTypeIdList()
{
  if (mSyncTypeIdList.isEmpty()) {
    ConstructSyncTypeIds();
  }
  return mSyncTypeIdList;
}

const QString& UniteInfo::GetSyncTypeIds()
{
  if (mSyncTypeIds.isEmpty()) {
    ConstructSyncTypeIds();
  }
  return mSyncTypeIds;
}

bool UniteInfo::IsTranslatedObject(const ObjectItemS& object)
{
  return mUnitedTypeIdList.contains(object->Type);
}

bool UniteInfo::TranslateObject(const ObjectItemS& object)
{
  auto itr = mUnitTypeIdMap.find(object->Type);
  if (itr != mUnitTypeIdMap.end()) {
    object->Type = itr.value();
    return true;
  }
  return false;
}

bool UniteInfo::ValidateId(const Db& db, const QString& objUuid, bool& valid, int* id)
{
  ObjectItemS obj;
  if (!db.getObjectTable()->GetObjectByGuid(objUuid, obj)) {
    return false;
  }
  valid = !obj.isNull();
  if (id && valid) {
    *id = obj->Id;
  }
  return true;
}

void UniteInfo::SetUniteStage(int objectId, int stage)
{
  UniteStat* uniteStat = FindUniteStat(objectId);
  UniteStat::UniteStageStat* uniteStageStat = &uniteStat->UniteStageStatMap[stage];
  uniteStageStat->OkCount++;
  if (!uniteStageStat->LastOk) {
    if (++uniteStageStat->MessageCount >= uniteStageStat->NextMessage) {
      uniteStageStat->NextMessage *= 2;
      Log.Info(QString("Unite stage done (id: %1, stage: %2, ok: %3, fail: %4)")
               .arg(objectId).arg(stage).arg(uniteStageStat->OkCount).arg(uniteStageStat->FailCount));
    }
    uniteStageStat->LastOk = true;
  }
}

void UniteInfo::SetUniteStageFail(int objectId, int stage)
{
  UniteStat* uniteStat = FindUniteStat(objectId);
  UniteStat::UniteStageStat* uniteStageStat = &uniteStat->UniteStageStatMap[stage];
  uniteStageStat->FailCount++;
  if (!uniteStageStat->LastOk) {
    if (++uniteStageStat->MessageCount >= uniteStageStat->NextMessage) {
      uniteStageStat->NextMessage *= 2;
      Log.Warning(QString("Unite stage fail (id: %1, stage: %2, ok: %3, fail: %4)")
                  .arg(objectId).arg(stage).arg(uniteStageStat->OkCount).arg(uniteStageStat->FailCount));
    }
    uniteStageStat->LastOk = false;
  }
}

void UniteInfo::WarnOnce(const QString& text)
{
  QMutexLocker lock(&mMutex);
  if (!mWarningMessages.contains(text)) {
    mWarningMessages.insert(text);
    lock.unlock();
    Log.Warning(text);
  }
}

void UniteInfo::ConstructSyncTypeIds()
{
  for (auto itr = mUnitTypeIdMap.begin(); itr != mUnitTypeIdMap.end(); itr++) {
    mSyncTypeIdList << QString::number(itr.key());
  }
  mSyncTypeIds = mSyncTypeIdList.join(",");
}

UniteStat* UniteInfo::FindUniteStat(int objectId)
{
  QMutexLocker lock(&mMutex);
  auto itr = mUniteStageMap.find(objectId);
  if (itr == mUniteStageMap.end()) {
    itr = mUniteStageMap.insert(objectId, UniteStat());
  }
  return &*itr;
}


UniteInfo::UniteInfo(int _ServiceId)
  : mServiceId(_ServiceId)
{
}

