#include <QSqlQuery>
#include <QVariant>

#include "VaStatHours.h"


QString VaStatHoursTable::TableName()
{
  return "va_stat_hours";
}

QString VaStatHoursTable::Columns()
{
  return "_vstat,_fimage,hour";
}

bool VaStatHoursTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item)
{
  VaStatHours* it;
  item.reset(it = new VaStatHours());
  it->VstatId = q->value(index++).value<int>();
  it->FimageId = q->value(index++).value<qint64>();
  it->Hour = q->value(index++).value<QDateTime>();
  return true;
}

bool VaStatHoursTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item)
{
  const VaStatHours& it = static_cast<const VaStatHours&>(item);
  q->bindValue(index++, Db::ToKey(it.VstatId));
  q->bindValue(index++, Db::ToKey(it.FimageId));
  q->bindValue(index++, it.Hour);
  return true;
}


VaStatHoursTable::VaStatHoursTable(const Db& _Db)
  : DbTableT<qint64, VaStatHours>(_Db)
{
}
