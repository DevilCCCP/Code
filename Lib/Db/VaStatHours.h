#pragma once

#include <QString>
#include <QDateTime>
#include <QPoint>
#include <QRect>

#include <Lib/Db/DbTable.h>


DefineDbClassS(VaStatHours);

class VaStatHours: public DbItemT<qint64>
{
public:
  int        VstatId;
  qint64     FimageId;
  QDateTime  Hour;

public:
  /*override */virtual bool Equals(const DbItemT<qint64>& other) const override
  {
    const VaStatHours& vs = static_cast<const VaStatHours&>(other);
    return DbItemT<qint64>::Equals(other) && VstatId == vs.VstatId && FimageId == vs.FimageId && Hour == vs.Hour;
  }

public:
  VaStatHours(): DbItemT<qint64>(), VstatId(0), FimageId(0) { }
  /*override */virtual ~VaStatHours() { }
};

class VaStatHoursTable: public DbTableT<qint64, VaStatHours>
{
protected:
  /*override */virtual QString TableName() override;
  /*override */virtual QString Columns() override;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item) override;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item) override;

public:
  VaStatHoursTable(const Db& _Db);
};
