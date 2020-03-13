#pragma once

#include <Lib/Include/Common.h>

#include "Db.h"


class MapItem
{
  int Id;
  int Key;
  int Value;
};

class MapTable
{
protected:
  const Db& mDb;

private:
  QString mPreparedSelectSlaves;
  QString mPreparedInsert;
  QString mPreparedBatchInsert;
  QString mPreparedRemove;
  QString mPreparedRemove2;
  QString mPreparedRemoveSlaves;
  QString mPreparedRemoveMasters;

protected:
  /*new */virtual const char* GetTableName() = 0;
  /*new */virtual const char* GetColumnKeyName() = 0;
  /*new */virtual const char* GetColumnValueName() = 0;

public:
  bool Select(const QString& where, QMap<int, int>& map);
  bool SelectSlaves(int masterId, QList<int>& slaveIds);
  bool InsertItem(int key, int value, int* id = nullptr);
  bool InsertItems(QList<int> keys, QList<int> values);
  bool InsertItems(int key, QList<int> values);
  bool RemoveItem(int id);
  bool RemoveItem(int key, int value);
  bool RemoveSlaves(int masterId);
  bool RemoveMasters(int slaveId);

private:
  void Prepare();

public:
  MapTable(const Db& _Db);
  /*new */virtual ~MapTable();
};
