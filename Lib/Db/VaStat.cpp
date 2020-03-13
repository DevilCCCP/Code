#include <QSqlQuery>
#include <QVariant>

#include "VaStat.h"


QString VaStatTable::TableName()
{
  return "va_stat";
}

QString VaStatTable::Columns()
{
  return "_object,_vstype";
}

bool VaStatTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<int> >& item)
{
  VaStat* it;
  item.reset(it = new VaStat());
  it->ObjectId = q->value(index++).value<int>();
  it->VstypeId = q->value(index++).value<int>();
  return true;
}

bool VaStatTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<int>& item)
{
  const VaStat& it = static_cast<const VaStat&>(item);
  q->bindValue(index++, Db::ToKey(it.ObjectId));
  q->bindValue(index++, Db::ToKey(it.VstypeId));
  return true;
}

bool VaStatTable::GetSimple(int objectId, int vstypeId, VaStatS& vaStatItem, bool useCache)
{
  auto keyPair = qMakePair(objectId, vstypeId);
  if (useCache) {
    auto itr = mItemsMap.find(keyPair);
    if (itr != mItemsMap.end()) {
      vaStatItem = itr.value();
      return true;
    }
  }

  QList<VaStatS> list;
  if (!Select(QString("WHERE _object = %1 AND _vstype = %2").arg(objectId).arg(vstypeId), list)) {
    return false;
  }
  if (!list.isEmpty()) {
    vaStatItem = list.first();
  } else {
    vaStatItem.reset(new VaStat());
    vaStatItem->ObjectId = objectId;
    vaStatItem->VstypeId = vstypeId;
    if (!Insert(vaStatItem)) {
      return false;
    }
  }
  mItemsMap[keyPair] = vaStatItem;
  return true;
}


VaStatTable::VaStatTable(const Db& _Db)
  : DbTableT<int, VaStat>(_Db)
{
}
