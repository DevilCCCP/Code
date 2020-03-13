#include <QSqlQuery>
#include <QVariant>

#include "ObjectStateHours.h"


QString ObjectStateHoursTable::TableName()
{
  return "object_state_hours";
}

QString ObjectStateHoursTable::Columns()
{
  return "_object,_ostype,state_good,state_bad,triggered_hour";
}

bool ObjectStateHoursTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item)
{
  ObjectStateHours* it;
  item.reset(it = new ObjectStateHours());
  it->ObjectId = q->value(index++).value<int>();
  it->OstypeId = q->value(index++).value<int>();
  it->StateGood = q->value(index++).value<int>();
  it->StateBad = q->value(index++).value<int>();
  it->TriggeredHour = q->value(index++).value<QDateTime>();
  return true;
}

bool ObjectStateHoursTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item)
{
  const ObjectStateHours& it = static_cast<const ObjectStateHours&>(item);
  q->bindValue(index++, Db::ToKey(it.ObjectId));
  q->bindValue(index++, Db::ToKey(it.OstypeId));
  q->bindValue(index++, it.StateGood);
  q->bindValue(index++, it.StateBad);
  q->bindValue(index++, it.TriggeredHour);
  return true;
}


ObjectStateHoursTable::ObjectStateHoursTable(const Db& _Db)
  : DbTableT<qint64, ObjectStateHours>(_Db)
{
}
