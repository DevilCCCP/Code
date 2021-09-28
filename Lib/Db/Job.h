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
  bool       IsActive;
  int        Priority;
  qint64     Iter;
  qint64     IterEnd;
  qint64     Done;
  qint64     Fail;
  QDateTime  ActiveTime;

public:
  /*override */virtual bool Equals(const DbItemT<qint64>& other) const override;

  /*override */virtual qint64 Key(int index) const override;
  /*override */virtual void SetKey(int index, qint64 id) override;
  /*override */virtual QString Text(int column) const override;
  /*override */virtual bool SetText(int column, const QString& text) override;
  /*override */virtual QVariant Data(int column) const override;
  /*override */virtual bool SetData(int column, const QVariant& data) override;

public:
  Job(): DbItemT<qint64>(), IsActive(true), Priority(0), Iter(0), IterEnd(0), Done(0), Fail(0) { }
  /*override */virtual ~Job() { }
};

class JobTable: public DbTableT<qint64, Job>
{
  qint64 mCounter;

protected:
  /*override */virtual QString TableName() override;
  /*override */virtual QString Columns() override;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item) override;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item) override;

  /*override */virtual bool CreateDefaultItem(QSharedPointer<DbItemT<qint64> >& item) override;
  /*override */virtual void NewDefaultItem(QSharedPointer<DbItemT<qint64> >& item) override;

public:
  /*override */virtual QStringList Headers() const override;
  /*override */virtual QString Icon() const override;

public:
  JobTable(const Db& _Db);
};
