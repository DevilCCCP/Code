#pragma once

#include <QString>
#include <QDateTime>
#include <QPoint>
#include <QRect>

#include <Lib/Db/DbTable.h>


DefineDbClassS(ObjectState2);

class ObjectState2: public DbItemT<int>
{
public:
  int        ObjectId;
  int        OstypeId;
  int        State;
  QDateTime  ChangeTime;

public:
  /*override */virtual bool Equals(const DbItemT<int>& other) const override;

  /*override */virtual qint64 Key(int index) const override;
  /*override */virtual void SetKey(int index, qint64 id) override;
  /*override */virtual QString Text(int column) const override;
  /*override */virtual bool SetText(int column, const QString& text) override;
  /*override */virtual QVariant Data(int column) const override;
  /*override */virtual bool SetData(int column, const QVariant& data) override;

public:
  ObjectState2(): DbItemT<int>(), ObjectId(0), OstypeId(0) { }
  /*override */virtual ~ObjectState2() { }
};

class ObjectState2Table: public DbTableT<int, ObjectState2>
{
protected:
  /*override */virtual QString TableName() override;
  /*override */virtual QString Columns() override;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<int> >& item) override;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<int>& item) override;

  /*override */virtual bool CreateDefaultItem(QSharedPointer<DbItemT<int> >& item) override;
  /*override */virtual void NewDefaultItem(QSharedPointer<DbItemT<int> >& item) override;

public:
  /*override */virtual QStringList Headers() const override;
  /*override */virtual QString Icon() const override;

public:
  ObjectState2Table(const Db& _Db);
};
