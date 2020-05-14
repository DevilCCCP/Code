#include <QSqlQuery>
#include <QVariant>

#include "ObjectLog.h"


bool ObjectLog::Equals(const DbItemT<qint64>& other) const
{
  const ObjectLog& vs = static_cast<const ObjectLog&>(other);
  return DbItemT<qint64>::Equals(other) && ObjectId == vs.ObjectId && PeriodStart == vs.PeriodStart && PeriodEnd == vs.PeriodEnd && ThreadName == vs.ThreadName && WorkName == vs.WorkName && TotalTime == vs.TotalTime && Circles == vs.Circles && WorkTime == vs.WorkTime && LongestWork == vs.LongestWork;
}

qint64 ObjectLog::Key(int index) const
{
  switch (index) {
    case 0: return ObjectId;
  }
  return 0;
}

void ObjectLog::SetKey(int index, qint64 id)
{
  Q_UNUSED(id);

  switch (index) {
    case 0: ObjectId = id; break;
  }
}

QString ObjectLog::Text(int column) const
{
  switch (column) {
    case 0: return PeriodStart.toString();
    case 1: return PeriodEnd.toString();
    case 2: return ThreadName;
    case 3: return WorkName;
    case 4: return QString::number(TotalTime);
    case 5: return QString::number(Circles);
    case 6: return QString::number(WorkTime);
    case 7: return QString::number(LongestWork);
  }
  return QString();
}

bool ObjectLog::SetText(int column, const QString& text)
{
  Q_UNUSED(text);

  switch (column) {
    case 0: PeriodStart = QDateTime::fromString(text); return true;
    case 1: PeriodEnd = QDateTime::fromString(text); return true;
    case 2: ThreadName = text; return true;
    case 3: WorkName = text; return true;
    case 4: TotalTime = text.toInt(); return true;
    case 5: Circles = text.toInt(); return true;
    case 6: WorkTime = text.toInt(); return true;
    case 7: LongestWork = text.toInt(); return true;
  }
  return false;
}

QVariant ObjectLog::Data(int column) const
{
  switch (column) {
    case 0: return QVariant(PeriodStart);
    case 1: return QVariant(PeriodEnd);
    case 2: return QVariant(ThreadName);
    case 3: return QVariant(WorkName);
    case 4: return QVariant(TotalTime);
    case 5: return QVariant(Circles);
    case 6: return QVariant(WorkTime);
    case 7: return QVariant(LongestWork);
  }
  return QVariant();
}

bool ObjectLog::SetData(int column, const QVariant& data)
{
  Q_UNUSED(data);

  switch (column) {
    case 0: PeriodStart = data.toDateTime(); return true;
    case 1: PeriodEnd = data.toDateTime(); return true;
    case 2: ThreadName = data.toString(); return true;
    case 3: WorkName = data.toString(); return true;
    case 4: TotalTime = data.toInt(); return true;
    case 5: Circles = data.toInt(); return true;
    case 6: WorkTime = data.toInt(); return true;
    case 7: LongestWork = data.toInt(); return true;
  }
  return false;
}

QString ObjectLogTable::TableName()
{
  return "object_log";
}

QString ObjectLogTable::Columns()
{
  return "_object,period_start,period_end,thread_name,work_name,total_time,circles,work_time,longest_work";
}

bool ObjectLogTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item)
{
  ObjectLog* it;
  item.reset(it = new ObjectLog());
  it->ObjectId = q->value(index++).value<int>();
  it->PeriodStart = q->value(index++).value<QDateTime>();
  it->PeriodEnd = q->value(index++).value<QDateTime>();
  it->ThreadName = q->value(index++).value<QString>();
  it->WorkName = q->value(index++).value<QString>();
  it->TotalTime = q->value(index++).value<int>();
  it->Circles = q->value(index++).value<int>();
  it->WorkTime = q->value(index++).value<int>();
  it->LongestWork = q->value(index++).value<int>();
  return true;
}

bool ObjectLogTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item)
{
  const ObjectLog& it = static_cast<const ObjectLog&>(item);
  q->bindValue(index++, Db::ToKey(it.ObjectId));
  q->bindValue(index++, it.PeriodStart);
  q->bindValue(index++, it.PeriodEnd);
  q->bindValue(index++, it.ThreadName);
  q->bindValue(index++, it.WorkName);
  q->bindValue(index++, it.TotalTime);
  q->bindValue(index++, it.Circles);
  q->bindValue(index++, it.WorkTime);
  q->bindValue(index++, it.LongestWork);
  return true;
}

bool ObjectLogTable::CreateDefaultItem(QSharedPointer<DbItemT<qint64> >& item)
{
  item.reset(new ObjectLog());
  QSharedPointer<ObjectLog> it = qSharedPointerCast<ObjectLog>(item);
  if (!Insert(it)) {
    return false;
  }
  return true;
}

void ObjectLogTable::NewDefaultItem(QSharedPointer<DbItemT<qint64> >& item)
{
  item.reset(new ObjectLog());
}

QStringList ObjectLogTable::Headers() const
{
  return QStringList() << "Period start" << "Period end" << "Thread name" << "Work name" << "Total time" << "Circles" << "Work time" << "Longest work";
}

QString ObjectLogTable::Icon() const
{
  return QString(":/Icons/ObjectLog.png");
}

bool ObjectLogTable::TruncHours(int objectId, int hour)
{
  auto q = getDb().MakeQuery();
  q->prepare(QString("SELECT object_log_trunc_hours(?, ?);"));
  int index = 0;
  q->bindValue(index++, objectId);
  q->bindValue(index++, hour);
  return getDb().ExecuteNonQuery(q);
}


ObjectLogTable::ObjectLogTable(const Db& _Db)
  : DbTableT<qint64, ObjectLog>(_Db)
{
}
