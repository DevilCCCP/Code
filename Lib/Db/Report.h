#pragma once

#include <QString>
#include <QDateTime>
#include <QPoint>
#include <QRect>

#include <Lib/Db/DbTable.h>


DefineDbClassS(Report);

class Report: public DbItemT<qint64>
{
public:
  int        ObjectId;
  int        Type;
  QDateTime  PeriodBegin;
  QDateTime  PeriodEnd;
  QByteArray Data;

public:
  /*new*/virtual bool Equals(const DbItemT<qint64>& other) const override
  {
    const Report& vs = static_cast<const Report&>(other);
    return DbItemT<qint64>::Equals(other) && ObjectId == vs.ObjectId && Type == vs.Type
        && PeriodBegin == vs.PeriodBegin && PeriodEnd == vs.PeriodEnd && Data == vs.Data;
  }

public:
  Report(): DbItemT<qint64>(), ObjectId(0) { }
  /*new*/virtual ~Report() { }
};

class ReportTable: public DbTableT<qint64, Report>
{
protected:
  /*override */virtual QString TableName() override;
  /*override */virtual QString Columns() override;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item) override;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item) override;

public:
  ReportTable(const Db& _Db);
};
