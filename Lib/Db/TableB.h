#pragma once

#include <QString>

#include <Lib/Include/Common.h>

#include "Db.h"
#include "TableItem.h"


DefineClassS(TableB);
DefineClassS(QSqlQuery);

class TableB
{
protected:
  const Db& mDb;
private:
  QString mPreparedColumns;
  QString mPreparedValues;
  QString mPreparedSetColValues;
  QString mPreparedSelect;
  QString mPreparedSelectCount;
  QString mPreparedSelectWithId;
  QString mPreparedInsertWithId;
  QString mPreparedInsert;
  QString mPreparedInsertBatch;
  QString mPreparedUpdate;
  QString mPreparedRemove;
  QString mPreparedClone;
  QString mPreparedTopId;

  qint64  mCloneId;
  int     mCloneCounter;

protected:
  const QString& PreparedColumns() { return mPreparedColumns; }
  const QString& PreparedValues()  { return mPreparedValues;  }

protected:
  /*new */virtual const char* GetTableName() = 0;
  /*new */virtual const char* GetColumnNames() = 0;
  /*new */virtual void OnApplyItem(QueryS& q, const TableItemBS& item, int& index) = 0;
  /*new */virtual void OnRowFillItem(QueryS& q, TableItemBS& item, int& index) = 0;

public:
  /*new */virtual bool InsertCopy(const qint64& id, qint64* newId = nullptr);

public:
  bool CreateItem(const TableItemBS& item);
  bool SelectItem(const qint64& id, TableItemBS& item);
  bool InsertItem(const TableItemBS& item);
  bool InsertItems(const QStringList& rows);
  bool UpdateItem(const TableItemBS& item);
  bool RemoveItem(const TableItemBS& item);
  bool RemoveItem(const qint64& id);
  QString GetColumns(const QString& table);
  qint64 GetTopId();
  TableItemBS GetItem(const qint64& id, bool* ok = nullptr);
  TableItemBS GetItem(const QString& conditions, bool* ok = nullptr);
  bool GetItems(const QString& conditions, QList<TableItemBS>& items);
  bool GetItems(QList<TableItemBS>& items);
  bool DeleteItems(const QString& conditions);
  qint64 GetTotalCount(bool* ok = nullptr);
  qint64 GetCount(const QString& conditions, bool* ok = nullptr);

  template<typename TableItemT>
  bool SelectItem(const qint64& id, QSharedPointer<TableItemT>& item)
  {
    TableItemBS item_;
    if (SelectItem(id, item_)) {
      item = item_.template staticCast<TableItemT>();
      return true;
    } else {
      item.reset();
      return false;
    }
  }
  template<typename TableItemT>
  bool UpdateItem(const QSharedPointer<TableItemT>& item) { return UpdateItem(item.template staticCast<TableItemB>()); }
  template<typename TableItemT>
  bool InsertItem(QSharedPointer<TableItemT>& item) { return InsertItem(item.template staticCast<TableItemB>()); }

protected:
  QString GetSelectColumns(const QString& prefix);
  QSqlQueryS PrepareSelectWithId();
  QSqlQueryS PrepareInsertWithId();
  QSqlQueryS PrepareInsert();
  QSqlQueryS PrepareInsertBatch(const QStringList& rows);
  QSqlQueryS PrepareUpdate(const QString& conditions);
  QSqlQueryS PrepareRemove(const QString& conditions);
  QSqlQueryS PrepareSelect(const QString& conditions);
  QSqlQueryS PrepareSelectCount(const QString& conditions);
  QSqlQueryS PrepareSelectTopId();
  void ParseColumns();

public:
  TableB(const Db& _Db);
  /*new */virtual ~TableB();
};
