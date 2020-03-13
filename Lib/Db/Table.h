#pragma once

#include <QSharedPointer>
#include <QMap>
#include <QString>

#include <Lib/Include/Common.h>

#include "Db.h"
#include "TableItem.h"


class Table
{
protected:
  const Db&  mDb;

  QMap<int, TableItemS> mItems;
  bool                  mLoaded;

protected:
  /*new */virtual const char* Name() = 0;
  /*new */virtual const char* Select();
  /*new */virtual const char* Insert();
  /*new */virtual const char* Update();
  /*new */virtual const char* Delete();
  /*new */virtual bool OnRowLoad(QueryS& q, TableItemS& unit);
  /*new */virtual bool OnRowFillItem(QueryS& q, TableItemS& unit);
  /*new */virtual bool OnSetItem(QueryS& q, const TableItem& unit);
  /*new */virtual void CreateIndexes();
  /*new */virtual void ClearIndexes();

protected:
  void SelectOne(int id, QString& queryText);

public:
  bool Open() { if (mLoaded) return true; return Reload(); }
  bool Load();
  bool Reload();
  void Clear();
  bool LoadWhere(const QString& queryWhere);
  bool LoadCount(qint64& count);

  const QMap<int, TableItemS>& GetItems() { if (!mLoaded) Reload(); return mItems; }
  TableItemS GetItem(int id, bool useCache = true);
  TableItemS GetItem(const QString& queryWhere);

  template<typename ItemT>
  bool InsertItem(const QSharedPointer<ItemT>& item) { return InsertItem(qSharedPointerCast<TableItem, ItemT>(item)); }
  template<typename ItemT>
  bool UpdateItem(const QSharedPointer<ItemT>& item) { return UpdateItem(*item); }
  template<typename ItemT>
  bool RemoveItem(const QSharedPointer<ItemT>& item) { return RemoveItem(item->Id); }

  bool InsertItem(const TableItemS& item);
  bool UpdateItem(const TableItem& item);
  bool UpdateItemId(int id, int newId);
  bool RemoveItem(int id);

  Table(const Db& _Db);
  /*new */virtual ~Table();
};
