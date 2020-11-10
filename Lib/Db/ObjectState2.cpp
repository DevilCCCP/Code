#include <QSqlQuery>
#include <QVariant>

#include "ObjectState2.h"


bool ObjectState2::Equals(const DbItemT<int>& other) const
{
  const ObjectState2& vs = static_cast<const ObjectState2&>(other);
  return DbItemT<int>::Equals(other) && ObjectId == vs.ObjectId && OstypeId == vs.OstypeId && State == vs.State && ChangeTime == vs.ChangeTime;
}

qint64 ObjectState2::Key(int index) const
{
  switch (index) {
    case 0: return ObjectId;
    case 1: return OstypeId;
  }
  return 0;
}

void ObjectState2::SetKey(int index, qint64 id)
{
  Q_UNUSED(id);

  switch (index) {
    case 0: ObjectId = id; break;
    case 1: OstypeId = id; break;
  }
}

QString ObjectState2::Text(int column) const
{
  switch (column) {
    case 0: return QString::number(State);
    case 1: return ChangeTime.toString();
  }
  return QString();
}

bool ObjectState2::SetText(int column, const QString& text)
{
  Q_UNUSED(text);

  switch (column) {
    case 0: State = text.toInt(); return true;
    case 1: ChangeTime = QDateTime::fromString(text); return true;
  }
  return false;
}

QVariant ObjectState2::Data(int column) const
{
  switch (column) {
    case 0: return QVariant(State);
    case 1: return QVariant(ChangeTime);
  }
  return QVariant();
}

bool ObjectState2::SetData(int column, const QVariant& data)
{
  Q_UNUSED(data);

  switch (column) {
    case 0: State = data.toInt(); return true;
    case 1: ChangeTime = data.toDateTime(); return true;
  }
  return false;
}

QString ObjectState2Table::TableName()
{
  return "object_state";
}

QString ObjectState2Table::Columns()
{
  return "_object,_ostype,state,change_time";
}

bool ObjectState2Table::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<int> >& item)
{
  ObjectState2* it;
  item.reset(it = new ObjectState2());
  it->ObjectId = q->value(index++).value<int>();
  it->OstypeId = q->value(index++).value<int>();
  it->State = q->value(index++).value<int>();
  it->ChangeTime = q->value(index++).value<QDateTime>();
  return true;
}

bool ObjectState2Table::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<int>& item)
{
  const ObjectState2& it = static_cast<const ObjectState2&>(item);
  q->bindValue(index++, Db::ToKey(it.ObjectId));
  q->bindValue(index++, Db::ToKey(it.OstypeId));
  q->bindValue(index++, it.State);
  q->bindValue(index++, it.ChangeTime);
  return true;
}

bool ObjectState2Table::CreateDefaultItem(QSharedPointer<DbItemT<int> >& item)
{
  item.reset(new ObjectState2());
  QSharedPointer<ObjectState2> it = qSharedPointerCast<ObjectState2>(item);
  if (!Insert(it)) {
    return false;
  }
  return true;
}

void ObjectState2Table::NewDefaultItem(QSharedPointer<DbItemT<int> >& item)
{
  item.reset(new ObjectState2());
}

QStringList ObjectState2Table::Headers() const
{
  return QStringList() << "State" << "Change time";
}

QString ObjectState2Table::Icon() const
{
  return QString(":/Icons/ObjectState.png");
}


ObjectState2Table::ObjectState2Table(const Db& _Db)
  : DbTableT<int, ObjectState2>(_Db)
{
}
