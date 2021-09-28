#pragma once

#include <QString>
#include <QDateTime>
#include <QPoint>
#include <QRect>

#include <Lib/Db/DbTable.h>


DefineDbClassS(TestBig);

class TestBig: public DbItemT<int>
{
public:
  QString    BigData[16];

public:
  /*override */virtual bool Equals(const DbItemT<int>& other) const Q_DECL_OVERRIDE;

  /*override */virtual qint64 Key(int index) const Q_DECL_OVERRIDE;
  /*override */virtual void SetKey(int index, qint64 id) Q_DECL_OVERRIDE;
  /*override */virtual QString Text(int column) const Q_DECL_OVERRIDE;
  /*override */virtual bool SetText(int column, const QString& text) Q_DECL_OVERRIDE;
  /*override */virtual QVariant Data(int column) const Q_DECL_OVERRIDE;
  /*override */virtual bool SetData(int column, const QVariant& data) Q_DECL_OVERRIDE;

public:
  TestBig(): DbItemT<int>() { }
  /*override */virtual ~TestBig() { }
};

class TestBigTable: public DbTableT<int, TestBig>
{
protected:
  /*override */virtual QString TableName() override;
  /*override */virtual QString Columns() override;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemB>& item) override;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemB& item) override;

  /*override */virtual bool CreateDefaultItem(QSharedPointer<DbItemT<int> >& item) Q_DECL_OVERRIDE;
  /*override */virtual void NewDefaultItem(QSharedPointer<DbItemT<int> >& item) Q_DECL_OVERRIDE;

public:
  /*override */virtual QStringList Headers() const Q_DECL_OVERRIDE;
  /*override */virtual QString Icon() const Q_DECL_OVERRIDE;

public:
  TestBigTable(const Db& _Db);
};
