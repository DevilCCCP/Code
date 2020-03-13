#pragma once

#include <QString>

#include "DbTable.h"


DefineDbClassS(ArmMonitors);

class ArmMonitors: public DbItem
{
public:
  int     Object;
  QString Name;
  QString Descr;
  int     Num;
  int     Width;
  int     Height;
  QPoint  Size;
  bool    Used;

public:
  /*new*/virtual bool Equals(const DbItem& other)
  {
    const ArmMonitors& vs = static_cast<const ArmMonitors&>(other);
    return DbItem::Equals(other) && Object == vs.Object && Name == vs.Name && Descr == vs.Descr
         && Num == vs.Num && Width == vs.Width && Height == vs.Height && Size == vs.Size && Used == vs.Used;
  }

public:
  ArmMonitors(): DbItem(), Object(0) { }
  /*new*/virtual ~ArmMonitors() { }
};

class ArmMonitorsTable: public DbTableT<int, ArmMonitors>
{
protected:
  /*override */virtual QString TableName() Q_DECL_OVERRIDE;
  /*override */virtual QString Columns() Q_DECL_OVERRIDE;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItem>& item) Q_DECL_OVERRIDE;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItem& item) Q_DECL_OVERRIDE;

public:
  ArmMonitorsTable(const Db& _Db);
};
