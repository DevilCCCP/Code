#pragma once

#include <Lib/Db/Db.h>


DefineClassS(Core);

class Core
{
  Db&           mDb;

  PROPERTY_GET(int,     ServerTypeId)
  PROPERTY_GET(QString, ServerTypeName)
  PROPERTY_GET(int,     ScheduleTypeId)

  bool                  mLoadError;

public:
  Db& GetDb() { return mDb; }
  const ObjectTableS& getObjectTable() { return mDb.getObjectTable(); }
  const ObjectTypeTableS& getObjectTypeTable() { return mDb.getObjectTypeTable(); }
  const ObjectStateTableS& getObjectStateTable() { return mDb.getObjectStateTable(); }
  const ObjectStateValuesTableS& getObjectStateValuesTable() { return mDb.getObjectStateValuesTable(); }
  const EventTypeTableS& getEventTypeTable() { return mDb.getEventTypeTable(); }
  const EventTableS& getEventTable() { return mDb.getEventTable(); }
  const FilesTableS& getFilesTable() { return mDb.getFilesTable(); }

public:
  bool ReloadSchema();

private:
  bool GetTypeId(const QString& abbr, int& id, QString* name = nullptr);

public:
  explicit Core(Db& _Db);
};

