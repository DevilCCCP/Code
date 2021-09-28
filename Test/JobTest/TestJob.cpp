#include <QSqlQuery>
#include <QVariant>

#include "TestJob.h"


bool TestJob::Equals(const DbItemT<qint64>& other) const
{
  const TestJob& vs = static_cast<const TestJob&>(other);
  return DbItemT<qint64>::Equals(other) && Name == vs.Name && Descr == vs.Descr && IsActive == vs.IsActive && Priority == vs.Priority && Iter == vs.Iter && IterEnd == vs.IterEnd && Done == vs.Done && Fail == vs.Fail && ActiveTime == vs.ActiveTime && Circles == vs.Circles;
}

qint64 TestJob::Key(int index) const
{
  switch (index) {

  }
  return 0;
}

void TestJob::SetKey(int index, qint64 id)
{
  Q_UNUSED(id);

  switch (index) {

  }
}

QString TestJob::Text(int column) const
{
  switch (column) {
    case 0: return Name;
    case 1: return Descr;
    case 2: return IsActive? QString("true"): QString("false");
    case 3: return QString::number(Priority);
    case 4: return QString::number(Iter);
    case 5: return QString::number(IterEnd);
    case 6: return QString::number(Done);
    case 7: return QString::number(Fail);
    case 8: return ActiveTime.toString();
    case 9: return QString::number(Circles);
  }
  return QString();
}

bool TestJob::SetText(int column, const QString& text)
{
  Q_UNUSED(text);

  switch (column) {
    case 0: Name = text; return true;
    case 1: Descr = text; return true;
    case 2: IsActive = (text.toLower() == "true" || text == "1"); return true;
    case 3: Priority = text.toInt(); return true;
    case 4: Iter = text.toLongLong(); return true;
    case 5: IterEnd = text.toLongLong(); return true;
    case 6: Done = text.toLongLong(); return true;
    case 7: Fail = text.toLongLong(); return true;
    case 8: ActiveTime = QDateTime::fromString(text); return true;
    case 9: Circles = text.toInt(); return true;
  }
  return false;
}

QVariant TestJob::Data(int column) const
{
  switch (column) {
    case 0: return QVariant(Name);
    case 1: return QVariant(Descr);
    case 2: return QVariant(IsActive);
    case 3: return QVariant(Priority);
    case 4: return QVariant(Iter);
    case 5: return QVariant(IterEnd);
    case 6: return QVariant(Done);
    case 7: return QVariant(Fail);
    case 8: return QVariant(ActiveTime);
    case 9: return QVariant(Circles);
  }
  return QVariant();
}

bool TestJob::SetData(int column, const QVariant& data)
{
  Q_UNUSED(data);

  switch (column) {
    case 0: Name = data.toString(); return true;
    case 1: Descr = data.toString(); return true;
    case 2: IsActive = data.toBool(); return true;
    case 3: Priority = data.toInt(); return true;
    case 4: Iter = data.toLongLong(); return true;
    case 5: IterEnd = data.toLongLong(); return true;
    case 6: Done = data.toLongLong(); return true;
    case 7: Fail = data.toLongLong(); return true;
    case 8: ActiveTime = data.toDateTime(); return true;
    case 9: Circles = data.toInt(); return true;
  }
  return false;
}

QString TestJobTable::TableName()
{
  return "test_job";
}

QString TestJobTable::Columns()
{
  return "name,descr,is_active,priority,iter,iter_end,done,fail,active_time,circles";
}

bool TestJobTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item)
{
  TestJob* it;
  item.reset(it = new TestJob());
  it->Name = q->value(index++).value<QString>();
  it->Descr = q->value(index++).value<QString>();
  it->IsActive = q->value(index++).value<bool>();
  it->Priority = q->value(index++).value<int>();
  it->Iter = q->value(index++).value<qint64>();
  it->IterEnd = q->value(index++).value<qint64>();
  it->Done = q->value(index++).value<qint64>();
  it->Fail = q->value(index++).value<qint64>();
  it->ActiveTime = q->value(index++).value<QDateTime>();
  it->Circles = q->value(index++).value<int>();
  return true;
}

bool TestJobTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item)
{
  const TestJob& it = static_cast<const TestJob&>(item);
  q->bindValue(index++, it.Name);
  q->bindValue(index++, it.Descr);
  q->bindValue(index++, it.IsActive);
  q->bindValue(index++, it.Priority);
  q->bindValue(index++, it.Iter);
  q->bindValue(index++, it.IterEnd);
  q->bindValue(index++, it.Done);
  q->bindValue(index++, it.Fail);
  q->bindValue(index++, it.ActiveTime);
  q->bindValue(index++, it.Circles);
  return true;
}

bool TestJobTable::CreateDefaultItem(QSharedPointer<DbItemT<qint64> >& item)
{
  item.reset(new TestJob());
  QSharedPointer<TestJob> it = qSharedPointerCast<TestJob>(item);
  if (mCounter < 0) {
    if (!SelectCount("", mCounter)) {
      return false;
    }
  }
  ++mCounter;
  it->Name = QString("TestJob %1").arg(mCounter);

  if (!Insert(it)) {
    return false;
  }
  return true;
}

void TestJobTable::NewDefaultItem(QSharedPointer<DbItemT<qint64> >& item)
{
  item.reset(new TestJob());
}

QStringList TestJobTable::Headers() const
{
  return QStringList() << "Name" << "Descr" << "Is active" << "Priority" << "Iter" << "Iter end" << "Done" << "Fail" << "Active time" << "Circles";
}

QString TestJobTable::Icon() const
{
  return QString(":/Icons/TestJob.png");
}


TestJobTable::TestJobTable(const Db& _Db)
  : DbTableT<qint64, TestJob>(_Db)
  , mCounter(-1)
{
}
