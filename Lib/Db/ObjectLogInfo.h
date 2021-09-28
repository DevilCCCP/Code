#pragma once

#include <QString>
#include <QDateTime>
#include <QPoint>
#include <QRect>

#include <Lib/Db/DbTable.h>


DefineDbClassS(ObjectLogInfo);

class ObjectLogInfo: public DbItemT<qint64>
{
public:
  int        ObjectId;
  QDateTime  HoursStart;
  QDateTime  HoursEnd;
  QDateTime  LastTrunc;
  QDateTime  LastClean;

public:
  /*override */virtual bool Equals(const DbItemT<qint64>& other) const override;

  /*override */virtual qint64 Key(int index) const override;
  /*override */virtual void SetKey(int index, qint64 id) override;
  /*override */virtual QString Text(int column) const override;
  /*override */virtual bool SetText(int column, const QString& text) override;
  /*override */virtual QVariant Data(int column) const override;
  /*override */virtual bool SetData(int column, const QVariant& data) override;

public:
  ObjectLogInfo(): DbItemT<qint64>(), ObjectId(0) { }
  /*override */virtual ~ObjectLogInfo() { }
};

class ObjectLogInfoTable: public DbTableT<qint64, ObjectLogInfo>
{
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
  bool LoadTruncListByType(const QString& objectTypeList, QVector<ObjectLogInfoS>& items);
  bool LoadCleanListByType(const QString& objectTypeList, QVector<ObjectLogInfoS>& items);

public:
  ObjectLogInfoTable(const Db& _Db);
};
