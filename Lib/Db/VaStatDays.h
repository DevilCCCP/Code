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
  /*override */virtual bool Equals(const DbItemT<qint64>& other) const override
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
  /*override */virtual QString TableName() override;
  /*override */virtual QString Columns() override;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item) override;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item) override;

public:
  VaStatDaysTable(const Db& _Db);
};
