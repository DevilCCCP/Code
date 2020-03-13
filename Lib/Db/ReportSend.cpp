#include <QSqlQuery>
#include <QVariant>

#include "ReportSend.h"


QString ReportSendTable::TableName()
{
  return "report_send";
}

QString ReportSendTable::Columns()
{
  return "_oto,_last_report,send_time";
}

bool ReportSendTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item)
{
  ReportSend* it;
  item.reset(it = new ReportSend());
  it->OtoId = q->value(index++).value<int>();
  it->LastReportId = q->value(index++).value<qint64>();
  it->SendTime = q->value(index++).value<QDateTime>();
  return true;
}

bool ReportSendTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item)
{
  const ReportSend& it = static_cast<const ReportSend&>(item);
  q->bindValue(index++, Db::ToKey(it.OtoId));
  q->bindValue(index++, Db::ToKey(it.LastReportId));
  q->bindValue(index++, it.SendTime);
  return true;
}


ReportSendTable::ReportSendTable(const Db& _Db)
  : DbTableT<qint64, ReportSend>(_Db)
{
}
