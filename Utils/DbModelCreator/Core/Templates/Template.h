#pragma once

#include <QString>
#include <QDateTime>
#include <QPoint>
#include <QRect>

#include <Lib/Db/DbTable.h>


DefineDbClassS(ClassT);

class ClassT: public DbItemT<TypeT>
{
public:
DECLARE
public:
  /*override */virtual bool Equals(const DbItemT<TypeT>& other) const override;

  /*override */virtual qint64 Key(int index) const override;
  /*override */virtual void SetKey(int index, qint64 id) override;
  /*override */virtual QString Text(int column) const override;
  /*override */virtual bool SetText(int column, const QString& text) override;
  /*override */virtual QVariant Data(int column) const override;
  /*override */virtual bool SetData(int column, const QVariant& data) override;

public:
  ClassT(): DbItemT<TypeT>()CTOR_INIT { }
  /*override */virtual ~ClassT() { }
};

class ClassTTable: public DbTableT<TypeT, ClassT>
{NAME_H1
protected:
  /*override */virtual QString TableName() override;
  /*override */virtual QString Columns() override;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<TypeT> >& item) override;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<TypeT>& item) override;

  /*override */virtual bool CreateDefaultItem(QSharedPointer<DbItemT<TypeT> >& item) override;
  /*override */virtual void NewDefaultItem(QSharedPointer<DbItemT<TypeT> >& item) override;

public:
  /*override */virtual QStringList Headers() const override;
  /*override */virtual QString Icon() const override;

public:
  ClassTTable(const Db& _Db);
};
