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
  /*override */virtual bool Equals(const DbItemT<int>& other) const Q_DECL_OVERRIDE;

  /*override */virtual qint64 Key(int index) const Q_DECL_OVERRIDE;
  /*override */virtual void SetKey(int index, qint64 id) Q_DECL_OVERRIDE;
  /*override */virtual QString Text(int column) const Q_DECL_OVERRIDE;
  /*override */virtual bool SetText(int column, const QString& text) Q_DECL_OVERRIDE;
  /*override */virtual QVariant Data(int column) const Q_DECL_OVERRIDE;
  /*override */virtual bool SetData(int column, const QVariant& data) Q_DECL_OVERRIDE;

public:
  ObjectState2(): DbItemT<int>(), ObjectId(0), OstypeId(0) { }
  /*override */virtual ~ObjectState2() { }
};

class ObjectState2Table: public DbTableT<int, ObjectState2>
{
protected:
  /*override */virtual QString TableName() Q_DECL_OVERRIDE;
  /*override */virtual QString Columns() Q_DECL_OVERRIDE;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<int> >& item) Q_DECL_OVERRIDE;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<int>& item) Q_DECL_OVERRIDE;

  /*override */virtual bool CreateDefaultItem(QSharedPointer<DbItemT<int> >& item) Q_DECL_OVERRIDE;
  /*override */virtual void NewDefaultItem(QSharedPointer<DbItemT<int> >& item) Q_DECL_OVERRIDE;

public:
  /*override */virtual QStringList Headers() const Q_DECL_OVERRIDE;
  /*override */virtual QString Icon() const Q_DECL_OVERRIDE;

public:
  ObjectState2Table(const Db& _Db);
};
