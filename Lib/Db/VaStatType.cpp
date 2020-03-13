#include <QSqlQuery>
#include <QVariant>

#include "VaStatType.h"


QString VaStatTypeTable::TableName()
{
  return "va_stat_type";
}

QString VaStatTypeTable::Columns()
{
  return "abbr,name,descr";
}

bool VaStatTypeTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<int> >& item)
{
  VaStatType* it;
  item.reset(it = new VaStatType());
  it->Abbr = q->value(index++).value<QString>();
  it->Name = q->value(index++).value<QString>();
  it->Descr = q->value(index++).value<QString>();
  return true;
}

bool VaStatTypeTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<int>& item)
{
  const VaStatType& it = static_cast<const VaStatType&>(item);
  q->bindValue(index++, it.Abbr);
  q->bindValue(index++, it.Name);
  q->bindValue(index++, it.Descr);
  return true;
}

bool VaStatTypeTable::GetByAbbr(const QString& abbr, VaStatTypeS& item, bool useCache)
{
  auto itr = useCache? mItemsCache.find(abbr): mItemsCache.end();
  if (itr == mItemsCache.end()) {
    QList<VaStatTypeS> list;
    if (!Select(QString("WHERE abbr = %1").arg(ToSql(abbr)), list)) {
      return false;
    }
    if (list.size() != 1) {
      return false;
    }
    itr = mItemsCache.insert(abbr, list.first());
  }

  item = itr.value();
  return true;
}


VaStatTypeTable::VaStatTypeTable(const Db& _Db)
  : DbTableT<int, VaStatType>(_Db)
{
}
