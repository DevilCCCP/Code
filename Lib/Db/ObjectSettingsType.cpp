#include "ObjectSettingsType.h"


const char* ObjectSettingsTypeTable::Name()
{
  return "object_settings_type";
}

const char* ObjectSettingsTypeTable::Select()
{
  return "SELECT _id, name, descr, _otype, key, type, min_value, max_value FROM object_settings_type";
}

bool ObjectSettingsTypeTable::OnRowFillItem(QueryS& q, TableItemS& unit)
{
  ObjectSettingsType* item;
  unit = TableItemS(item = new ObjectSettingsType());
  int index = 3;
  item->ObjectTypeId = q->value(index++).toInt();
  item->Key = q->value(index++).toString();
  item->Type = q->value(index++).toString();
  item->MinValue = q->value(index++).toString();
  item->MaxValue = q->value(index++).toString();
  return TableNamed::OnRowFillItem(q, unit);
}

void ObjectSettingsTypeTable::CreateIndexes()
{
  for (auto itr = mItems.begin(); itr != mItems.end(); itr++) {
    const ObjectSettingsType* item = static_cast<const ObjectSettingsType*>(itr.value().data());
    mTypeKeyIndexs[item->ObjectTypeId][item->Key] = item;
  }
}

void ObjectSettingsTypeTable::ClearIndexes()
{
  mTypeKeyIndexs.clear();
}

const ObjectSettingsType* ObjectSettingsTypeTable::GetObjectTypeSettingsType(int typeId, const QString& key)
{
  auto itr = mTypeKeyIndexs.find(typeId);
  if (itr != mTypeKeyIndexs.end()) {
    const QMap<QString, const ObjectSettingsType*>& map = itr.value();
    auto itr = map.find(key);
    if (itr != map.end()) {
      return itr.value();
    }
  }
  return nullptr;
}


ObjectSettingsTypeTable::ObjectSettingsTypeTable(const Db& _Db)
  : TableNamed(_Db)
{
}

