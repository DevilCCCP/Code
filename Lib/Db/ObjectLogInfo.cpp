#include <QSqlQuery>
#include <QVariant>

#include "ObjectLogInfo.h"


bool ObjectLogInfo::Equals(const DbItemT<qint64>& other) const
{
  const ObjectLogInfo& vs = static_cast<const ObjectLogInfo&>(other);
  return DbItemT<qint64>::Equals(other) && ObjectId == vs.ObjectId && HoursStart == vs.HoursStart && HoursEnd == vs.HoursEnd && LastTrunc == vs.LastTrunc && LastClean == vs.LastClean;
}

qint64 ObjectLogInfo::Key(int index) const
{
  switch (index) {
    case 0: return ObjectId;
  }
  return 0;
}

void ObjectLogInfo::SetKey(int index, qint64 id)
{
  Q_UNUSED(id);

  switch (index) {
    case 0: ObjectId = id; break;
  }
}

QString ObjectLogInfo::Text(int column) const
{
  switch (column) {
    case 0: return HoursStart.toString();
    case 1: return HoursEnd.toString();
    case 2: return LastTrunc.toString();
    case 3: return LastClean.toString();
  }
  return QString();
}

bool ObjectLogInfo::SetText(int column, const QString& text)
{
  Q_UNUSED(text);

  switch (column) {
    case 0: HoursStart = QDateTime::fromString(text); return true;
    case 1: HoursEnd = QDateTime::fromString(text); return true;
    case 2: LastTrunc = QDateTime::fromString(text); return true;
    case 3: LastClean = QDateTime::fromString(text); return true;
  }
  return false;
}

QVariant ObjectLogInfo::Data(int column) const
{
  switch (column) {
    case 0: return QVariant(HoursStart);
    case 1: return QVariant(HoursEnd);
    case 2: return QVariant(LastTrunc);
    case 3: return QVariant(LastClean);
  }
  return QVariant();
}

bool ObjectLogInfo::SetData(int column, const QVariant& data)
{
  Q_UNUSED(data);

  switch (column) {
    case 0: HoursStart = data.toDateTime(); return true;
    case 1: HoursEnd = data.toDateTime(); return true;
    case 2: LastTrunc = data.toDateTime(); return true;
    case 3: LastClean = data.toDateTime(); return true;
  }
  return false;
}

QString ObjectLogInfoTable::TableName()
{
  return "object_log_info";
}

QString ObjectLogInfoTable::Columns()
{
  return "_object,hours_start,hours_end,last_trunc,last_clean";
}

bool ObjectLogInfoTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item)
{
  ObjectLogInfo* it;
  item.reset(it = new ObjectLogInfo());
  it->ObjectId = q->value(index++).value<int>();
  it->HoursStart = q->value(index++).value<QDateTime>();
  it->HoursEnd = q->value(index++).value<QDateTime>();
  it->LastTrunc = q->value(index++).value<QDateTime>();
  it->LastClean = q->value(index++).value<QDateTime>();
  return true;
}

bool ObjectLogInfoTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item)
{
  const ObjectLogInfo& it = static_cast<const ObjectLogInfo&>(item);
  q->bindValue(index++, Db::ToKey(it.ObjectId));
  q->bindValue(index++, it.HoursStart);
  q->bindValue(index++, it.HoursEnd);
  q->bindValue(index++, it.LastTrunc);
  q->bindValue(index++, it.LastClean);
  return true;
}

bool ObjectLogInfoTable::CreateDefaultItem(QSharedPointer<DbItemT<qint64> >& item)
{
  item.reset(new ObjectLogInfo());
  QSharedPointer<ObjectLogInfo> it = qSharedPointerCast<ObjectLogInfo>(item);
  if (!Insert(it)) {
    return false;
  }
  return true;
}

void ObjectLogInfoTable::NewDefaultItem(QSharedPointer<DbItemT<qint64> >& item)
{
  item.reset(new ObjectLogInfo());
}

QStringList ObjectLogInfoTable::Headers() const
{
  return QStringList() << "Hours start" << "Hours end" << "Last trunc" << "Last clean";
}

QString ObjectLogInfoTable::Icon() const
{
  return QString(":/Icons/ObjectLogInfo.png");
}


bool ObjectLogInfoTable::LoadTruncListByType(const QString& objectTypeList, QVector<ObjectLogInfoS>& items)
{
  return SelectJoin("li", "JOIN object o ON o._id = li._object"
                    , QString("WHERE o._otype IN (%1)"
                              " ORDER BY last_trunc NULLS FIRST, _object").arg(objectTypeList)
                    , items);
}

bool ObjectLogInfoTable::LoadCleanListByType(const QString& objectTypeList, QVector<ObjectLogInfoS>& items)
{
  return SelectJoin("li", "JOIN object o ON o._id = li._object"
                    , QString("WHERE o._otype IN (%1)"
                              " ORDER BY last_clean NULLS FIRST, _object").arg(objectTypeList)
                    , items);
}


ObjectLogInfoTable::ObjectLogInfoTable(const Db& _Db)
  : DbTableT<qint64, ObjectLogInfo>(_Db)
{
}
