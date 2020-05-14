#include <QSqlQuery>
#include <QVariant>
#include <QDataStream>

#include "DataTable.h"


bool DataItem::Equals(const DbItemT<qint64>& other) const
{
  if (const DataItem* otherItem = dynamic_cast<const DataItem*>(&other)) {
    QByteArray data1;
    QDataStream stream1(&data1, QIODevice::WriteOnly);
    this->Serialize(&stream1);
    QByteArray data2;
    QDataStream stream2(&data2, QIODevice::WriteOnly);
    otherItem->Serialize(&stream2);
    return data1 == data2;
  }
  return false;
}

//bool DataTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item)
//{
//  DataItem* it;
//  item.reset(it = new DataItem());
//  return OnRowReadData(q, index, it);
//}

//bool DataTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item)
//{
//  const DataItem& it = static_cast<const DataItem&>(item);
//  return OnRowWriteData(q, index, it);
//}
