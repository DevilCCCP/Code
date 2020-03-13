#pragma once

#include <QString>
#include <QList>

#include <Lib/Db/ObjectLog.h>


struct LogUnitSchema {
  int     ObjectTypeId;
  QString ThreadName;
  QString WorkName;
  QString ViewName;

  LogUnitSchema(int _ObjectTypeId, const QString& _ThreadName, const QString& _WorkName, const QString& _ViewName):
    ObjectTypeId(_ObjectTypeId), ThreadName(_ThreadName), WorkName(_WorkName), ViewName(_ViewName)
  { }
};
typedef QList<LogUnitSchema> LogSchema;

struct LogObjectSchema {
  int     ObjectId;
  QString ThreadName;
  QString WorkName;
  QString ViewName;

  LogObjectSchema(int _ObjectId, const QString& _ThreadName, const QString& _WorkName, const QString& _ViewName):
    ObjectId(_ObjectId), ThreadName(_ThreadName), WorkName(_WorkName), ViewName(_ViewName)
  { }
};
typedef QList<LogObjectSchema> ObjectSchema;

typedef QVector<ObjectLogS> LogPeriod;
typedef QMap<QString, LogPeriod> WorkLogPeriodMap;
typedef QMap<QString, WorkLogPeriodMap> ThreadLogPeriodMap;
typedef QMap<int, ThreadLogPeriodMap> ObjectLogPeriodMap;
