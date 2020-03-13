#include "ObjectSettings.h"


const char* ObjectSettingsTable::Name()
{
  return "object_settings";
}

const char* ObjectSettingsTable::Select()
{
  return "SELECT _id, _object, key, value FROM object_settings";
}

const char* ObjectSettingsTable::Insert()
{
  return "INSERT INTO object_settings(_object, key, value)"
         " VALUES (?, ?, ?)";
}

const char* ObjectSettingsTable::Update()
{
  return "UPDATE object_settings SET _object = ?, key = ?, value = ?";
}

const char* ObjectSettingsTable::Delete()
{
  return "DELETE FROM object_settings";
}

bool ObjectSettingsTable::OnRowFillItem(QueryS& q, TableItemS& unit)
{
  ObjectSettings* item;
  unit = TableItemS(item = new ObjectSettings());
  int index = 1;
  item->ObjectId = q->value(index++).toInt();
  item->Key = q->value(index++).toString();
  item->Value = q->value(index++).toString();
  return true;
}

bool ObjectSettingsTable::OnSetItem(QueryS& q, const TableItem& unit)
{
  const ObjectSettings& item = static_cast<const ObjectSettings&>(unit);
  int index = 0;
  q->bindValue(index++, item.ObjectId);
  q->bindValue(index++, item.Key);
  q->bindValue(index++, item.Value);
  return true;
}

void ObjectSettingsTable::CreateIndexes()
{
  for (auto itr = mItems.begin(); itr != mItems.end(); itr++) {
    const TableItemS& item = itr.value();
    const ObjectSettingsS& objectSettings = item.staticCast<ObjectSettings>();
    mObjectSettingsIndex[objectSettings->ObjectId].append(objectSettings);
  }
  mIndexed = true;
}

void ObjectSettingsTable::ClearIndexes()
{
  mObjectSettingsIndex.clear();
  mIndexed = false;
}

bool ObjectSettingsTable::GetObjectSettings(int objectId, QList<ObjectSettingsS>& settings, bool useCache)
{
  if (useCache) {
    settings = mObjectSettingsIndex[objectId];
    return true;
  }

  if (!LoadWhere(QString("WHERE _object = %1").arg(objectId))) {
    return false;
  }

  UpdateSettings(settings);
  return true;
}

bool ObjectSettingsTable::GetObjectSettings(QString key, QString value, QList<ObjectSettingsS>& settings)
{
  if (!LoadWhere(QString("WHERE key = '%1' AND value = '%2'")
                 .arg(key).arg(value))) {
    return false;
  }

  UpdateSettings(settings);
  return true;
}

bool ObjectSettingsTable::GetObjectSettings(int objectId, QString key, QString value, QList<ObjectSettingsS>& settings)
{
  if (!LoadWhere(QString("WHERE _object = %1 AND key = '%2' AND value = '%3'")
                 .arg(objectId).arg(key).arg(value))) {
    return false;
  }

  UpdateSettings(settings);
  return true;
}

void ObjectSettingsTable::UpdateSettings(QList<ObjectSettingsS> &settings)
{
  settings.clear();
  for (auto itr = mItems.begin(); itr != mItems.end(); itr++) {
    const TableItemS& item = itr.value();
    settings.append(item.staticCast<ObjectSettings>());
  }
}


ObjectSettingsTable::ObjectSettingsTable(const Db& _Db)
  : Table(_Db)
  , mIndexed(false)
{
}
