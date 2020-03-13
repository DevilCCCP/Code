#pragma once

#include <QString>

#include "DbTable.h"


DefineDbClassS(MonitorLayouts);

class MonitorLayouts: public DbItem
{
public:
  int        Monitor;
  QRect      Place;
//  int        Flag;
  QList<int> Cameras;
  bool       CameraLoaded;

public:
  /*override */virtual bool Equals(const DbItem& other) Q_DECL_OVERRIDE
  {
    const MonitorLayouts& vs = static_cast<const MonitorLayouts&>(other);
    return DbItem::Equals(other) && Monitor == vs.Monitor && Place == vs.Place;// && Flag == vs.Flag;
  }

public:
  MonitorLayouts(): DbItem(), Monitor(0), CameraLoaded(false) { }
  /*override */virtual ~MonitorLayouts() { }
};

class MonitorLayoutsTable: public DbTableT<int, MonitorLayouts>
{
protected:
  /*override */virtual QString TableName() Q_DECL_OVERRIDE;
  /*override */virtual QString Columns() Q_DECL_OVERRIDE;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItem>& item) Q_DECL_OVERRIDE;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItem& item) Q_DECL_OVERRIDE;

public:
  MonitorLayoutsTable(const Db& _Db);
};
