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

////////////////////////////////
/// DbItemA
////
class DbItemA
{
public:
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

////////////////////////////////
/// DbItemT
////
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

////////////////////////////////
/// DbTableA
////
template<typename IntT>
class DbTableA
{
  const Db& mDb;

protected:
  PROTECTED_GET(QString,     Table)
  PROTECTED_GET(QStringList, Columns)

  PROTECTED_GET(QString,     PreparedColumns)
  PROTECTED_GET(QString,     PreparedValues)
  PROTECTED_GET(QString,     PreparedSelect)
  PROTECTED_GET(QString,     PreparedSelOne)
  PROTECTED_GET(QString,     PreparedSelCnt)
  PROTECTED_GET(QString,     PreparedSelTop)
  PROTECTED_GET(QString,     PreparedInsert)
  PROTECTED_GET(QString,     PreparedInsCopy)
  PROTECTED_GET(QString,     PreparedUpdate)
  PROTECTED_GET(QString,     PreparedDelete)
  PROTECTED_GET(QString,     PreparedInsertPack)

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

  bool Select(const QString& where, QList<QSharedPointer<DbItemT<IntT> > >& itemsList)
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

  QString GetColumnsWithTag(const QString& tag)
  {
    this->PrepareStrings();

    QString columns = QString("%1._id").arg(tag);
    for (int i = 0; i < this->getColumns().size(); i++) {
      columns.append(QString(",%1.%2").arg(tag, this->getColumns().at(i)));
    }
    return columns + " ";
  }

  bool FillItem(QueryS& q, int& index, QSharedPointer<DbItemT<IntT> >& item)
  {
    IntT id = q->value(index++).toLongLong();
    if (!this->OnRowRead(q, index, item)) {
      item.clear();
      return false;
    }

    item->Id = id;
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
    mPreparedValues  = QString("(") + values.join(',') + QString(")");
    mPreparedSelect  = QString("SELECT _id,") + mPreparedColumns + " FROM " + mTable + " ";
    mPreparedSelOne  = QString("SELECT ") + mPreparedColumns + " FROM " + mTable + " WHERE _id=? LIMIT 1";
    mPreparedSelCnt  = QString("SELECT COUNT(_id) FROM ") + mTable + " ";
    mPreparedSelTop  = QString("SELECT _id FROM ") + mTable + " %1 ORDER BY _id DESC LIMIT 1";
    mPreparedInsert  = QString("INSERT INTO ") + mTable + "(" + mPreparedColumns + ") VALUES " + mPreparedValues + " RETURNING _id";
    mPreparedInsCopy = QString("INSERT INTO ") + mTable + "(" + mPreparedColumns + ") (SELECT " + mPreparedColumns
        + " FROM " + mTable + " WHERE _id = ?) RETURNING _id;";
    mPreparedUpdate  = QString("UPDATE ") + mTable + " SET " + setValues.join(',') + " WHERE _id=?";
    mPreparedDelete  = QString("DELETE FROM ") + mTable + " ";
    mPreparedInsertPack = QString("INSERT INTO ") + mTable + "(" + mPreparedColumns + ") VALUES ";
  }


public:
  DbTableA(const Db& _Db)
    : mDb(_Db)
  { }
  virtual ~DbTableA()
  { }
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
