#pragma once

#include <QDateTime>

#include <Lib/Db/Table.h>
#include "Cell.h"


DefineDbClassS(DbIndex);

class DbIndexItem: public TableItem
{
public:
  int       UnitId;
  QDateTime StartTime;
  QDateTime EndTime;
  int       Condition;

public:
  /*override */virtual bool Equals(const TableItem&) const Q_DECL_OVERRIDE;

  DbIndexItem() { }
  /*override */virtual ~DbIndexItem() { }
};

class DbIndex: public Table
{
  int     mUnitId;

public:
  bool Create();
  bool ConnectUnit(int _UnitId);
  bool Resize(int capacity);

protected:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE;
  /*override */virtual const char* Select() Q_DECL_OVERRIDE;
  /*override */virtual bool OnRowFillItem(QueryS& q, TableItemS& unit) Q_DECL_OVERRIDE;

public:
  bool FindCell(const QDateTime& timestamp, int& cellId, QDateTime& startTimestamp);
  bool GetCurrentCell(int &cellId);
  bool GetNextCell(int& cellId);
  bool GetLastCell(int& lastCell);
  bool RepairCellInfo(int unitId, int currentCell, int lastCell);

  bool OpenCell(const int &cellId, const int& condition);
  bool UpdateCell(const int &cellId, const QDateTime& startTime, const QDateTime& endTime, const int& condition);
  bool ResetCell(const int &cellId);

public:
  DbIndex(Db &_Db);
  /*override */virtual ~DbIndex();
};

