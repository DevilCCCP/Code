#include "Event.h"

#include <Lib/Log/Log.h>


bool EventType::Equals(const TableItem& other) const
{
  const EventType& other_ = dynamic_cast<const EventType&>(other);
  return Id == other_.Id && Name == other_.Name && Descr == other_.Descr && Icon == other_.Icon;
}


const char* EventTypeTable::Name()
{
  return "event_type";
}

const char* EventTypeTable::Select()
{
  return "SELECT _id, name, descr, icon FROM event_type ";
}

bool EventTypeTable::OnRowFillItem(QueryS& q, TableItemS& unit)
{
  unit = TableItemS(new EventType());
  EventType* item = static_cast<EventType*>(unit.data());
  item->Icon = q->value(3).toString();
  return TableNamed::OnRowFillItem(q, unit);
}


EventTypeTable::EventTypeTable(const Db& _Db)
  : TableNamed(_Db)
{
}

EventTypeTable::~EventTypeTable()
{
}


bool Event::Equals(const TableItem& other) const
{
  const Event& other_ = dynamic_cast<const Event&>(other);
  return Id == other_.Id && ObjectId == other_.ObjectId && EventTypeId == other_.EventTypeId;
}


const char* EventTable::Name()
{
  return "event";
}

const char* EventTable::Select()
{
  return "SELECT _id, _object, _etype FROM event";
}

const char* EventTable::Insert()
{
  return "INSERT INTO event (_object, _etype) VALUES (?, ?)";
}

bool EventTable::OnRowFillItem(QueryS& q, TableItemS& unit)
{
  unit = TableItemS(new Event());
  Event* item = static_cast<Event*>(unit.data());
  item->ObjectId = q->value(1).toInt();
  item->EventTypeId = q->value(2).toInt();
  return true;
}

bool EventTable::OnSetItem(QueryS& q, const TableItem& unit)
{
  const Event& item = static_cast<const Event&>(unit);
  q->addBindValue(item.ObjectId);
  q->addBindValue(item.EventTypeId);
  return true;
}

void EventTable::CreateIndexes()
{
  for (auto itr = mItems.begin(); itr != mItems.end(); itr++) {
    const Event* event = static_cast<const Event*>(itr.value().data());
    mObjectTypes.insert(event->ObjectId, event->EventTypeId);
  }
}

void EventTable::ClearIndexes()
{
  mObjectTypes.clear();
}

bool EventTable::InitEvent(int detectorId, int eventTypeId, int* id)
{
  auto itr = mInitedEvents.find(detectorId);
  if (itr != mInitedEvents.end()) {
    auto itr2 = itr->find(eventTypeId);
    if (itr2 != itr->end()) {
      *id = itr2.value();
      return true;
    }
  }

  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT event_init(%1, %2);").arg(detectorId).arg(eventTypeId));
  if (mDb.ExecuteQuery(q) && q->next() && q->value(0).isValid()) {
    bool ok;
    int evId = q->value(0).toInt(&ok);
    mInitedEvents[detectorId][eventTypeId] = evId;
    if (id) {
      *id = evId;
    }
    return ok;
  }
  return false;
}

bool EventTable::TriggerEvent(int eventId, const QDateTime& timestamp, qreal value, qint64* id)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("INSERT INTO event_log(_event, triggered_time, value) VALUES (?, ?, ?) RETURNING _id;"));
  int index = 0;
  q->bindValue(index++, eventId);
  q->bindValue(index++, timestamp);
  q->bindValue(index++, value);
  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return false;
  }

  if (id) {
    *id = q->value(0).toLongLong();
  }
  return true;
}

bool EventTable::TriggerEvent(int eventId, const qint64& fileId, const QDateTime& timestamp, qreal value, qint64* id)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("INSERT INTO event_log(_event, _file, triggered_time, value) VALUES (?, ?, ?, ?) RETURNING _id;"));
  int index = 0;
  q->bindValue(index++, eventId);
  q->bindValue(index++, fileId);
  q->bindValue(index++, timestamp);
  q->bindValue(index++, value);
  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return false;
  }

  if (id) {
    *id = q->value(0).toLongLong();
  }
  return true;
}

bool EventTable::TriggerEvents(const QVariantList& eventIds, const QVariantList& timestamps, const QVariantList& values)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("INSERT INTO event_log(_event, triggered_time, value) VALUES (?, ?, ?)"));
  q->addBindValue(eventIds);
  q->addBindValue(timestamps);
  q->addBindValue(values);
  return mDb.ExecuteBatch(q);
}

bool EventTable::TriggerEvents(const QVariantList& eventIds, const QVariantList& timestamps, const QVariantList& values, const QString& doneQuery)
{
  auto q = mDb.MakeQuery();
  QString query = QString("INSERT INTO event_log (_event, triggered_time, value) VALUES \n");
  QStringList valuesList;
  for (int i = 0; i < eventIds.size(); i++) {
    valuesList.append("(?, ?, ?)");
  }
  query.append(valuesList.join(",\n"));
  query.append(QString(";\n") + doneQuery);

  q->prepare(query);
  for (int i = 0; i < eventIds.size(); i++) {
    q->addBindValue(eventIds[i]);
    q->addBindValue(timestamps[i]);
    q->addBindValue(values[i]);
  }
  return mDb.ExecuteNonQuery(q);
}

bool EventTable::InitStat(int detectorId, int eventTypeId, int* id)
{
  auto itr = mInitedEvents.find(detectorId);
  if (itr != mInitedEvents.end()) {
    auto itr2 = itr->find(eventTypeId);
    if (itr2 != itr->end()) {
      *id = itr2.value();
      return true;
    }
  }

  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT event_stat_init(%1, %2);").arg(detectorId).arg(eventTypeId));
  if (mDb.ExecuteQuery(q) && q->next() && q->value(0).isValid()) {
    bool ok;
    int evStatId = q->value(0).toInt(&ok);
    mInitedEvents[detectorId][eventTypeId] = evStatId;
    if (id) {
      *id = evStatId;
    }
    return ok;
  }
  return false;
}

bool EventTable::StatEvent(int statId, const QDateTime& timestamp, qreal value, int periodMs)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("UPDATE event_stat SET triggered_time=?, value=?, period=? WHERE _id = %1;").arg(statId));
  int index = 0;
  q->bindValue(index++, timestamp);
  q->bindValue(index++, value);
  q->bindValue(index++, periodMs);
  return mDb.ExecuteQuery(q);
}

bool EventTable::CancelEvent(const qint64& id)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("DELETE FROM event_log WHERE _id = %1;").arg(id));
  return mDb.ExecuteNonQuery(q);
}

bool EventTable::CreateEvent(const QString& objUuid, const QString& eventName, int* id)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT e._id, o._id, t._id FROM object o"
                     " INNER JOIN event_type t ON t.name = ?"
                     " LEFT JOIN event e ON e._object = o._id AND e._etype = t._id"
                     " WHERE o.guid = ?;"));
  q->addBindValue(eventName);
  q->addBindValue(objUuid);
  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return false;
  }

  int eventId = q->value(0).toInt();
  int objectId = q->value(1).toInt();
  int eventTypeId = q->value(2).toInt();
  q.clear();
  if (!eventId) {
    Event* event;
    TableItemS item(event = new Event());
    event->ObjectId = objectId;
    event->EventTypeId = eventTypeId;
    if (!InsertItem(item)) {
      return false;
    }
    eventId = item->Id;
  }

  if (id) {
    *id = eventId;
  }
  return true;
}

bool EventTable::ImportEvent(int objectId, int eventId, const QDateTime& timestamp, int value, const QByteArray& data, int uniId, const QString& segUuid, const qint64& topEvent, qint64* id)
{
  auto q = mDb.MakeQuery();
  q->prepare("SELECT event_import(?,?,?,?,?,?,?,?);");
  q->addBindValue(objectId);
  q->addBindValue(eventId);
  q->addBindValue(timestamp);
  q->addBindValue(value);
  q->addBindValue(data);
  q->addBindValue(uniId);
  q->addBindValue(segUuid);
  q->addBindValue(topEvent);
  if (!mDb.ExecuteNonQuery(q)) {
    return false;
  }
  if (!q->next()) {
    return false;
  }
  if (id) {
    *id = q->value(0).toLongLong();
  }
  return true;
}

bool EventTable::GetFirstEventTs(QDateTime& timestamp)
{
  auto q = mDb.MakeQuery();
  q->prepare("SELECT triggered_time FROM event_log ORDER BY triggered_time ASC LIMIT 1;");
  if (!mDb.ExecuteNonQuery(q)) {
    return false;
  }
  if (q->next()) {
    timestamp = q->value(0).toDateTime();
  } else {
    timestamp = QDateTime();
  }
  return true;
}

bool EventTable::GetFirstStateTs(QDateTime& timestamp)
{
  auto q = mDb.MakeQuery();
  q->prepare("SELECT triggered_hour FROM event_stat_hours ORDER BY triggered_hour ASC LIMIT 1;");
  if (!mDb.ExecuteNonQuery(q)) {
    return false;
  }
  if (q->next()) {
    timestamp = q->value(0).toDateTime();
  } else {
    timestamp = QDateTime();
  }
  return true;
}

bool EventTable::LoadEventLog(qint64 minId, QList<EventLog>& events)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT _id, _event, triggered_time"
                     " FROM event_log"
                     " WHERE _id > %1"
                     " ORDER BY _id;").arg(minId));
  if (!mDb.ExecuteNonQuery(q)) {
    return false;
  }
  while (q->next()) {
    EventLog log;
    log.Id            = q->value(0).toLongLong();
    log.EventId       = q->value(1).toInt();
    log.TriggeredTime = q->value(2).toDateTime();
    events.append(log);
  }
  return true;
}

bool EventTable::RemoveEventLogByPeriod(const qint64& objectId, const QDateTime& timeTo)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("WITH rm_file AS ("
                     " DELETE FROM event_log"
                     " WHERE _id IN ("
                     " SELECT l._id FROM event_log l"
                     " JOIN event e ON e._id = l._event"
                     " WHERE e._object = %1 AND e._id = l._event AND l.triggered_time < ?)"
                     " RETURNING _file)"
                     " DELETE FROM files USING rm_file WHERE _id = _file;").arg(objectId));
  q->addBindValue(timeTo);
  return mDb.ExecuteNonQuery(q);
}

bool EventTable::RemoveEventLogByCount(const qint64& objectId, int count)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT COUNT(l._id) FROM event_log l"
                     " JOIN event e ON e._id = l._event"
                     " WHERE e._object = %1;").arg(objectId));
  if (!mDb.ExecuteNonQuery(q) || !q->next()) {
    return false;
  }
  int hasCount = q->value(0).toInt();
  if (hasCount <= count) {
    return true;
  }

  int rmCount = hasCount - count;
  q->prepare(QString("WITH rm_file AS ("
                     " DELETE FROM event_log"
                     " WHERE _id IN ("
                     " SELECT l._id FROM event_log l"
                     " JOIN event e ON e._id = l._event"
                     " WHERE e._object = %1"
                     " ORDER BY l._id"
                     " LIMIT %2) RETURNING _file)"
                     " DELETE FROM files USING rm_file WHERE _id = _file;").arg(objectId).arg(rmCount));
  return mDb.ExecuteNonQuery(q);
}

bool EventTable::RemoveEventLogBySize(const qint64& objectId, const qint64 size)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT COUNT(l._id), SUM(OCTET_LENGTH(f.data)) FROM event_log l"
                     " JOIN event e ON e._id = l._event"
                     " JOIN files f ON f._id = l._file"
                     " WHERE e._object = %1;").arg(objectId));
  if (!mDb.ExecuteNonQuery(q) || !q->next()) {
    return false;
  }
  int     hasCount = q->value(0).toInt();
  qint64 totalSize = q->value(1).toLongLong();
  if (totalSize <= size) {
    return true;
  }

  qint64 rmSize = totalSize - size;
  int rmCount = (int)(hasCount * rmSize / size) + 1;
  q->prepare(QString("WITH rm_file AS ("
                     " DELETE FROM event_log"
                     " WHERE _id IN ("
                     " SELECT l._id FROM event_log l"
                     " JOIN event e ON e._id = l._event"
                     " WHERE e._object = %1"
                     " ORDER BY l._id"
                     " LIMIT %2) RETURNING _file)"
                     " DELETE FROM files USING rm_file WHERE _id = _file;").arg(objectId).arg(rmCount));
  return mDb.ExecuteNonQuery(q);
}

bool EventTable::RemoveAllEventLogByPeriod(const QDateTime& timeTo)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("WITH rm_file AS ("
                     " DELETE FROM event_log"
                     " WHERE _id IN ("
                     " SELECT _id FROM event_log"
                     " WHERE triggered_time < ?)"
                     " RETURNING _file)"
                     " DELETE FROM files USING rm_file WHERE _id = _file;"));
  q->addBindValue(timeTo);
  return mDb.ExecuteNonQuery(q);
}

bool EventTable::RemoveAllEventLogByCount(int count)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT COUNT(l._id) FROM event_log l"));
  if (!mDb.ExecuteNonQuery(q) || !q->next()) {
    return false;
  }
  int hasCount = q->value(0).toInt();
  if (hasCount <= count) {
    return true;
  }

  int rmCount = hasCount - count;
  q->prepare(QString("WITH rm_file AS ("
                     " DELETE FROM event_log"
                     " WHERE _id IN ("
                     " SELECT l._id FROM event_log l"
                     " ORDER BY l._id"
                     " LIMIT %1) RETURNING _file)"
                     " DELETE FROM files USING rm_file WHERE _id = _file;").arg(rmCount));
  return mDb.ExecuteNonQuery(q);
}

bool EventTable::RemoveAllEventLogFilesBySize(const qint64 size)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT COUNT(l._id), SUM(OCTET_LENGTH(f.data)) FROM event_log l"
                     " JOIN files f ON f._id = l._file"));
  if (!mDb.ExecuteNonQuery(q) || !q->next()) {
    return false;
  }
  int     hasCount = q->value(0).toInt();
  qint64 totalSize = q->value(1).toLongLong();
  if (totalSize <= size) {
    return true;
  }

  qint64 rmSize = totalSize - size;
  int rmCount = (int)(hasCount * rmSize / size) + 1;
  q->prepare(QString("WITH rm_file AS ("
                     " SELECT _file FROM event_log"
                     " ORDER BY _id"
                     " LIMIT %2)"
                     " DELETE FROM files USING rm_file WHERE _id = _file;").arg(rmCount));
  return mDb.ExecuteNonQuery(q);
}


EventTable::EventTable(const Db& _Db)
  : Table(_Db)
{
}

EventTable::~EventTable()
{
}
