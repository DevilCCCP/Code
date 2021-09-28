#include <QString>

#include <Lib/Log/Log.h>

#include "TableB.h"


bool TableB::InsertCopy(const qint64& id, qint64* newId)
{
  auto q = mDb.MakeQuery();
  if (true) { //if (mPreparedClone.isEmpty()) {
    ParseColumns();
    QString copyColumns = mPreparedColumns;
    if (copyColumns.startsWith("name")) {
      if (mCloneId == id && mCloneCounter > 0) {
        copyColumns.insert(4, QString(" || ' copy %1'").arg(mCloneCounter));
      } else {
        copyColumns.insert(4, " || ' copy'");
      }
      if (mCloneId == id) {
        mCloneCounter++;
      } else {
        mCloneId = id;
        mCloneCounter = 1;
      }
    }
    mPreparedClone = QString("INSERT INTO %1 (%2) (SELECT %3 FROM %1 WHERE _id = ?) RETURNING _id;")
        .arg(GetTableName(), mPreparedColumns, copyColumns);
  }
  q->prepare(mPreparedClone);
  q->addBindValue(id);
  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return false;
  }
  if (newId) {
    *newId = q->value(0).toLongLong();
  }
  return true;
}

bool TableB::CreateItem(const TableItemBS& item)
{
  auto q = PrepareInsertWithId();
  int index = 0;
  q->bindValue(index++, item->Id);
  OnApplyItem(q, item, index);

  return mDb.ExecuteNonQuery(q);
}

bool TableB::SelectItem(const qint64& id, TableItemBS& item)
{
  auto q = PrepareSelectWithId();
  int index = 0;
  q->bindValue(index++, id);
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }
  if (q->next()) {
    int index = 0;
    OnRowFillItem(q, item, index);
    item->Id = id;
  }

  return true;
}

bool TableB::InsertItem(const TableItemBS& item)
{
  auto q = PrepareInsert();
  int index = 0;
  OnApplyItem(q, item, index);

  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return false;
  }

  item->Id = q->value(0).toLongLong();
  return true;
}

bool TableB::InsertItems(const QStringList& rows)
{
  auto q = mDb.MakeQuery();

  ParseColumns();
  if (mPreparedInsertBatch.isEmpty()) {
    mPreparedInsertBatch = QString("INSERT INTO %1 (%2) VALUES ")
        .arg(GetTableName()).arg(mPreparedColumns);
  }
  q->prepare(mPreparedInsertBatch + rows.join(','));

  if (!mDb.ExecuteNonQuery(q)) {
    return false;
  }
  return true;
}

bool TableB::UpdateItem(const TableItemBS& item)
{
  auto q = PrepareUpdate("WHERE _id = " + QString::number(item->Id));
  int index = 0;
  OnApplyItem(q, item, index);

  return mDb.ExecuteNonQuery(q);
}

bool TableB::RemoveItem(const TableItemBS& item)
{
  return RemoveItem(item->Id);
}

bool TableB::RemoveItem(const qint64& id)
{
  auto q = PrepareRemove("WHERE _id = " + QString::number(id));
  return mDb.ExecuteNonQuery(q);
}

QString TableB::GetColumns(const QString& table)
{
  QStringList columns = QString(GetColumnNames()).split(QChar(','), Qt::SkipEmptyParts);
  columns.prepend("_id");
  for (auto itr = columns.begin(); itr != columns.end(); itr++) {
    QString& column = *itr;
    *itr = table + "." + column.trimmed();
  }
  return columns.join(", ");
}

qint64 TableB::GetTopId()
{
  auto q = PrepareSelectTopId();
  return mDb.ExecuteScalar(q).toLongLong();
}

TableItemBS TableB::GetItem(const qint64& id, bool* ok)
{
  return GetItem(QString("WHERE _id = ") + QString::number(id), ok);
}

TableItemBS TableB::GetItem(const QString& conditions, bool* ok)
{
  TableItemBS item;

  auto q = PrepareSelect(conditions);

  bool ok_ = mDb.ExecuteQuery(q);
  if (ok) {
    *ok = ok_;
  }
  if (!ok_ || !q->next()) {
    return item;
  }

  int index = 1;
  OnRowFillItem(q, item, index);
  item->Id = q->value(0).toLongLong();
  while (q->next()) {
    Log.Warning(QString("Select more than one row (cond: '%1')").arg(conditions));
  }
  return item;
}

bool TableB::GetItems(const QString& conditions, QList<TableItemBS>& items)
{
  items.clear();

  auto q = PrepareSelect(conditions);
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    TableItemBS item;

    int index = 1;
    OnRowFillItem(q, item, index);
    item->Id = q->value(0).toLongLong();
    items.append(item);
  }
  return true;
}

bool TableB::GetItems(QList<TableItemBS>& items)
{
  return GetItems(QString(), items);
}

bool TableB::DeleteItems(const QString& conditions)
{
  auto q = PrepareRemove(conditions);
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  return true;
}

qint64 TableB::GetTotalCount(bool* ok)
{
  QString conditions;
  return GetCount(conditions, ok);
}

qint64 TableB::GetCount(const QString& conditions, bool* ok)
{
  auto q = PrepareSelectCount(conditions);
  if (mDb.ExecuteQuery(q) && q->next()) {
    return q->value(0).toInt(ok);
  }
  if (ok) {
    *ok = false;
  }
  return 0;
}

QString TableB::GetSelectColumns(const QString& prefix)
{
  QString result = QString(" %1._id").arg(prefix);
  QStringList columns = QString(GetColumnNames()).split(QChar(','), Qt::SkipEmptyParts);
  for (auto itr = columns.begin(); itr != columns.end(); itr++) {
    result.append(QString(", %1.%2").arg(prefix).arg(*itr));
  }
  return result;
}

QSqlQueryS TableB::PrepareSelectWithId()
{
  auto q = mDb.MakeQuery();
  if (mPreparedSelectWithId.isEmpty()) {
    ParseColumns();
    mPreparedSelectWithId = QString("SELECT %2 FROM %1 WHERE _id=?;")
        .arg(GetTableName()).arg(mPreparedColumns);
  }
  q->prepare(mPreparedSelectWithId);
  return q;
}

QSqlQueryS TableB::PrepareInsertWithId()
{
  auto q = mDb.MakeQuery();
  if (mPreparedInsertWithId.isEmpty()) {
    ParseColumns();
    mPreparedInsertWithId = QString("INSERT INTO %1 (_id, %2) VALUES (?, %3) RETURNING _id;")
        .arg(GetTableName()).arg(mPreparedColumns).arg(mPreparedValues);
  }
  q->prepare(mPreparedInsertWithId);
  return q;
}

QSqlQueryS TableB::PrepareInsert()
{
  auto q = mDb.MakeQuery();
  if (mPreparedInsert.isEmpty()) {
    ParseColumns();
    mPreparedInsert = QString("INSERT INTO %1 (%2) VALUES (%3) RETURNING _id;")
        .arg(GetTableName()).arg(mPreparedColumns).arg(mPreparedValues);
  }
  q->prepare(mPreparedInsert);
  return q;
}

QSqlQueryS TableB::PrepareInsertBatch(const QStringList& rows)
{
  auto q = mDb.MakeQuery();
  if (mPreparedInsertBatch.isEmpty()) {
    ParseColumns();
    mPreparedInsertBatch = QString("INSERT INTO %1 (%2)\n VALUES ")
        .arg(GetTableName()).arg(mPreparedColumns);
  }
  q->prepare(mPreparedInsertBatch + rows.join(",\n"));
  return q;
}

QSqlQueryS TableB::PrepareUpdate(const QString& conditions)
{
  auto q = mDb.MakeQuery();
  if (mPreparedUpdate.isEmpty()) {
    ParseColumns();
    mPreparedUpdate = QString("UPDATE %1 SET %2 ")
        .arg(GetTableName()).arg(mPreparedSetColValues);
  }
  q->prepare(mPreparedUpdate + conditions);
  return q;
}

QSqlQueryS TableB::PrepareRemove(const QString& conditions)
{
  auto q = mDb.MakeQuery();
  if (mPreparedRemove.isEmpty()) {
    ParseColumns();
    mPreparedRemove = QString("DELETE FROM %1 ").arg(GetTableName());
  }
  q->prepare(mPreparedRemove + conditions);
  return q;
}

QSqlQueryS TableB::PrepareSelect(const QString& conditions)
{
  auto q = mDb.MakeQuery();
  if (mPreparedSelect.isEmpty()) {
    ParseColumns();
    mPreparedSelect = QString("SELECT _id, %2 FROM %1 ")
        .arg(GetTableName()).arg(mPreparedColumns);
  }
  q->prepare(mPreparedSelect + conditions);
  return q;
}

QSqlQueryS TableB::PrepareSelectCount(const QString& conditions)
{
  auto q = mDb.MakeQuery();
  if (mPreparedSelectCount.isEmpty()) {
    ParseColumns();
    mPreparedSelectCount = QString("SELECT COUNT(_id) FROM %1 ").arg(GetTableName());
  }
  q->prepare(mPreparedSelectCount + conditions);
  return q;
}

QSqlQueryS TableB::PrepareSelectTopId()
{
  auto q = mDb.MakeQuery();
  if (mPreparedTopId.isEmpty()) {
    mPreparedTopId = QString("SELECT max(_id) FROM %1 ").arg(GetTableName());
  }
  q->prepare(mPreparedTopId);
  return q;
}

void TableB::ParseColumns()
{
  if (!mPreparedColumns.isEmpty()) {
    return;
  }

  mPreparedColumns = QString(GetColumnNames()).trimmed();
  mPreparedValues = QString("?");
  mPreparedSetColValues = QString();
  for (auto itr = mPreparedColumns.begin(); itr != mPreparedColumns.end(); itr++) {
    const QChar& ch = *itr;
    if (ch == QChar(',')) {
      mPreparedValues.append(",?");
      mPreparedSetColValues.append("=?,");
    } else {
      mPreparedSetColValues.append(ch);
    }
  }
  mPreparedSetColValues.append("=?");
}


TableB::TableB(const Db& _Db)
  : mDb(_Db)
  , mCloneId(0), mCloneCounter(0)
{
}

TableB::~TableB()
{
}
