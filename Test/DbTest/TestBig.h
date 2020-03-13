#pragma once

#include <QString>

#include <Lib/Db/DbTable.h>


DefineDbClassS(TestBig);

class TestBig: public DbItemB
{
public:
  QString mData[16];

public:
  /*override */virtual bool Equals(const DbItemB& other)
  {
    const TestBig& vs = static_cast<const TestBig&>(other);
    if (!DbItemB::Equals(other)) {
      return false;
    }

    for (int i = 0; i < 16; i++) {
      if (mData[i] != vs.mData[i]) {
        return false;
      }
    }
    return true;
  }

public:
  TestBig(): DbItemB() { }
  /*override */virtual ~TestBig() { }
};

class TestBigTable: public DbTableT<qint64, TestBig>
{
protected:
  /*override */virtual QString TableName() Q_DECL_OVERRIDE;
  /*override */virtual QString Columns() Q_DECL_OVERRIDE;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemB>& item) Q_DECL_OVERRIDE;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemB& item) Q_DECL_OVERRIDE;

public:
  int TotalSize();

public:
  TestBigTable(const DbS& _Db);
};
