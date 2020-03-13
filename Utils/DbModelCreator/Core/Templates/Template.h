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
  /*override */virtual bool Equals(const DbItemT<TypeT>& other) Q_DECL_OVERRIDE;

  /*override */virtual qint64 Key(int index) const Q_DECL_OVERRIDE;
  /*override */virtual void SetKey(int index, qint64 id) Q_DECL_OVERRIDE;
  /*override */virtual QString Text(int column) const Q_DECL_OVERRIDE;
  /*override */virtual bool SetText(int column, const QString& text) Q_DECL_OVERRIDE;
  /*override */virtual QVariant Data(int column) const Q_DECL_OVERRIDE;
  /*override */virtual bool SetData(int column, const QVariant& data) Q_DECL_OVERRIDE;

public:
  ClassT(): DbItemT<TypeT>()CTOR_INIT { }
  /*override */virtual ~ClassT() { }
};

class ClassTTable: public DbTableT<TypeT, ClassT>
{NAME_H1
protected:
  /*override */virtual QString TableName() Q_DECL_OVERRIDE;
  /*override */virtual QString Columns() Q_DECL_OVERRIDE;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<TypeT> >& item) Q_DECL_OVERRIDE;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<TypeT>& item) Q_DECL_OVERRIDE;

  /*override */virtual bool CreateDefaultItem(QSharedPointer<DbItemT<TypeT> >& item) Q_DECL_OVERRIDE;
  /*override */virtual void NewDefaultItem(QSharedPointer<DbItemT<TypeT> >& item) Q_DECL_OVERRIDE;

public:
  /*override */virtual QStringList Headers() const Q_DECL_OVERRIDE;
  /*override */virtual QString Icon() const Q_DECL_OVERRIDE;

public:
  ClassTTable(const Db& _Db);
};
