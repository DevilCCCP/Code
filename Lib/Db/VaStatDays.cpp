#include <QSqlQuery>
#include <QVariant>

#include "VaStatDays.h"


QString VaStatDaysTable::TableName()
{
  return "va_stat_days";
}

QString VaStatDaysTable::Columns()
{
  return "_vstat,_fimage,day";
}

bool VaStatDaysTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item)
{
  VaStatDays* it;
  item.reset(it = new VaStatDays());
  it->VstatId = q->value(index++).value<int>();
  it->FimageId = q->value(index++).value<qint64>();
  it->Day = q->value(index++).value<QDateTime>();
  return true;
}

bool VaStatDaysTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item)
{
  const VaStatDays& it = static_cast<const VaStatDays&>(item);
  q->bindValue(index++, Db::ToKey(it.VstatId));
  q->bindValue(index++, Db::ToKey(it.FimageId));
  q->bindValue(index++, it.Day);
  return true;
}


VaStatDaysTable::VaStatDaysTable(const Db& _Db)
  : DbTableT<qint64, VaStatDays>(_Db)
{
}
