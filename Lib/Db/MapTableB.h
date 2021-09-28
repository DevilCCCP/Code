#pragma once

#include <QList>

#include <Lib/Include/Common.h>

#include "Db.h"


DefineClassS(MapItemB);

class MapItemB
{
public:
  qint64 Id;
  qint64 Key;
  qint64 Value;
};

class MapTableB
{
protected:
  const Db& mDb;
private:
  QString mPreparedInsertSelect;
  QString mPreparedInsert;
  QString mPreparedInsertUniq;
  QString mPreparedRemove;
  QString mPreparedRemove2;
  QString mPreparedRemoveM;
  QString mPreparedRemoveS;
  QString mPreparedItemsMap;
  QString mPreparedParents;
  QString mPreparedChilds;
  QString mPreparedParentIds;
  QString mPreparedChildIds;
  QString mPreparedParentsCount;
  QString mPreparedChildsCount;
  QString mPreparedRemoveDuplicates1;

protected:
  /*new */virtual const char* GetTableName() = 0;
  /*new */virtual const char* GetColumnKeyName() = 0;
  /*new */virtual const char* GetColumnValueName() = 0;

public:
//  bool InsertUniqItem(const qint64& key, const qint64& value, qint64* id = nullptr);
  bool InsertItem(const qint64& key, const qint64& value, qint64* id = nullptr);
  bool InsertWithQuery(const QString& select);
  bool RemoveItem(const qint64& id);
  bool RemoveItem(const qint64& parentId, const qint64& childId);
  bool RemoveSlave(const qint64& childId);
  bool RemoveMaster(const qint64& parentId);
  bool GetParents(const qint64& id, QList<MapItemBS>& items);
  bool GetChilds(const qint64& id, QList<MapItemBS>& items);
  bool GetParentIds(const qint64& id, QList<qint64>& ids);
  bool GetChildIds(const qint64& id, QList<qint64>& ids);
  bool GetParentsCount(const qint64& id, qint64& count);
  bool GetChildsCount(const qint64& id, qint64& count);
  bool GetAllItems(const QString& condition, QMultiMap<qint64, qint64>& itemsMap);
  bool RemoveDuplicates(const qint64& keyId);
private:
  bool LoadItems(QueryS& q, QList<MapItemBS>& items);
  bool LoadIds(QueryS& q, QList<qint64>& ids);

private:
  void Prepare();

public:
  MapTableB(const Db& _Db);
  /*new */virtual ~MapTableB();
};
