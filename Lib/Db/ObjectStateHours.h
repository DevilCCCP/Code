#pragma once

#include <QString>
#include <QDateTime>
#include <QPoint>
#include <QRect>

#include <Lib/Db/DbTable.h>


DefineDbClassS(ObjectStateHours);

class ObjectStateHours: public DbItemT<qint64>
{
public:
  int        ObjectId;
  int        OstypeId;
  int        StateGood;
  int        StateBad;
  QDateTime  TriggeredHour;

public:
  /*override */virtual bool Equals(const DbItemT<qint64>& other) Q_DECL_OVERRIDE
  {
    const ObjectStateHours& vs = static_cast<const ObjectStateHours&>(other);
    return DbItemT<qint64>::Equals(other) && ObjectId == vs.ObjectId && OstypeId == vs.OstypeId && StateGood == vs.StateGood && StateBad == vs.StateBad && TriggeredHour == vs.TriggeredHour;
  }

public:
  ObjectStateHours(): DbItemT<qint64>(), ObjectId(0), OstypeId(0) { }
  /*override */virtual ~ObjectStateHours() { }
};

class ObjectStateHoursTable: public DbTableT<qint64, ObjectStateHours>
{
protected:
  /*override */virtual QString TableName() Q_DECL_OVERRIDE;
  /*override */virtual QString Columns() Q_DECL_OVERRIDE;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item) Q_DECL_OVERRIDE;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item) Q_DECL_OVERRIDE;

public:
  ObjectStateHoursTable(const Db& _Db);
};
