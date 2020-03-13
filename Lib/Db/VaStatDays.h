#pragma once

#include <QString>
#include <QDateTime>
#include <QPoint>
#include <QRect>

#include <Lib/Db/DbTable.h>


DefineDbClassS(VaStatDays);

class VaStatDays: public DbItemT<qint64>
{
public:
  int        VstatId;
  qint64     FimageId;
  QDateTime  Day;

public:
  /*override */virtual bool Equals(const DbItemT<qint64>& other) Q_DECL_OVERRIDE
  {
    const VaStatDays& vs = static_cast<const VaStatDays&>(other);
    return DbItemT<qint64>::Equals(other) && VstatId == vs.VstatId && FimageId == vs.FimageId && Day == vs.Day;
  }

public:
  VaStatDays(): DbItemT<qint64>(), VstatId(0), FimageId(0) { }
  /*override */virtual ~VaStatDays() { }
};

class VaStatDaysTable: public DbTableT<qint64, VaStatDays>
{
protected:
  /*override */virtual QString TableName() Q_DECL_OVERRIDE;
  /*override */virtual QString Columns() Q_DECL_OVERRIDE;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item) Q_DECL_OVERRIDE;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item) Q_DECL_OVERRIDE;

public:
  VaStatDaysTable(const Db& _Db);
};
