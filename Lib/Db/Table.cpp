#include <typeinfo>
#include <QSet>

#include <Lib/Log/Log.h>

#include "Table.h"


const char* Table::Select()
{
  Log.Warning(QString("Db class %1 is not selectable").arg(typeid(*this).name()));
  return nullptr;
}

const char* Table::Insert()
{
  Log.Warning(QString("Db class %1 is not insertable").arg(typeid(*this).name()));
  return nullptr;
}

const char* Table::Update()
{
  Log.Warning(QString("Db class %1 is not updatable").arg(typeid(*this).name()));
  return nullptr;
}

const char* Table::Delete()
{
  Log.Warning(QString("Db class %1 is not removable").arg(typeid(*this).name()));
  return nullptr;
}

bool Table::OnRowLoad(QueryS &q, TableItemS &unit)
{
  int id = q->value(0).toInt();
  if (id == 0) {
    return false;
  }
  if (OnRowFillItem(q, unit)) {
    unit->Id = id;
    return true;
  } else {
    return false;
  }
}

bool Table::OnRowFillItem(QueryS&, TableItemS&)
{
  Log.Warning("DbTable OnRowFillItem illegal call");
  return false;
}

bool Table::OnSetItem(QueryS&, const TableItem&)
{
  Log.Warning("DbTable OnSetItem illegal call");
  return false;
}

void Table::CreateIndexes()
{
}

void Table::ClearIndexes()
{
}

void Table::SelectOne(int id, QString &queryText)
{
  queryText = QString("%1 WHERE _id = %2").arg(Select()).arg(id);
}

bool Table::Load()
{
  if (!mLoaded) {
    Reload();
  }
  return mLoaded;
}

bool Table::Reload()
{
  bool reload = false;
  QSet<int> oldIds;
  for (auto itr = mItems.begin(); itr != mItems.end(); itr++) {
    oldIds.insert(itr.key());
  }

  auto q = mDb.MakeQuery();
  q->prepare(Select());
  if (mDb.ExecuteQuery(q)) {
    while (q->next()) {
      TableItemS unit;
      if (OnRowLoad(q, unit)) {
        int id = unit->Id;
        auto itr = mItems.find(id);
        if (itr != mItems.end()) {
          if (!itr.value()->Equals(*unit)) {
            itr.value() = unit;
            reload = true;
          }
        } else {
          mItems[id] = unit;
          reload = true;
        }
        oldIds.remove(id);
      }
    }

    for (auto itr = oldIds.begin(); itr != oldIds.end(); itr++) {
      mItems.remove(*itr);
      reload = true;
    }
    mLoaded = true;
  } else {
    mLoaded = false;
  }

  if (reload) {
    ClearIndexes();
    CreateIndexes();
  }
  return reload;
}

void Table::Clear()
{
  ClearIndexes();
  mItems.clear();
  mLoaded = false;
}

bool Table::LoadWhere(const QString& queryWhere)
{
  Clear();

  auto q = mDb.MakeQuery();
  q->prepare(QString(Select()) + " " + queryWhere);
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    TableItemS unit;
    if (OnRowLoad(q, unit)) {
      int id = unit->Id;
      mItems[id] = unit;
    }
  }
  CreateIndexes();
  return true;
}

bool Table::LoadCount(qint64& count)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT COUNT(_id) FROM %1").arg(Name()));
  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return false;
  }
  count = q->value(0).toLongLong();
  return true;
}

TableItemS Table::GetItem(int id, bool useCache)
{
  if (useCache) {
    auto itr = mItems.find(id);
    if (itr != mItems.end()) {
      return itr.value();
    }
  }

  QString queryText;
  SelectOne(id, queryText);

  auto q = mDb.MakeQuery();
  q->prepare(queryText);
  if (mDb.ExecuteQuery(q))
  {
    while (q->next()) {
      TableItemS unit;
      if (OnRowLoad(q, unit)) {
        mItems[id] = unit;
        return unit;
      }
    }
  }
  return TableItemS();
}

TableItemS Table::GetItem(const QString &queryWhere)
{
  QString queryText = QString("%1 %2").arg(Select()).arg(queryWhere);

  auto q = mDb.MakeQuery();
  q->prepare(queryText);
  if (mDb.ExecuteQuery(q))
  {
    while (q->next()) {
      TableItemS unit;
      if (OnRowLoad(q, unit)) {
        mItems[unit->Id] = unit;
        return unit;
      }
    }
  }
  return TableItemS();
}

bool Table::InsertItem(const TableItemS& item)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString(Insert()) + " RETURNING _id");
  if (!OnSetItem(q, *item)) {
    return false;
  }
  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return false;
  }

  item->Id = q->value(0).toInt();
  return true;
}

bool Table::UpdateItem(const TableItem& item)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString(Update()) + " WHERE _id=" + QString::number(item.Id));
  if (!OnSetItem(q, item)) {
    return false;
  }
  return mDb.ExecuteQuery(q);
}

bool Table::UpdateItemId(int id, int newId)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("UPDATE %1 SET _id=%2 WHERE _id=%3;").arg(Name()).arg(newId).arg(id));
  if (!mDb.ExecuteNonQuery(q)) {
    return false;
  }

  mItems[newId] = mItems[id];
  mItems.remove(id);
  return true;
}

bool Table::RemoveItem(int id)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString(Delete()) + " WHERE _id=" + QString::number(id));
  bool reload = mDb.ExecuteNonQuery(q);
  if (reload) {
    if (mItems.remove(id) > 0) {
      ClearIndexes();
      CreateIndexes();
    }
  }
  return reload;
}

Table::Table(const Db& _Db)
  : mDb(_Db), mLoaded(false)
{
}

Table::~Table()
{
}


