#pragma once

#include <QString>
#include <QDateTime>
#include <QPoint>
#include <QRect>

#include <Lib/Db/DbTable.h>


DefineDbClassS(Job);

class Job: public DbItemT<qint64>
{
public:
  QString    Name;
  QString    Descr;
  QByteArray Data;
  int        Priority;
  int        Iter;
  int        IterEnd;
  int        Done;
  int        Fail;

public:
  /*override */virtual bool Equals(const DbItemT<qint64>& other) Q_DECL_OVERRIDE
  {
    const Job& vs = static_cast<const Job&>(other);
    return DbItemT<qint64>::Equals(other) && Name == vs.Name && Descr == vs.Descr && Data == vs.Data && Priority == vs.Priority && Iter == vs.Iter && IterEnd == vs.IterEnd && Done == vs.Done && Fail == vs.Fail;
  }

public:
  Job(): DbItemT<qint64>() { }
  /*override */virtual ~Job() { }
};

class JobTable: public DbTableT<qint64, Job>
{
protected:
  /*override */virtual QString TableName() Q_DECL_OVERRIDE;
  /*override */virtual QString Columns() Q_DECL_OVERRIDE;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item) Q_DECL_OVERRIDE;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item) Q_DECL_OVERRIDE;

public:
  JobTable(const Db& _Db);
};
