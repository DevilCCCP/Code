#include <QSqlQuery>
#include <QVariant>

#include "Report.h"


QString ReportTable::TableName()
{
  return "report";
}

QString ReportTable::Columns()
{
  return "_object,type,period_begin,period_end,data";
}

bool ReportTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item)
{
  Report* it;
  item.reset(it = new Report());
  it->ObjectId = q->value(index++).value<int>();
  it->Type = q->value(index++).value<int>();
  it->PeriodBegin = q->value(index++).value<QDateTime>();
  it->PeriodEnd = q->value(index++).value<QDateTime>();
  it->Data  = q->value(index++).value<QByteArray>();
  return true;
}

bool ReportTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item)
{
  const Report& it = static_cast<const Report&>(item);
  q->bindValue(index++, Db::ToKey(it.ObjectId));
  q->bindValue(index++, it.Type);
  q->bindValue(index++, it.PeriodBegin);
  q->bindValue(index++, it.PeriodEnd);
  q->bindValue(index++, it.Data);
  return true;
}


ReportTable::ReportTable(const Db& _Db)
  : DbTableT<qint64, Report>(_Db)
{
}
