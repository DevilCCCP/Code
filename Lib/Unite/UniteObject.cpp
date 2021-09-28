#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectSettings.h>
#include <Lib/Db/Variables.h>

#include "UniteObject.h"
#include "UniteInfo.h"


bool UniteObject::ToJson(const ObjectItemS& obj, QByteArray& jsonData)
{
  mCurrentObject = obj;

  if (!LoadObjectSlaves() || !PackObjects()) {
    return false;
  }

  jsonData = mCurrentJsonDoc.toJson();
  return !jsonData.isEmpty();
}

bool UniteObject::FromJson(const QString& serverUuid, const QByteArray& pkey, const QByteArray& jsonData)
{
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(jsonData, &err);
  if (err.error != QJsonParseError::NoError) {
    Log.Warning(QString("UpdateObject: parse query json fail (err: '%1')").arg(err.errorString()));
    return false;
  }

  QJsonObject root = doc.object();
  QJsonValue jsonObjects = root.value("Objects");
  if (!jsonObjects.isArray()) {
    Log.Warning(QString("UpdateObject: parse json failed at Objects"));
    return false;
  }

  ObjectItemS serverObject;
  QMap<int, ObjectItemS> objs;
  QSet<int> usedObjs;
  int objTotal = 0;
  int objRemoved = 0;
  QJsonArray jsonObjectsArray = jsonObjects.toArray();
  for (auto itr = jsonObjectsArray.begin(); itr != jsonObjectsArray.end(); itr++) {
    const QJsonValue& jsonObject = *itr;
    if (!jsonObject.isObject()) {
      Log.Warning(QString("UpdateObject: parse json failed at Objects array"));
    }

    mCurrentObject.reset(new ObjectItem());
    QJsonObject jsonObjectNode = jsonObject.toObject();
    QString typeName         = jsonObjectNode.value("Type").toString();
    if (!GetObjectTypeId(typeName, mCurrentObject->Type)) {
      return false;
    }

    int id                   = jsonObjectNode.value("Id").toInt();
    mCurrentObject->Name     = jsonObjectNode.value("Name").toString();
    mCurrentObject->Descr    = jsonObjectNode.value("Descr").toString();
    mCurrentObject->Guid     = jsonObjectNode.value("Guid").toString();
    mCurrentObject->Version  = jsonObjectNode.value("Version").toString();
    mCurrentObject->Revision = jsonObjectNode.value("Revision").toInt();
    mCurrentObject->Uri      = jsonObjectNode.value("Uri").toString();
    mCurrentObject->Status   = jsonObjectNode.value("Status").toInt();
    int masterId             = jsonObjectNode.value("Master").toInt();

    mCurrentSettings.clear();
    QJsonValue jsonObjectsSettings = jsonObjectNode.value("Settings");
    if (!jsonObjectsSettings.isArray()) {
      Log.Warning(QString("UpdateObject: parse json failed at Object settings"));
      return false;
    }
    QJsonArray jsonObjectsSettingsArray = jsonObjectsSettings.toArray();
    for (auto itr = jsonObjectsSettingsArray.begin(); itr != jsonObjectsSettingsArray.end(); itr++) {
      const QJsonValue& jsonObject = *itr;
      if (!jsonObject.isObject()) {
        Log.Warning(QString("UpdateObject: parse json failed at Object settings array"));
      }

      QJsonObject jsonObjectNode = jsonObject.toObject();
      ObjectSettingsS settings(new ObjectSettings());

      settings->ObjectId = mCurrentObject->Id;
      settings->Key      = jsonObjectNode.value("Key").toString();
      settings->Value    = jsonObjectNode.value("Value").toString();
      mCurrentSettings.append(settings);
    }

//    mCurrentVariables.clear();
//    QJsonValue jsonObjectsVariables = jsonObjectNode.value("Variables");
//    if (!jsonObjectsVariables.isArray()) {
//      Log.Warning(QString("UpdateObject: parse json failed at Object variables"));
//      return false;
//    }
//    QJsonArray jsonObjectsVariablesArray = jsonObjectsVariables.toArray();
//    for (auto itr = jsonObjectsVariablesArray.begin(); itr != jsonObjectsVariablesArray.end(); itr++) {
//      const QJsonValue& jsonObject = *itr;
//      if (!jsonObject.isObject()) {
//        Log.Warning(QString("UpdateObject: parse json failed at Object variables array"));
//      }

//      QJsonObject jsonObjectNode = jsonObject.toObject();
//      VariablesS variables(new Variables());

//      variables->Object   = mCurrentObject->Id;
//      variables->Key      = jsonObjectNode.value("Key").toString();
//      variables->Value    = jsonObjectNode.value("Value").toString();
//      mCurrentVariables.append(variables);
//    }

    if (!mCurrentObject->Id && !UpdateObjectOne()) {
      return false;
    }

    objs[id] = mCurrentObject;
    if (mCurrentObject->Guid == serverUuid) {
      serverObject = mCurrentObject;
      if (!UpdateObjectSettings()/* || !UpdateObjectVariables()*/) {
        return false;
      }
      continue;
    }

    if (masterId) {
      auto itr = objs.find(masterId);
      if (itr == objs.end()) {
        LOG_WARNING_ONCE("Unite with bad master");
        return false;
      }
      ObjectItemS masterObject = itr.value();
      if (!masterObject->Id) {
        if (!mDb.getObjectTable()->GetObjectIdByGuid(masterObject->Guid, masterObject->Id)) {
          return false;
        }
      }
      mCurrentObject->ParentId = masterObject->Id;
    }
    if (!UpdateObjectSettings()/* || !UpdateObjectVariables()*/ || !UpdateParentConnection() || !UpdateObjectOne()) {
      return false;
    }
    usedObjs.insert(mCurrentObject->Id);

    objTotal++;
  }

  if (!serverObject || serverObject->Id == 0) {
    return false;
  }

  if (!RemoveUnused(serverObject->Id, usedObjs, objRemoved)) {
    return false;
  }

  if (!mDb.getObjectTable()->UpdateItem(serverObject)) {
    return false;
  }

  if (!pkey.isEmpty()) {
    VariablesS pKeyVariable;
    if (!mDb.getVariablesTable()->SelectVariable(serverObject->Id, "Pkey", pKeyVariable)) {
      return false;
    }
    if (!pKeyVariable) {
      Log.Info(QString("Setup Pkey for object %1").arg(serverObject->Id));
      if (!mDb.getVariablesTable()->InsertVariable(serverObject->Id, "Pkey", QString::fromLatin1(pkey))) {
        return false;
      }
    }
  }

  Log.Info(QString("Update object done (name: '%1', total: %2, removed: %3)").arg(mCurrentObject->Name).arg(objTotal).arg(objRemoved));
  return true;
}

bool UniteObject::LoadObjectSlaves()
{
  mUpdateObjects.clear();
  QMap<int, ObjectItemS> items;
  if (!mDb.getObjectTable()->LoadSlavesOfTypes(mCurrentObject->Id, mUniteInfo->GetSyncTypeIds(), items)) {
    return false;
  }
  for (auto itr = items.begin(); itr != items.end(); itr++) {
    const ObjectItemS& item = itr.value();
    mUpdateObjects.append(UpdateObject(item, mCurrentObject->Id));
  }

  QList<ObjectItemS> parents = items.values();
  while (!parents.isEmpty()) {
    QList<ObjectItemS> nextParents;
    for (auto itr = parents.begin(); itr != parents.end(); itr++) {
      const ObjectItemS& parent = *itr;
      QMap<int, ObjectItemS> slaves;
      if (!mDb.getObjectTable()->LoadSlaves(parent->Id, slaves)) {
        return false;
      }
      for (auto itr = slaves.begin(); itr != slaves.end(); itr++) {
        const ObjectItemS& item = itr.value();
        mUpdateObjects.append(UpdateObject(item, parent->Id));
      }
      nextParents.append(slaves.values());
    }
    parents = nextParents;
  }

  mUpdateObjects.prepend(UpdateObject(mCurrentObject, 0));
  return true;
}

bool UniteObject::PackObjects()
{
  QJsonObject root;
  QJsonArray jsonObjectsArray;
  for (auto itr = mUpdateObjects.begin(); itr != mUpdateObjects.end(); itr++) {
    const UpdateObject& upInfo = *itr;
    const ObjectItem* obj = upInfo.Object.data();
    QJsonObject jsonObjectNode;
    TableItemS typeObj = mDb.getObjectTypeTable()->GetItem(obj->Type);
    if (!typeObj) {
      return false;
    }
    const ObjectTypeItem* type = static_cast<const ObjectTypeItem*>(typeObj.data());

    jsonObjectNode.insert("Id",       obj->Id);
    jsonObjectNode.insert("Type",     type->Name);
    jsonObjectNode.insert("Name",     obj->Name);
    jsonObjectNode.insert("Descr",    obj->Descr);
    jsonObjectNode.insert("Guid",     obj->Guid);
    jsonObjectNode.insert("Version",  obj->Version);
    jsonObjectNode.insert("Revision", obj->Revision);
    jsonObjectNode.insert("Uri",      obj->Uri);
    jsonObjectNode.insert("Status",   obj->Status);
    jsonObjectNode.insert("Master",   upInfo.Master);

    QList<ObjectSettingsS> settings;
    if (!mDb.getObjectSettingsTable()->GetObjectSettings(obj->Id, settings, false)) {
      return false;
    }
    QJsonArray jsonObjectsSettingsArray;
    foreach (const ObjectSettingsS& setting, settings) {
      QJsonObject jsonSettingNode;
      jsonSettingNode.insert("Key",       setting->Key);
      jsonSettingNode.insert("Value",     setting->Value);

      jsonObjectsSettingsArray.append(QJsonValue(jsonSettingNode));
    }
    jsonObjectNode.insert("Settings", QJsonValue(jsonObjectsSettingsArray));

//    QList<VariablesS> variables;
//    if (!mDb.getVariablesTable()->SelectVariables(obj->Id, variables)) {
//      return false;
//    }
//    QJsonArray jsonObjectsVariablesArray;
//    foreach (const VariablesS& variable, variables) {
//      QJsonObject jsonVariableNode;
//      jsonVariableNode.insert("Key",       variable->Key);
//      jsonVariableNode.insert("Value",     variable->Value);

//      jsonObjectsVariablesArray.append(QJsonValue(jsonVariableNode));
//    }
//    jsonObjectNode.insert("Variables", QJsonValue(jsonObjectsVariablesArray));

    jsonObjectsArray.append(QJsonValue(jsonObjectNode));
  }
  root.insert("Objects", QJsonValue(jsonObjectsArray));

  mCurrentJsonDoc.setObject(root);
  return true;
}

bool UniteObject::GetObjectTypeId(const QString& typeName, int& id)
{
  if (const NamedItem* item = mDb.getObjectTypeTable()->GetItemByName(typeName)) {
    id = item->Id;
    return true;
  }
  return false;
}

bool UniteObject::UpdateObjectOne()
{
  if (!mUniteInfo->TranslateObject(mCurrentObject)) {
    return false;
  }

  ObjectItemS localObject;
  if (!mDb.getObjectTable()->GetObjectByGuid(mCurrentObject->Guid, localObject)) {
    return false;
  }
  if (localObject) {
    if (mCurrentObject->Type != localObject->Type) {
      Log.Warning(QString("Unite item fail (uuid: '%1', type: %2, new type: %3)")
                  .arg(mCurrentObject->Guid).arg(localObject->Type).arg(mCurrentObject->Type));
      return false;
    }
    mCurrentObject->Id = localObject->Id;
    if (localObject->IsEqual(*mCurrentObject)) {
      return true;
    }
  }

  bool isNewItem = mCurrentObject->Id == 0;
  if (isNewItem) {
    if (!mDb.getObjectTable()->InsertItem(mCurrentObject)) {
      return false;
    }
  } else {
    if (!mDb.getObjectTable()->UpdateItem(mCurrentObject)) {
      return false;
    }
//    if (!table->RemoveSlaves(mCurrentObject->Id)) {
//      return false;
//    }
  }
  Log.Info(QString("%1 item (Name: '%2', Uuid: '%3')").arg(isNewItem? "Registered": "Update")
           .arg(mCurrentObject->Name).arg(mCurrentObject->Guid));
  return true;
}

bool UniteObject::UpdateObjectSettings()
{
  if (!mCurrentObject->Id) {
    if (!mDb.getObjectTable()->GetObjectIdByGuid(mCurrentObject->Guid, mCurrentObject->Id)) {
      return false;
    }
  }

  QList<ObjectSettingsS> oldSettings;
  if (!mDb.getObjectSettingsTable()->GetObjectSettings(mCurrentObject->Id, oldSettings, false)) {
    return false;
  }

  foreach (const ObjectSettingsS& setting, mCurrentSettings) {
    bool isNew = true;
    for (auto itr = oldSettings.begin(); itr != oldSettings.end(); itr++) {
      const ObjectSettingsS& oldSetting = *itr;
      if (oldSetting->Key != setting->Key) {
        continue;
      }

      if (oldSetting->Value != setting->Value) {
        oldSetting->Value = setting->Value;
        if (!mDb.getObjectSettingsTable()->UpdateItem(oldSetting)) {
          return false;
        }
      }
      oldSettings.erase(itr);
      isNew = false;
      break;
    }
    if (isNew) {
      setting->ObjectId = mCurrentObject->Id;
      if (!mDb.getObjectSettingsTable()->InsertItem(setting)) {
        return false;
      }
    }
  }

  for (auto itr = oldSettings.begin(); itr != oldSettings.end(); itr++) {
    const ObjectSettingsS& oldSetting = *itr;
    if (!mDb.getObjectSettingsTable()->RemoveItem(oldSetting)) {
      return false;
    }
  }
  return true;
}

bool UniteObject::UpdateObjectVariables()
{
  if (!mCurrentObject->Id) {
    if (!mDb.getObjectTable()->GetObjectIdByGuid(mCurrentObject->Guid, mCurrentObject->Id)) {
      return false;
    }
  }

  QList<VariablesS> oldVariables;
  if (!mDb.getVariablesTable()->SelectVariables(mCurrentObject->Id, oldVariables)) {
    return false;
  }

  foreach (const VariablesS& variable, mCurrentVariables) {
    bool isNew = true;
    for (auto itr = oldVariables.begin(); itr != oldVariables.end(); itr++) {
      const VariablesS& oldVariable = *itr;
      if (oldVariable->Key != variable->Key) {
        continue;
      }

      if (oldVariable->Value != variable->Value) {
        oldVariable->Value = variable->Value;
        if (!mDb.getVariablesTable()->Update(oldVariable)) {
          return false;
        }
      }
      oldVariables.erase(itr);
      isNew = false;
      break;
    }
    if (isNew) {
      variable->Object = mCurrentObject->Id;
      if (!mDb.getVariablesTable()->Insert(variable)) {
        return false;
      }
    }
  }

  for (auto itr = oldVariables.begin(); itr != oldVariables.end(); itr++) {
    const VariablesS& oldVariables = *itr;
    if (!mDb.getVariablesTable()->Delete(oldVariables)) {
      return false;
    }
  }
  return true;
}

bool UniteObject::UpdateParentConnection()
{
  if (!mCurrentObject->ParentId) {
    return true;
  }

  QList<int> masters;
  if (!mDb.getObjectTable()->LoadMasters(mCurrentObject->Id, masters)) {
    return false;
  }
  if (masters.contains(mCurrentObject->ParentId)) {
    return true;
  }
  return mDb.getObjectTable()->CreateLink(mCurrentObject->ParentId, mCurrentObject->Id);
}

bool UniteObject::RemoveUnused(int serverId, const QSet<int>& usedObjs, int& objRemoved)
{
  QSet<int> existed;
  QList<int> level;
  QList<int> nextLevel;
  for (level.append(serverId); !level.isEmpty(); level.swap(nextLevel), nextLevel.clear()) {
    for (auto itr = level.begin(); itr != level.end(); itr++) {
      int id = *itr;
      mDb.getObjectTable()->LoadSlaves(id, nextLevel);
    }
    foreach(int idSlave, nextLevel) {
      existed.insert(idSlave);
    }
  }

  existed.subtract(usedObjs);
  for (auto itr = existed.begin(); itr != existed.end(); itr++) {
    int id = *itr;
    if (!mDb.getObjectTable()->RemoveItem(id)) {
      return false;
    }
  }
  objRemoved = existed.size();
  return true;
}

UniteObject::UniteObject(const Db& _Db, UniteInfo* _UniteInfo)
  : mDb(_Db), mUniteInfo(_UniteInfo)
{
}
