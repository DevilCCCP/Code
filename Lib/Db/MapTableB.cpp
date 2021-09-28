#include "MapTableB.h"


//bool MapTableB::InsertUniqItem(const qint64& key, const qint64& value, qint64* id)
//{
//  if (mPreparedInsertUniq.isEmpty()) {
//    Prepare();
//  }
//  auto q = mDb.MakeQuery();
//  q->prepare(mPreparedInsertUniq);
//  q->bindValue(0, key);
//  q->bindValue(1, value);
//  q->bindValue(2, key);
//  q->bindValue(3, value);

//  if (!mDb.ExecuteQuery(q) || !q->next()) {
//    return false;
//  }
//  if (id) {
//    *id = q->value(0).toLongLong();
//  }
//  return true;
//}

bool MapTableB::InsertItem(const qint64& key, const qint64& value, qint64* id)
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
    *id = q->value(0).toLongLong();
  }
  return true;
}

bool MapTableB::InsertWithQuery(const QString& select)
{
  if (mPreparedInsertSelect.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedInsertSelect + "(" + select + ");");

  return mDb.ExecuteNonQuery(q);
}

bool MapTableB::RemoveItem(const qint64& id)
{
  if (mPreparedRemove.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedRemove);
  q->bindValue(0, id);

  return mDb.ExecuteNonQuery(q);
}

bool MapTableB::RemoveItem(const qint64& parentId, const qint64& childId)
{
  if (mPreparedRemove2.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedRemove2);
  q->bindValue(0, parentId);
  q->bindValue(1, childId);

  return mDb.ExecuteNonQuery(q);
}

bool MapTableB::RemoveSlave(const qint64& childId)
{
  if (mPreparedRemoveS.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedRemoveS);
  q->bindValue(0, childId);

  return mDb.ExecuteNonQuery(q);
}

bool MapTableB::RemoveMaster(const qint64& parentId)
{
  if (mPreparedRemoveM.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedRemoveM);
  q->bindValue(0, parentId);

  return mDb.ExecuteNonQuery(q);
}

bool MapTableB::GetParents(const qint64& id, QList<MapItemBS>& items)
{
  if (mPreparedParents.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedParents);
  q->bindValue(0, id);

  return LoadItems(q, items);
}

bool MapTableB::GetChilds(const qint64& id, QList<MapItemBS>& items)
{
  if (mPreparedChilds.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedChilds);
  q->bindValue(0, id);

  return LoadItems(q, items);
}

bool MapTableB::GetParentIds(const qint64& id, QList<qint64>& ids)
{
  if (mPreparedParentIds.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedParentIds);
  q->bindValue(0, id);

  return LoadIds(q, ids);
}

bool MapTableB::GetChildIds(const qint64& id, QList<qint64>& ids)
{
  if (mPreparedChildIds.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedChildIds);
  q->bindValue(0, id);

  return LoadIds(q, ids);
}

bool MapTableB::GetParentsCount(const qint64& id, qint64& count)
{
  if (mPreparedParentsCount.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedParentsCount);
  q->bindValue(0, id);

  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return false;
  }
  count = q->value(0).toLongLong();
  return true;
}

bool MapTableB::GetChildsCount(const qint64& id, qint64& count)
{
  if (mPreparedChildsCount.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedChildsCount);
  q->bindValue(0, id);

  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return false;
  }
  count = q->value(0).toLongLong();
  return true;
}

bool MapTableB::GetAllItems(const QString& condition, QMultiMap<qint64, qint64>& itemsMap)
{
  if (mPreparedItemsMap.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedItemsMap + " " + condition);

  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  itemsMap.clear();
  while (q->next()) {
    qint64 key = q->value(0).toLongLong();
    qint64 value = q->value(1).toLongLong();
    itemsMap.insert(key, value);
  }
  return true;
}

bool MapTableB::RemoveDuplicates(const qint64& keyId)
{
  if (mPreparedRemoveDuplicates1.isEmpty()) {
    Prepare();
  }
  auto q = mDb.MakeQuery();
  q->prepare(mPreparedRemoveDuplicates1);
  q->bindValue(0, keyId);

  return mDb.ExecuteNonQuery(q);
}

bool MapTableB::LoadItems(QueryS& q, QList<MapItemBS>& items)
{
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  items.clear();
  while (q->next()) {
    MapItemBS item(new MapItemB());
    item->Id = q->value(0).toLongLong();
    item->Key = q->value(1).toLongLong();
    item->Value = q->value(2).toLongLong();
    items.append(item);
  }
  return true;
}

bool MapTableB::LoadIds(QueryS& q, QList<qint64>& ids)
{
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  ids.clear();
  while (q->next()) {
    qint64 id = q->value(0).toLongLong();
    ids.append(id);
  }
  return true;
}

void MapTableB::Prepare()
{
  mPreparedInsertSelect = QString("INSERT INTO %1(%2, %3) ")
      .arg(GetTableName(), GetColumnKeyName(), GetColumnValueName());
  mPreparedInsert = QString("INSERT INTO %1(%2, %3) VALUES (?, ?) RETURNING _id")
      .arg(GetTableName(), GetColumnKeyName(), GetColumnValueName());
  mPreparedRemove = QString("DELETE FROM %1 WHERE _id=?")
      .arg(GetTableName());
  mPreparedRemove2 = QString("DELETE FROM %1 WHERE %2=? AND %3=?")
      .arg(GetTableName(), GetColumnKeyName(), GetColumnValueName());
  mPreparedRemoveM = QString("DELETE FROM %1 WHERE %2=?")
      .arg(GetTableName(), GetColumnKeyName());
  mPreparedRemoveS = QString("DELETE FROM %1 WHERE %2=?")
      .arg(GetTableName(), GetColumnValueName());
  mPreparedItemsMap = QString("SELECT %2, %3 FROM %1")
      .arg(GetTableName(), GetColumnKeyName(), GetColumnValueName());
  mPreparedParents = QString("SELECT _id, %2, %3 FROM %1 WHERE %3=?")
      .arg(GetTableName(), GetColumnKeyName(), GetColumnValueName());
  mPreparedChilds = QString("SELECT _id, %2, %3 FROM %1 WHERE %2=?")
      .arg(GetTableName(), GetColumnKeyName(), GetColumnValueName());
  mPreparedParentIds = QString("SELECT %2 FROM %1 WHERE %3=?")
      .arg(GetTableName(), GetColumnKeyName(), GetColumnValueName());
  mPreparedChildIds = QString("SELECT %3 FROM %1 WHERE %2=?")
      .arg(GetTableName(), GetColumnKeyName(), GetColumnValueName());
  mPreparedParentsCount = QString("SELECT COUNT(_id) FROM %1 WHERE %2=?")
      .arg(GetTableName(), GetColumnValueName());
  mPreparedChildsCount = QString("SELECT COUNT(_id) FROM %1 WHERE %2=?")
      .arg(GetTableName(), GetColumnKeyName());
  mPreparedInsertUniq = QString("DELETE FROM %1 WHERE %2=? AND %3=?;"
                                " INSERT INTO %1(%2, %3) VALUES (?, ?)")
      .arg(GetTableName(), GetColumnKeyName(), GetColumnValueName());
  mPreparedRemoveDuplicates1 = QString("DELETE FROM %1 WHERE _id IN (SELECT _id"
                                       " FROM (SELECT _id,"
                                       " ROW_NUMBER() OVER (partition BY %2, %3 ORDER BY _id) AS rnum"
                                       " FROM %1 WHERE %2 = ?) t WHERE t.rnum > 1);")
      .arg(GetTableName(), GetColumnKeyName(), GetColumnValueName());
}

MapTableB::MapTableB(const Db& _Db)
  : mDb(_Db)
{
}

MapTableB::~MapTableB()
{
}
