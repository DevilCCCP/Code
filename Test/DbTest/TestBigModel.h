#pragma once

#include <Lib/Db/Db.h>

#include "Test.h"
#include "TestBig.h"


class TestBigModel: public Test
{
  DbS     mDb;
  QString mTable;
  int     mColumns;
  int     mCount;
  TestBigTableS mTestBigtable;

public:
  virtual const char* Name() { return "TestBigModel"; }

  virtual bool Prepare();
  virtual bool Do();

public:
  TestBigModel(const QString& _Table, int _Columns, int _Count)
    : mTable(_Table), mColumns(_Columns), mCount(_Count)
  { }
};

class TestBigModelIns: public Test
{
  DbS     mDb;
  QString mTable;
  int     mColumns;
  int     mCount;
  TestBigTableS mTestBigtable;
  QVector<QString> mTestData;

public:
  virtual const char* Name() { return "TestBigModelIns"; }

  virtual bool Prepare();
  virtual bool Do();

public:
  TestBigModelIns(const QString& _Table, int _Columns, int _Count)
    : mTable(_Table), mColumns(_Columns), mCount(_Count)
  { }
};
