#include <QSqlQuery>
#include <QVariant>

#include "Job.h"


QString JobTable::TableName()
{
  return "job";
}

QString JobTable::Columns()
{
  return "name,descr,data,priority,iter,iter_end,done,fail";
}

bool JobTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item)
{
  Job* it;
  item.reset(it = new Job());
  it->Name = q->value(index++).value<QString>();
  it->Descr = q->value(index++).value<QString>();
  it->Data = q->value(index++).value<QByteArray>();
  it->Priority = q->value(index++).value<int>();
  it->Iter = q->value(index++).value<int>();
  it->IterEnd = q->value(index++).value<int>();
  it->Done = q->value(index++).value<int>();
  it->Fail = q->value(index++).value<int>();
  return true;
}

bool JobTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item)
{
  const Job& it = static_cast<const Job&>(item);
  q->bindValue(index++, it.Name);
  q->bindValue(index++, it.Descr);
  q->bindValue(index++, it.Data);
  q->bindValue(index++, it.Priority);
  q->bindValue(index++, it.Iter);
  q->bindValue(index++, it.IterEnd);
  q->bindValue(index++, it.Done);
  q->bindValue(index++, it.Fail);
  return true;
}


JobTable::JobTable(const Db& _Db)
  : DbTableT<qint64, Job>(_Db)
{
}
