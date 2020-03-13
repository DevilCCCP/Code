#include "MapTable.h"


bool MapTable::Select(const QString& where, QMap<int, int>& map)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT %2, %3 FROM %1 %4").arg(GetTableName()).arg(GetColumnKeyName()).arg(GetColumnValueName()).arg(where));

  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    int idKey   = q->value(0).toInt();
    int idValue = q->value(1).toInt();
    map.insert(idKey, idValue);
  }
  return true;
}

bool MapTable::SelectSlaves(int masterId, QList<int>& slaveIds)
{
  if (mPreparedSelectSlaves.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedSelectSlaves);
  q->bindValue(0, masterId);

  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    int id = q->value(0).toInt();
    slaveIds.append(id);
  }
  return true;
}

bool MapTable::InsertItem(int key, int value, int* id)
{
  if (mPreparedInsert.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedInsert);
  q->bindValue(0, key);
  q->bindValue(1, value);

  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return false;
  }
  if (id) {
    *id = q->value(0).toInt();
  }
  return true;
}

bool MapTable::InsertItems(QList<int> keys, QList<int> values)
{
  if (mPreparedBatchInsert.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedBatchInsert);

  QVariantList vlKeys;
  QVariantList vlValues;

  foreach (const int& key, keys)
    vlKeys << key;
  foreach (const int& value, values)
    vlValues << value;

  q->addBindValue(vlKeys);
  q->addBindValue(vlValues);

  if (!mDb.ExecuteBatch(q)) {
    return false;
  }

  return true;
}

bool MapTable::InsertItems(int key, QList<int> values)
{
  QList<int> keys;
  keys.reserve(values.size());
  for (int i=0; i<values.size(); ++i)
    keys.append(key);

  return InsertItems(keys, values);
}

bool MapTable::RemoveItem(int id)
{
  if (mPreparedRemove.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedRemove);
  q->bindValue(0, id);

  return mDb.ExecuteNonQuery(q);
}

bool MapTable::RemoveItem(int key, int value)
{
  if (mPreparedRemove2.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedRemove2);
  q->bindValue(0, key);
  q->bindValue(1, value);

  return mDb.ExecuteNonQuery(q);
}

bool MapTable::RemoveSlaves(int masterId)
{
  if (mPreparedRemove.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedRemoveSlaves);
  q->bindValue(0, masterId);

  return mDb.ExecuteNonQuery(q);
}

bool MapTable::RemoveMasters(int slaveId)
{
  if (mPreparedRemove.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedRemoveMasters);
  q->bindValue(0, slaveId);

  return mDb.ExecuteNonQuery(q);
}

void MapTable::Prepare()
{
  mPreparedSelectSlaves = QString("SELECT %2 FROM %1 WHERE %3=?").arg(GetTableName()).arg(GetColumnValueName()).arg(GetColumnKeyName());
  mPreparedInsert = QString("INSERT INTO %1(%2, %3) VALUES (?, ?) RETURNING _id")
      .arg(GetTableName()).arg(GetColumnKeyName()).arg(GetColumnValueName());
  mPreparedBatchInsert = QString("INSERT INTO %1(%2, %3) VALUES (?, ?)")
      .arg(GetTableName()).arg(GetColumnKeyName()).arg(GetColumnValueName());
  mPreparedRemove = QString("DELETE FROM %1 WHERE _id=?").arg(GetTableName());
  mPreparedRemove2 = QString("DELETE FROM %1 WHERE %2=? AND %3=?")
      .arg(GetTableName()).arg(GetColumnKeyName()).arg(GetColumnValueName());
  mPreparedRemoveSlaves = QString("DELETE FROM %1 WHERE %2=?").arg(GetTableName()).arg(GetColumnKeyName());
  mPreparedRemoveMasters = QString("DELETE FROM %1 WHERE %2=?").arg(GetTableName()).arg(GetColumnValueName());
}

MapTable::MapTable(const Db& _Db)
  : mDb(_Db)
{
}

MapTable::~MapTable()
{
}
