#include <Lib/Log/Log.h>

#include "DbIndex.h"


const int kWaitConnectMs = 3000;

bool DbIndexItem::Equals(const TableItem &) const
{
  return false;
}

bool DbIndex::Create()
{
  return true;
}

bool DbIndex::ConnectUnit(int _UnitId)
{
  mUnitId = _UnitId;
  return true;
}

bool DbIndex::Resize(int capacity)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT storage_resize(%1)").arg(capacity));
  return mDb.ExecuteNonQuery(q);
}

const char *DbIndex::Name()
{
  return "storage_cell";
}

const char *DbIndex::Select()
{
  return "SELECT _id, _unit, start_time, end_time, condition FROM storage_cell ";
}

bool DbIndex::OnRowFillItem(QueryS &q, TableItemS &unit)
{
  unit = TableItemS(new DbIndexItem());
  DbIndexItem* item = static_cast<DbIndexItem*>(unit.data());
  item->UnitId = q->value(1).toInt();
  item->StartTime = q->value(2).toDateTime();
  item->EndTime = q->value(3).toDateTime();
  item->Condition = q->value(4).toInt();
  return true;
}

bool DbIndex::FindCell(const QDateTime &timestamp, int &cellId, QDateTime& startTimestamp)
{
  auto q = mDb.MakeQuery();
  q->prepare("SELECT _id, start_time FROM storage_cell"
             " WHERE _unit = ? AND ? >= start_time AND (? < end_time OR end_time IS NULL)"
             " ORDER BY start_time DESC LIMIT 1");
  q->bindValue(0, mUnitId);
  q->bindValue(1, timestamp);
  q->bindValue(2, timestamp);
  if (mDb.ExecuteQuery(q) && q->next()) {
    cellId = q->value(0).toInt();
    startTimestamp = q->value(1).toDateTime();
    return true;
  }
  return false;
}

bool DbIndex::GetCurrentCell(int& cellId)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT get_current_cell(%1)").arg(mUnitId));
  if (mDb.ExecuteQuery(q) && q->next()) {
    cellId = q->value(0).toInt();
    return true;
  }
  return false;
}

bool DbIndex::GetNextCell(int& cellId)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT get_next_cell(%1)").arg(mUnitId));
  if (mDb.ExecuteQuery(q) && q->next()) {
    cellId = q->value(0).toInt();
    return true;
  }
  return false;
}

bool DbIndex::GetLastCell(int& lastCell)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT last_cell FROM storage_current_cell WHERE _unit = %1;").arg(mUnitId));
  if (mDb.ExecuteQuery(q) && q->next()) {
    lastCell = q->value(0).toInt();
    return true;
  }
  return false;
}

bool DbIndex::RepairCellInfo(int unitId, int currentCell, int lastCell)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("DELETE FROM storage_current_cell WHERE _unit = %1;").arg(unitId));
  if (!mDb.ExecuteNonQuery(q)) {
    return false;
  }
  q->prepare(QString("INSERT INTO storage_current_cell(_unit, current_cell, last_cell)"
                     " VALUES (%1, %2, %3);").arg(unitId).arg(currentCell).arg(lastCell));
  if (!mDb.ExecuteNonQuery(q)) {
    return false;
  }
  return true;
}

bool DbIndex::OpenCell(const int &cellId, const int &condition)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("UPDATE storage_cell SET _unit = %1, start_time = NULL"
                     ", end_time = NULL, condition = %2 WHERE _id = %3").arg(mUnitId).arg(condition).arg(cellId));
  return mDb.ExecuteNonQuery(q);
}

bool DbIndex::UpdateCell(const int &cellId, const QDateTime &startTime, const QDateTime &endTime, const int &condition)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("UPDATE storage_cell SET _unit = :unit, start_time = :startTime"
                     ", end_time = :endTime, condition = :condition WHERE _id = %1").arg(cellId));
  q->bindValue(":unit", mUnitId);
  q->bindValue(":startTime", startTime);
  q->bindValue(":endTime", endTime);
  q->bindValue(":condition", condition);
  return mDb.ExecuteNonQuery(q);
}

bool DbIndex::ResetCell(const int &cellId)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("UPDATE storage_cell SET _unit = 0, start_time = NULL"
                     ", end_time = NULL, condition = NULL WHERE _id = %1").arg(cellId));
  return mDb.ExecuteNonQuery(q);
}


DbIndex::DbIndex(Db& _Db)
  : Table(_Db)
  , mUnitId(0)
{
}

DbIndex::~DbIndex()
{
}


