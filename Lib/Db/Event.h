#pragma once

#include <QMultiMap>
#include <QDateTime>
#include <QElapsedTimer>

#include "TableNamed.h"

DefineDbClassS(EventType);
DefineDbClassS(Event);


// ================================================
// event type
// --
class EventType: public NamedItem
{
public:
  QString Icon;

public:
  /*override*/virtual bool Equals(const TableItem &other) const override;

public:
  EventType() { }
  /*override*/virtual ~EventType() { }
};

class EventTypeTable: public TableNamed
{
protected:
  /*override */virtual const char* Name() override;
  /*override*/virtual const char* Select() override;
  /*override*/virtual bool OnRowFillItem(QueryS& q, TableItemS& unit) override;

public:
  EventTypeTable(const Db& _Db);
  /*override*/virtual ~EventTypeTable();
};

struct EventLog {
  qint64    Id;
  int       EventId;
  QDateTime TriggeredTime;
};

// ================================================
// event
// --
class Event: public TableItem
{
public:
  int       ObjectId;
  int       EventTypeId;

public:
  /*override*/virtual bool Equals(const TableItem &other) const override;

public:
  Event() { }
  /*override*/virtual ~Event() { }
};

class EventTable: public Table
{
  QMultiMap<int, int>        mObjectTypes;

  QMap<int, QMap<int, int> > mInitedEvents;

public:
  QList<int> GetObjects() { return mObjectTypes.uniqueKeys(); }
  QList<int> GetObjectEventTypes(int id) { return mObjectTypes.values(id); }

protected:
  /*override */virtual const char* Name() override;
  /*override */virtual const char* Select() override;
  /*override */virtual const char* Insert() override;
  /*override */virtual bool OnRowFillItem(QueryS& q, TableItemS& unit) override;
  /*override */virtual bool OnSetItem(QueryS& q, const TableItem& unit) override;
  /*override */virtual void CreateIndexes() override;
  /*override */virtual void ClearIndexes() override;

public:
  bool InitEvent(int detectorId, int eventTypeId, int* id);
  bool TriggerEvent(int eventId, const QDateTime& timestamp, qreal value = 1, qint64* id = nullptr);
  bool TriggerEvent(int eventId, const qint64& fileId, const QDateTime& timestamp, qreal value = 1, qint64* id = nullptr);
  bool TriggerEvents(const QVariantList& eventIds, const QVariantList& timestamps, const QVariantList& values);
  bool TriggerEvents(const QVariantList& eventIds, const QVariantList& timestamps, const QVariantList& values, const QString& doneQuery);
  bool InitStat(int detectorId, int eventTypeId, int* id = nullptr);
  bool StatEvent(int statId, const QDateTime& timestamp, qreal value, int periodMs);
  bool CancelEvent(const qint64& id);

  bool CreateEvent(const QString& objUuid, const QString& eventName, int* id);
  bool ImportEvent(int objectId, int eventId, const QDateTime& timestamp, int value, const QByteArray& data, int uniId, const QString& segUuid, const qint64& topEvent, qint64* id = nullptr);

  bool GetFirstEventTs(QDateTime& timestamp);
  bool GetFirstStateTs(QDateTime& timestamp);
  bool LoadEventLog(qint64 minId, QList<EventLog>& events);

  bool RemoveEventLogByPeriod(const qint64& objectId, const QDateTime& timeTo);
  bool RemoveEventLogByCount(const qint64& objectId, int count);
  bool RemoveEventLogBySize(const qint64& objectId, const qint64 size);

  bool RemoveAllEventLogByPeriod(const QDateTime& timeTo);
  bool RemoveAllEventLogByCount(int count);
  bool RemoveAllEventLogFilesBySize(const qint64 size);

public:
  EventTable(const Db& _Db);
  /*override*/virtual ~EventTable();
};
