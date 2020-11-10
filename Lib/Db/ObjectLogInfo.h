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
  /*override */virtual bool Equals(const DbItemT<qint64>& other) const Q_DECL_OVERRIDE;

  /*override */virtual qint64 Key(int index) const Q_DECL_OVERRIDE;
  /*override */virtual void SetKey(int index, qint64 id) Q_DECL_OVERRIDE;
  /*override */virtual QString Text(int column) const Q_DECL_OVERRIDE;
  /*override */virtual bool SetText(int column, const QString& text) Q_DECL_OVERRIDE;
  /*override */virtual QVariant Data(int column) const Q_DECL_OVERRIDE;
  /*override */virtual bool SetData(int column, const QVariant& data) Q_DECL_OVERRIDE;

public:
  ObjectLogInfo(): DbItemT<qint64>(), ObjectId(0) { }
  /*override */virtual ~ObjectLogInfo() { }
};

class ObjectLogInfoTable: public DbTableT<qint64, ObjectLogInfo>
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
  bool LoadTruncListByType(const QString& objectTypeList, QVector<ObjectLogInfoS>& items);
  bool LoadCleanListByType(const QString& objectTypeList, QVector<ObjectLogInfoS>& items);

public:
  ObjectLogInfoTable(const Db& _Db);
};
