#pragma once

#include <QString>
#include <QDateTime>
#include <QPoint>
#include <QRect>

#include <Lib/Db/DbTable.h>


DefineDbClassS(ObjectLog);

class ObjectLog: public DbItemT<qint64>
{
public:
  int        ObjectId;
  QDateTime  PeriodStart;
  QDateTime  PeriodEnd;
  QString    ThreadName;
  QString    WorkName;
  int        TotalTime;
  int        Circles;
  int        WorkTime;
  int        LongestWork;

public:
  /*override */virtual bool Equals(const DbItemT<qint64>& other) const Q_DECL_OVERRIDE;

  /*override */virtual qint64 Key(int index) const Q_DECL_OVERRIDE;
  /*override */virtual void SetKey(int index, qint64 id) Q_DECL_OVERRIDE;
  /*override */virtual QString Text(int column) const Q_DECL_OVERRIDE;
  /*override */virtual bool SetText(int column, const QString& text) Q_DECL_OVERRIDE;
  /*override */virtual QVariant Data(int column) const Q_DECL_OVERRIDE;
  /*override */virtual bool SetData(int column, const QVariant& data) Q_DECL_OVERRIDE;

public:
  ObjectLog(): DbItemT<qint64>(), ObjectId(0) { }
  /*override */virtual ~ObjectLog() { }
};

class ObjectLogTable: public DbTableT<qint64, ObjectLog>
{
protected:
  /*override */virtual QString TableName() Q_DECL_OVERRIDE;
  /*override */virtual QString Columns() Q_DECL_OVERRIDE;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item) Q_DECL_OVERRIDE;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item) Q_DECL_OVERRIDE;

  /*override */virtual bool CreateDefaultItem(QSharedPointer<DbItemT<qint64> >& item) Q_DECL_OVERRIDE;
  /*override */virtual void NewDefaultItem(QSharedPointer<DbItemT<qint64> >& item) Q_DECL_OVERRIDE;

public:
  /*override */virtual QStringList Headers() const Q_DECL_OVERRIDE;
  /*override */virtual QString Icon() const Q_DECL_OVERRIDE;

public:
  bool TruncHours(int objectId, int hour);

public:
  ObjectLogTable(const Db& _Db);
};
