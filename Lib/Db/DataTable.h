#pragma once

#include <QByteArray>

#include "DbTable.h"


DefineClassS(DataItem);
//DefineClassS(DataTable);
class QDataStream;

class DataItem: public DbItemT<qint64>
{
public:
  /*override */virtual bool Equals(const DbItemT<qint64>& other) Q_DECL_OVERRIDE;

  /*new */virtual void Serialize(QDataStream* stream) const = 0;
  /*new */virtual void Deserialize(QDataStream* stream) = 0;

public:
  DataItem(): DbItemT<qint64>() { }
  /*override */virtual ~DataItem() { }
};

template<typename DataItemT>
class DataTable: public DbTableT<qint64, DataItemT>
{
protected:
  /*override */virtual QString Columns() Q_DECL_OVERRIDE
  {
    return "data";
  }

//  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item) Q_DECL_OVERRIDE;
//  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item) Q_DECL_OVERRIDE;

  bool OnRowReadData(QSqlQueryS& q, int& index, DataItemT* item)
  {
    QByteArray data = q->value(index++).value<QByteArray>();
    QDataStream stream(data);
    item->Deserialize(&stream);
    return true;
  }

  bool OnRowWriteData(QSqlQueryS& q, int& index, const DataItemT& item)
  {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    item.Serialize(&stream);
    q->bindValue(index++, data);
    return true;
  }

public:
  DataTable(const Db& _Db)
    : DbTableT<qint64, DataItemT>(_Db)
  {
  }
};

