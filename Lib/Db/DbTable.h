#pragma once

#include <QString>
#include <QVariant>
#include <QVector>
#include <QList>
#include <qsystemdetection.h>

#include <Lib/Include/Common.h>
#include <Lib/Log/Log.h>

#include "Db.h"
#include "DbTableA.h"


DefineClassS(QSqlQuery);

template<typename IntT, typename DbItemImpT>
class DbTableT: public DbTableA<IntT>
{
  QMap<IntT, QSharedPointer<DbItemT<IntT> > > mItems;
  bool                      mLoaded;

public:
  DbTableT(const Db& _Db)
    : DbTableA<IntT>(_Db)
    , mLoaded(false)
  { }

public:
  bool IsLoaded() { return mLoaded; }
  const QMap<IntT, QSharedPointer<DbItemT<IntT> > >& Items() const { return mItems; }

  bool GetItems(QList<QSharedPointer<DbItemImpT> >& itemList) const
  {
    itemList.clear();
    for (auto itr = mItems.begin(); itr != mItems.end(); itr++) {
      itemList.append(qSharedPointerCast<DbItemImpT, DbItemT<IntT> >(itr.value()));
    }
    return true;
  }

  bool GetItems(QVector<QSharedPointer<DbItemImpT> >& itemList) const
  {
    itemList.clear();
    for (auto itr = mItems.begin(); itr != mItems.end(); itr++) {
      itemList.append(qSharedPointerCast<DbItemImpT, DbItemT<IntT> >(itr.value()));
    }
    return true;
  }

  bool ItemById(const IntT& id, QSharedPointer<DbItemImpT>& item)
  {
    auto itr = mItems.find(id);
    if (itr != mItems.end()) {
      item = qSharedPointerCast<DbItemImpT, DbItemT<IntT> >(itr.value());
      return true;
    }

    return SelectById(id, item);
  }

  void AddToCache(const QSharedPointer<DbItemImpT>& item)
  {
    mItems[item->Id] = item;
  }

  void RemoveFromCache(const QSharedPointer<DbItemImpT>& item)
  {
    mItems.remove(item->Id);
  }

  bool Load()
  {
    if (IsLoaded()) {
      return true;
    } else {
      return Reload();
    }
  }

  bool Reload()
  {
    bool reload = !mLoaded;
    QList<IntT> oldIdList = mItems.keys();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14,0)
    QSet<IntT> oldIds(oldIdList.begin(), oldIdList.end());
#else
    QSet<IntT> oldIds = QSet<IntT>::fromList(oldIdList);
#endif

    this->PrepareStrings();
    const Db& db = this->getDb();
    auto q = db.MakeQuery();
    q->prepare(this->getPreparedSelect());
    if (db.ExecuteQuery(q)) {
      while (q->next()) {
        QSharedPointer<DbItemT<IntT> > item;
        int index = 0;
        IntT id = q->value(index++).toLongLong();
        if (this->OnRowRead(q, index, item)) {
          item->Id = id;
          auto itr = mItems.find(id);
          if (itr != mItems.end()) {
            if (!itr.value()->Equals(*item)) {
              DbTableA<IntT>::RemoveIndexItem(*itr.value());
              itr.value() = item;
              DbTableA<IntT>::AddIndexItem(*item);
              reload = true;
            }
          } else {
            mItems[id] = item;
            DbTableA<IntT>::AddIndexItem(*item);
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
      this->RecreateIndexes();
    }
    return reload;
  }

  void Clear()
  {
    this->ClearIndexes();
    mItems.clear();
    mLoaded = false;
  }

  bool Select(const QString& where, QMap<IntT, QSharedPointer<DbItemImpT> >& itemMap)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedSelect() + where);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }

    GetSelectedMap(q, itemMap);
    return true;
  }

  bool Select(const QString& where, QList<QSharedPointer<DbItemImpT> >& itemList)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedSelect() + where);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }

    GetSelectedList(q, itemList);
    return true;
  }

  bool Select(const QString& where, QVector<QSharedPointer<DbItemImpT> >& itemList)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedSelect() + where);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }

    GetSelectedVector(q, itemList);
    return true;
  }

  bool SelectWith(const QString& with, const QString& where, QList<QSharedPointer<DbItemImpT> >& itemList)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(with + " " + this->getPreparedSelect() + where);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }

    GetSelectedList(q, itemList);
    return true;
  }

  bool SelectWith(const QString& with, const QString& where, QVector<QSharedPointer<DbItemImpT> >& itemList)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(with + " " + this->getPreparedSelect() + where);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }

    GetSelectedVector(q, itemList);
    return true;
  }

  bool SelectJoin(const QString& tableTag, const QString& join, const QString& where, QList<QSharedPointer<DbItemImpT> >& itemList)
  {
    this->PrepareStrings();
    QString select = GetSelect(tableTag) + "FROM " + this->getTable() + " " + tableTag + QString(" ") + join + QString(" ") + where;

    auto q = this->getDb().MakeQuery();
    q->prepare(select);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }
    GetSelectedList(q, itemList);
    return true;
  }

  bool SelectJoin(const QString& tableTag, const QString& join, const QString& where, QVector<QSharedPointer<DbItemImpT> >& itemList)
  {
    this->PrepareStrings();
    QString select = GetSelect(tableTag) + "FROM " + this->getTable() + " " + tableTag + QString(" ") + join + QString(" ") + where;

    auto q = this->getDb().MakeQuery();
    q->prepare(select);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }
    GetSelectedVector(q, itemList);
    return true;
  }

  bool SelectOne(const QString& where, QSharedPointer<DbItemImpT>& item)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedSelect() + where);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }

    if (q->size() > 1) {
      Log.Warning(QString("Select one return more (count: %1)").arg(q->size()));
    }
    if (q->next()) {
      int index = 0;
      IntT id = q->value(index++).toLongLong();
      QSharedPointer<DbItemT<IntT> > itemBase;
      if (this->OnRowRead(q, index, itemBase)) {
        itemBase->Id = id;
        item = qSharedPointerCast<DbItemImpT, DbItemT<IntT> >(itemBase);
      }
    } else {
      item.clear();
    }
    return true;
  }

  bool SelectCount(const QString& where, qint64& count)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedSelCnt() + where);
    if (!this->getDb().ExecuteQuery(q) || !q->next()) {
      return false;
    }

    count = q->value(0).toLongLong();
    return true;
  }

  bool SelectTopId(const QString& where, IntT& id)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(QString(this->getPreparedSelTop()).arg(where));
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }

    id = q->next()? q->value(0).toLongLong(): 0;
    return true;
  }

  bool SelectById(const IntT& id, QSharedPointer<DbItemImpT>& item)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedSelOne());
    q->addBindValue(id);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }

    if (q->next()) {
      int index = 0;
      QSharedPointer<DbItemT<IntT> > item_;
      if (this->OnRowRead(q, index, item_)) {
        item_->Id = id;
        item = qSharedPointerCast<DbItemImpT, DbItemT<IntT> >(item_);
      } else {
        return false;
      }
      AddToCache(item);
    } else {
      item.clear();
    }
    return true;
  }

  bool Update(const QSharedPointer<DbItemImpT>& item)
  {
    if (!item->Id) {
      return Insert(item);
    }

    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedUpdate());
    int index = 0;
    if (!this->OnRowWrite(q, index, *item)) {
      return false;
    }
    q->bindValue(index++, item->Id);

    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }
    AddToCache(item);
    return true;
  }

  bool Insert(const QSharedPointer<DbItemImpT>& item)
  {
    this->PrepareStrings();

    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedInsert());
    int index = 0;
    if (!this->OnRowWrite(q, index, *item)) {
      return false;
    }

    if (!this->getDb().ExecuteQuery(q) || !q->next()) {
      return false;
    }
    item->Id = q->value(0).toLongLong();
    AddToCache(item);
    return true;
  }

  bool InsertCopy(const QSharedPointer<DbItemImpT>& item, IntT* cloneId = nullptr)
  {
    return InsertCopy(item->Id, cloneId);
  }

  bool InsertCopy(const IntT& id, IntT* cloneId = nullptr)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedInsCopy());
    q->bindValue(0, id);

    if (!this->getDb().ExecuteQuery(q) || !q->next()) {
      return false;
    }
    if (cloneId) {
      *cloneId = q->value(0).toLongLong();
    }
    return true;
  }

  bool Delete(const QString& where)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedDelete() + " " + where);

    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }
    return true;
  }

  bool Delete(const IntT& id)
  {
    if (!Delete(QString("WHERE _id=%1").arg(id))) {
      return false;
    }
    mItems.remove(id);
    return true;
  }

  bool Delete(const DbItemImpT& item)
  {
    return Delete(item.Id);
  }

  bool Delete(const QSharedPointer<DbItemImpT>& item)
  {
    return Delete(item->Id);
  }

  bool Delete(const QVector<QSharedPointer<DbItemImpT> >& itemList)
  {
    QStringList idList;
    foreach (const QSharedPointer<DbItemImpT>& item, itemList) {
      idList << QString::number(item->Id);
    }
    if (idList.isEmpty()) {
      return true;
    }
    if (!Delete(QString("WHERE _id IN (%1)").arg(idList.join(",")))) {
      return false;
    }
    for (const QSharedPointer<DbItemImpT>& item: itemList) {
      RemoveFromCache(item);
    }
    return true;
  }

  bool InsertPack(const QList<QSharedPointer<DbItemImpT> >& itemList, int count = 100)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    int lastCount = 0;
    for (int i = 0; i < itemList.size(); i += count) {
      int curCount = qMin(count, itemList.size() - i);
      if (curCount != lastCount) {
        QStringList valuesList;
        for (int j = 0; j < curCount; j++) {
          valuesList << this->getPreparedValues();
        }
        q->prepare(this->getPreparedInsertPack() + valuesList.join(',') + " RETURNING _id");
        lastCount = curCount;
      }

      int index = 0;
      for (int j = 0; j < curCount; j++) {
        this->OnRowWrite(q, index, *itemList.at(i + j));
      }

      if (!this->getDb().ExecuteQuery(q)) {
        return false;
      }

      for (int j = 0; q->next(); j++) {
        IntT id = q->value(0).toLongLong();
        const QSharedPointer<DbItemImpT>& item = itemList.at(i + j);
        item->Id = id;
        AddToCache(item);
      }
    }
    return true;
  }

  bool InsertPack(const QVector<QSharedPointer<DbItemImpT> >& itemList, int count = 100)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    int lastCount = 0;
    for (int i = 0; i < itemList.size(); i += count) {
      int curCount = qMin(count, itemList.size() - i);
      if (curCount != lastCount) {
        QStringList valuesList;
        for (int j = 0; j < curCount; j++) {
          valuesList << this->getPreparedValues();
        }
        q->prepare(this->getPreparedInsertPack() + valuesList.join(',') + " RETURNING _id");
        lastCount = curCount;
      }

      int index = 0;
      for (int j = 0; j < curCount; j++) {
        this->OnRowWrite(q, index, *itemList.at(i + j));
      }

      if (!this->getDb().ExecuteQuery(q)) {
        return false;
      }
    }
    return true;
  }

  bool RestorePack(const QVector<QSharedPointer<DbItemImpT> >& itemList, int count = 100)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    int lastCount = 0;
    for (int i = 0; i < itemList.size(); i += count) {
      int curCount = qMin(count, itemList.size() - i);
      if (curCount != lastCount) {
        QStringList valuesList;
        for (int j = 0; j < curCount; j++) {
          valuesList << this->getPreparedIdValues();
        }
        q->prepare(this->getPreparedRestorePack() + valuesList.join(','));
        lastCount = curCount;
      }

      int index = 0;
      for (int j = 0; j < curCount; j++) {
        const DbItemImpT& item = *itemList.at(i + j);
        q->bindValue(index++, item.Id);
        this->OnRowWrite(q, index, item);
      }

      if (!this->getDb().ExecuteNonQuery(q)) {
        return false;
      }
    }
    return true;
  }

protected:
  QString GetSelect()
  {
    this->PrepareStrings();
    return this->getPreparedSelect();
  }

  QString GetSelect(const QString& tag)
  {
    this->PrepareStrings();

    QString select = QString("SELECT %1._id").arg(tag);
    for (int i = 0; i < this->getColumns().size(); i++) {
      select.append(QString(",%1.%2").arg(tag, this->getColumns().at(i)));
    }
    return select + " ";
  }

  QString GetRows(const QString& tag)
  {
    this->PrepareStrings();

    QString rows = QString("%1._id").arg(tag);
    for (int i = 0; i < this->getColumns().size(); i++) {
      rows.append(QString(",%1.%2").arg(tag, this->getColumns().at(i)));
    }
    return rows;
  }

  void GetSelectedMap(QueryS& q, QMap<IntT, QSharedPointer<DbItemImpT> >& itemMap)
  {
    while (q->next()) {
      int index = 0;
      IntT id = q->value(index++).toLongLong();
      QSharedPointer<DbItemT<IntT> > item;
      if (this->OnRowRead(q, index, item)) {
        item->Id = id;
        itemMap[id] = item.template staticCast<DbItemImpT>();
      }
    }
  }

  void GetSelectedList(QueryS& q, QList<QSharedPointer<DbItemImpT> >& itemList)
  {
    while (q->next()) {
      int index = 0;
      IntT id = q->value(index++).toLongLong();
      QSharedPointer<DbItemT<IntT> > item;
      if (this->OnRowRead(q, index, item)) {
        item->Id = id;
        itemList.append(qSharedPointerCast<DbItemImpT, DbItemT<IntT> >(item));
      }
    }
  }

  void GetSelectedVector(QueryS& q, QVector<QSharedPointer<DbItemImpT> >& itemList)
  {
    itemList.reserve(itemList.size() + q->size());
    while (q->next()) {
      int index = 0;
      IntT id = q->value(index++).toLongLong();
      QSharedPointer<DbItemT<IntT> > item;
      if (this->OnRowRead(q, index, item)) {
        item->Id = id;
        itemList.append(qSharedPointerCast<DbItemImpT, DbItemT<IntT> >(item));
      }
    }
  }
};
