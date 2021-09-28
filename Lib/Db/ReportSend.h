#pragma once

#include <QString>
#include <QDateTime>
#include <QPoint>
#include <QRect>

#include <Lib/Db/DbTable.h>


DefineDbClassS(ReportSend);

class ReportSend: public DbItemT<qint64>
{
public:
  int       OtoId;
  qint64    LastReportId;
  QDateTime SendTime;

public:
  /*new*/virtual bool Equals(const DbItemT<qint64>& other) const override
  {
    const ReportSend& vs = static_cast<const ReportSend&>(other);
    return DbItemT<qint64>::Equals(other) && OtoId == vs.OtoId && LastReportId == vs.LastReportId && SendTime == vs.SendTime;
  }

public:
  ReportSend(): DbItemT<qint64>(), OtoId(0), LastReportId(0), SendTime(QDateTime::fromMSecsSinceEpoch(0)) { }
  /*new*/virtual ~ReportSend() { }
};

class ReportSendTable: public DbTableT<qint64, ReportSend>
{
protected:
  /*override */virtual QString TableName() override;
  /*override */virtual QString Columns() override;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item) override;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item) override;

public:
  ReportSendTable(const Db& _Db);
};
