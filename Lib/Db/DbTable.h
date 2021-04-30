#pragma once

#include <QString>
#include <QVariant>
#include <QVector>
#include <QList>
#include <qsystemdetection.h>

#include <Lib/Include/Common.h>
#include <Lib/Log/Log.h>

#include "Db.h"


DefineClassS(QSqlQuery);

class DbItemA
{
public:
  /*new*/virtual bool Equals(const DbItemA& other) { Q_UNUSED(other); return false; }

  /*new */virtual qint64 Key(int index) const { Q_UNUSED(index); return 0; }
  /*new */virtual void SetKey(int index, qint64 id) { Q_UNUSED(index); Q_UNUSED(id); }
  /*new */virtual QString Text(int column) const { Q_UNUSED(column); return QString(); }
  /*new */virtual bool SetText(int column, const QString& text) { Q_UNUSED(column); Q_UNUSED(text); return false; }
  /*new */virtual QVariant Data(int column) const { Q_UNUSED(column); return QVariant(); }
  /*new */virtual bool SetData(int column, const QVariant& data) { Q_UNUSED(column); Q_UNUSED(data); return false; }
  /*new */virtual bool IsDataReadOnly(int column) { Q_UNUSED(column); return false; }

protected:
  static QString TextFromData(const QByteArray& data)
  {
    return data.size() > 20
        ? QString("%1..%2").arg(QString::fromLatin1(data.left(12).toHex()), QString::fromLatin1(data.right(6).toHex()))
        : QString::fromLatin1(data.toHex());
  }

public:
  DbItemA() { }
  /*new*/virtual ~DbItemA() { }
};

template<typename IntT>
class DbItemT: public DbItemA
{
public:
  IntT Id;

public:
  /*new */virtual bool Equals(const DbItemT<IntT>& other) const { return Id == other.Id && Id != 0; }

public:
  DbItemT(): Id(0) { }
  DbItemT(int _Id): Id(_Id) { }
  /*new*/virtual ~DbItemT() { }
};

template<typename IntT>
class DbTableA
{
  const Db& mDb;

protected:
  PROTECTED_GET(QString,     Table)
  PROTECTED_GET(QStringList, Columns)

  PROTECTED_GET(QString,     PreparedColumns)
  PROTECTED_GET(QString,     PreparedValues)
  PROTECTED_GET(QString,     PreparedIdValues)
  PROTECTED_GET(QString,     PreparedSelect)
  PROTECTED_GET(QString,     PreparedSelOne)
  PROTECTED_GET(QString,     PreparedSelCnt)
  PROTECTED_GET(QString,     PreparedSelTop)
  PROTECTED_GET(QString,     PreparedInsert)
  PROTECTED_GET(QString,     PreparedInsCopy)
  PROTECTED_GET(QString,     PreparedRestore)
  PROTECTED_GET(QString,     PreparedUpdate)
  PROTECTED_GET(QString,     PreparedDelete)
  PROTECTED_GET(QString,     PreparedInsertPack)
  PROTECTED_GET(QString,     PreparedRestorePack)

protected:
  /*new */virtual QString TableName() = 0;
  /*new */virtual QString Columns() = 0;

  /*new */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<IntT> >& item) = 0;
  /*new */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<IntT>& item) = 0;

  /*new */virtual void AddIndexItem(const DbItemT<IntT>& item) { Q_UNUSED(item); }
  /*new */virtual void RemoveIndexItem(const DbItemT<IntT>& item) { Q_UNUSED(item); }
  /*new */virtual void RecreateIndexes() { }
  /*new */virtual void ClearIndexes() { }

  /*new */virtual bool CreateDefaultItem(QSharedPointer<DbItemT<IntT> >& item) { Q_UNUSED(item); return false; }
  /*new */virtual void NewDefaultItem(QSharedPointer<DbItemT<IntT> >& item) { Q_UNUSED(item); }

public:
  /*new */virtual QStringList Headers() const { return QStringList(); }
  /*new */virtual QString Icon() const { return QString(); }

protected:
  const Db& getDb() const { return mDb; }

public:
  QString KeyColumn(int index) const
  {
    this->PrepareStrings();
    for (int i = 0; i < this->getColumns().size(); i++) {
      const QString& column = this->getColumns().at(i);
      if (column.startsWith('_')) {
        if (index-- <= 0) {
          return column;
        }
      }
    }
    return QString();
  }

  QString DataColumn(int index) const
  {
    this->PrepareStrings();
    for (int i = 0; i < this->getColumns().size(); i++) {
      const QString& column = this->getColumns().at(i);
      if (!column.startsWith('_')) {
        if (index-- <= 0) {
          return column;
        }
      }
    }
    return QString();
  }

  bool TestTable(bool& ok)
  {
    auto q = mDb.MakeQuery();
    q->prepare(QString("SELECT COUNT(table_name) FROM information_schema.columns"
                       " WHERE table_name = %1 AND table_schema = 'public'").arg(ToSql(TableName())));
    if (!mDb.ExecuteQuery(q) || !q->next()) {
      return false;
    }
    ok = q->value(0).toInt() > 0;
    return true;
  }

  bool GetCount(qint64& count)
  {
    auto q = mDb.MakeQuery();
    q->prepare(QString("SELECT reltuples::bigint AS approximate_row_count FROM pg_class WHERE relname = %1;").arg(ToSql(TableName())));
    if (!mDb.ExecuteQuery(q) || !q->next()) {
      return false;
    }
    count = q->value(0).toInt();
    return true;
  }

  bool Create(QSharedPointer<DbItemT<IntT> >& item)
  {
    return CreateDefaultItem(item);
  }

  void New(QSharedPointer<DbItemT<IntT> >& item)
  {
    NewDefaultItem(item);
  }

  bool Select(const QString& where, QVector<QSharedPointer<DbItemT<IntT> > >& itemsList)
  {
    auto q = mDb.MakeQuery();
    q->prepare(QString("SELECT _id,") + Columns() + " FROM " + TableName() + ' ' + where);
    if (!mDb.ExecuteQuery(q)) {
      return false;
    }

    itemsList.reserve(itemsList.size() + q->size());
    while (q->next()) {
      int index = 0;
      IntT id = q->value(index++).toLongLong();
      QSharedPointer<DbItemT<IntT> > item;
      if (this->OnRowRead(q, index, item)) {
        item->Id = id;
        itemsList.append(item);
      }
    }
    return true;
  }

  bool Select(const QString& where, QMap<IntT, QSharedPointer<DbItemT<IntT> > >& itemsMap)
  {
    auto q = mDb.MakeQuery();
    q->prepare(QString("SELECT _id,") + Columns() + " FROM " + TableName() + ' ' + where);
    if (!mDb.ExecuteQuery(q)) {
      return false;
    }

    while (q->next()) {
      int index = 0;
      IntT id = q->value(index++).toLongLong();
      QSharedPointer<DbItemT<IntT> > item;
      if (this->OnRowRead(q, index, item)) {
        item->Id = id;
        itemsMap[item->Id] = item;
      }
    }
    return true;
  }

  bool Insert(const QSharedPointer<DbItemT<IntT> >& item)
  {
    return Insert(*item);
  }

  bool Insert(DbItemT<IntT>& item)
  {
    this->PrepareStrings();

    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedInsert());
    int index = 0;
    if (!this->OnRowWrite(q, index, item)) {
      return false;
    }

    if (!this->getDb().ExecuteQuery(q) || !q->next()) {
      return false;
    }
    item.Id = q->value(0).toLongLong();
    return true;
  }

  bool Restore(const QSharedPointer<DbItemT<IntT> >& item)
  {
    return Restore(*item);
  }

  bool Restore(DbItemT<IntT>& item)
  {
    this->PrepareStrings();

    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedRestore());
    int index = 0;
    q->bindValue(index++, item.Id);
    if (!this->OnRowWrite(q, index, item)) {
      return false;
    }

    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }
    return true;
  }

  bool SequenceUpdate(IntT id)
  {
    auto q = this->getDb().MakeQuery();
    q->prepare(QString("SELECT setval('%1__id_seq', %2)").arg(getTable()).arg(id));
    if (!this->getDb().ExecuteNonQuery(q)) {
      return false;
    }
    return true;
  }

  bool Update(const QSharedPointer<DbItemT<IntT> >& item)
  {
    return Update(*item);
  }

  bool Update(DbItemT<IntT>& item)
  {
    if (!item.Id) {
      return Insert(item);
    }

    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedUpdate());
    int index = 0;
    if (!this->OnRowWrite(q, index, item)) {
      return false;
    }
    q->bindValue(index++, item.Id);

    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }
    return true;
  }

  bool Delete(const IntT& id)
  {
    auto q = mDb.MakeQuery();
    q->prepare(QString("DELETE FROM ") + TableName() + " WHERE _id=" + QString::number(id));

    if (!mDb.ExecuteQuery(q)) {
      return false;
    }
    return true;
  }

protected:
  void PrepareStrings()
  {
    if (!mTable.isEmpty()) {
      return;
    }

    mTable = this->TableName();
    mPreparedColumns = this->Columns();
    mColumns = mPreparedColumns.split(',');
    QStringList values;
    QStringList setValues;
    for (int i = 0; i < mColumns.size(); i++) {
      values.append("?");
      setValues.append(mColumns[i] + "=?");
    }
    mPreparedValues      = QString("(") + values.join(',') + QString(")");
    mPreparedIdValues    = QString("(?,") + values.join(',') + QString(")");
    mPreparedSelect      = QString("SELECT _id,") + mPreparedColumns + " FROM " + mTable + " ";
    mPreparedSelOne      = QString("SELECT ") + mPreparedColumns + " FROM " + mTable + " WHERE _id=? LIMIT 1";
    mPreparedSelCnt      = QString("SELECT COUNT(_id) FROM ") + mTable + " ";
    mPreparedSelTop      = QString("SELECT _id FROM ") + mTable + " %1 ORDER BY _id DESC LIMIT 1";
    mPreparedInsert      = QString("INSERT INTO ") + mTable + "(" + mPreparedColumns + ") VALUES " + mPreparedValues + " RETURNING _id";
    mPreparedRestore     = QString("INSERT INTO ") + mTable + "(_id," + mPreparedColumns + ") VALUES " + mPreparedIdValues;
    mPreparedInsCopy     = QString("INSERT INTO ") + mTable + "(" + mPreparedColumns + ") (SELECT " + mPreparedColumns
        + " FROM " + mTable + " WHERE _id = ?) RETURNING _id;";
    mPreparedUpdate      = QString("UPDATE ") + mTable + " SET " + setValues.join(',') + " WHERE _id=?";
    mPreparedDelete      = QString("DELETE FROM ") + mTable + " ";
    mPreparedInsertPack  = QString("INSERT INTO ") + mTable + "(" + mPreparedColumns + ") VALUES ";
    mPreparedRestorePack = QString("INSERT INTO ") + mTable + "(_id," + mPreparedColumns + ") VALUES ";
  }


public:
  DbTableA(const Db& _Db)
    : mDb(_Db)
  { }
  virtual ~DbTableA()
  { }
};

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

  bool GetItems(QList<QSharedPointer<DbItemImpT> >& items) const
  {
    for (auto itr = mItems.begin(); itr != mItems.end(); itr++) {
      items.append(qSharedPointerCast<DbItemImpT, DbItemT<IntT> >(itr.value()));
    }
    return true;
  }

  bool GetItems(QVector<QSharedPointer<DbItemImpT> >& items) const
  {
    for (auto itr = mItems.begin(); itr != mItems.end(); itr++) {
      items.append(qSharedPointerCast<DbItemImpT, DbItemT<IntT> >(itr.value()));
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

    if (SelectById(id, item) && item) {
      AddToCache(item);
      return true;
    }
    return false;
  }

  void AddToCache(QSharedPointer<DbItemImpT>& item)
  {
    mItems[item->Id] = item;
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
    QSet<IntT> oldIds = mItems.keys().toSet();

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

  bool Select(const QString& where, QMap<IntT, QSharedPointer<DbItemImpT> >& itemsMap)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedSelect() + where);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }

    GetSelectedMap(q, itemsMap);
    return true;
  }

  bool Select(const QString& where, QList<QSharedPointer<DbItemImpT> >& itemsList)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedSelect() + where);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }

    GetSelectedList(q, itemsList);
    return true;
  }

  bool Select(const QString& where, QVector<QSharedPointer<DbItemImpT> >& items)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(this->getPreparedSelect() + where);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }

    GetSelectedVector(q, items);
    return true;
  }

  bool SelectWith(const QString& with, const QString& where, QList<QSharedPointer<DbItemImpT> >& items)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(with + " " + this->getPreparedSelect() + where);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }

    GetSelectedList(q, items);
    return true;
  }

  bool SelectWith(const QString& with, const QString& where, QVector<QSharedPointer<DbItemImpT> >& items)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    q->prepare(with + " " + this->getPreparedSelect() + where);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }

    GetSelectedVector(q, items);
    return true;
  }

  bool SelectJoin(const QString& tableTag, const QString& join, const QString& where, QList<QSharedPointer<DbItemImpT> >& items)
  {
    this->PrepareStrings();
    QString select = GetSelect(tableTag) + "FROM " + this->getTable() + " " + tableTag + QString(" ") + join + QString(" ") + where;

    auto q = this->getDb().MakeQuery();
    q->prepare(select);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }
    GetSelectedList(q, items);
    return true;
  }

  bool SelectJoin(const QString& tableTag, const QString& join, const QString& where, QVector<QSharedPointer<DbItemImpT> >& items)
  {
    this->PrepareStrings();
    QString select = GetSelect(tableTag) + "FROM " + this->getTable() + " " + tableTag + QString(" ") + join + QString(" ") + where;

    auto q = this->getDb().MakeQuery();
    q->prepare(select);
    if (!this->getDb().ExecuteQuery(q)) {
      return false;
    }
    GetSelectedVector(q, items);
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
    if (mLoaded) {
      mItems[item->Id] = item;
    }
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
    if (mLoaded) {
      mItems[item->Id] = item;
    }
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
    bool ok = Delete(QString("WHERE _id=%1").arg(id));
    if (ok && mLoaded) {
      mItems.remove(id);
    }
    return ok;
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
    return Delete(QString("WHERE _id IN (%1)").arg(idList.join(",")));
  }

  bool InsertPack(const QList<QSharedPointer<DbItemImpT> >& items, int count = 100)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    int lastCount = 0;
    for (int i = 0; i < items.size(); i += count) {
      int curCount = qMin(count, items.size() - i);
      if (curCount != lastCount) {
        QStringList valuesList;
        for (int j = 0; j < curCount; j++) {
          valuesList << this->getPreparedValues();
        }
        q->prepare(this->getPreparedInsertPack() + valuesList.join(','));
        lastCount = curCount;
      }

      int index = 0;
      for (int j = 0; j < curCount; j++) {
        this->OnRowWrite(q, index, *items.at(i + j));
      }

      if (!this->getDb().ExecuteNonQuery(q)) {
        return false;
      }
    }
    return true;
  }

  bool InsertPack(const QVector<QSharedPointer<DbItemImpT> >& items, int count = 100)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    int lastCount = 0;
    for (int i = 0; i < items.size(); i += count) {
      int curCount = qMin(count, items.size() - i);
      if (curCount != lastCount) {
        QStringList valuesList;
        for (int j = 0; j < curCount; j++) {
          valuesList << this->getPreparedValues();
        }
        q->prepare(this->getPreparedInsertPack() + valuesList.join(','));
        lastCount = curCount;
      }

      int index = 0;
      for (int j = 0; j < curCount; j++) {
        this->OnRowWrite(q, index, *items.at(i + j));
      }

      if (!this->getDb().ExecuteNonQuery(q)) {
        return false;
      }
    }
    return true;
  }

  bool RestorePack(const QVector<QSharedPointer<DbItemImpT> >& items, int count = 100)
  {
    this->PrepareStrings();
    auto q = this->getDb().MakeQuery();
    int lastCount = 0;
    for (int i = 0; i < items.size(); i += count) {
      int curCount = qMin(count, items.size() - i);
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
        const DbItemImpT& item = *items.at(i + j);
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

  void GetSelectedMap(QueryS& q, QMap<IntT, QSharedPointer<DbItemImpT> >& itemsMap)
  {
    while (q->next()) {
      int index = 0;
      IntT id = q->value(index++).toLongLong();
      QSharedPointer<DbItemT<IntT> > item;
      if (this->OnRowRead(q, index, item)) {
        item->Id = id;
        itemsMap[id] = item.template staticCast<DbItemImpT>();
      }
    }
  }

  void GetSelectedList(QueryS& q, QList<QSharedPointer<DbItemImpT> >& itemsList)
  {
    while (q->next()) {
      int index = 0;
      IntT id = q->value(index++).toLongLong();
      QSharedPointer<DbItemT<IntT> > item;
      if (this->OnRowRead(q, index, item)) {
        item->Id = id;
        itemsList.append(qSharedPointerCast<DbItemImpT, DbItemT<IntT> >(item));
      }
    }
  }

  void GetSelectedVector(QueryS& q, QVector<QSharedPointer<DbItemImpT> >& items)
  {
    items.reserve(items.size() + q->size());
    while (q->next()) {
      int index = 0;
      IntT id = q->value(index++).toLongLong();
      QSharedPointer<DbItemT<IntT> > item;
      if (this->OnRowRead(q, index, item)) {
        item->Id = id;
        items.append(qSharedPointerCast<DbItemImpT, DbItemT<IntT> >(item));
      }
    }
  }
};

typedef QSharedPointer<DbItemA>  DbItemAS;
typedef DbItemT<int>             DbItem;
typedef DbItemT<qint64>          DbItemB;
typedef QSharedPointer<DbItem>   DbItemS;
typedef QSharedPointer<DbItemB>  DbItemBS;

typedef DbTableA<int>            DbTable;
typedef DbTableA<qint64>         DbTableB;
typedef QSharedPointer<DbTable>  DbTableS;
typedef QSharedPointer<DbTableB> DbTableBS;


