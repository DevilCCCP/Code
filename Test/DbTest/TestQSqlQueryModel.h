#pragma once

#include <QSqlQueryModel>

#include <Lib/Db/Db.h>

#include "Test.h"


class TestQSqlQueryModel: public Test
{
  DbS     mDb;
  QString mTable;
  int     mColumns;
  int     mCount;
  QSqlQueryModel mModel;

public:
  virtual const char* Name() { return "TestQSqlQueryModel"; }

  virtual bool Prepare();
  virtual bool Do();

public:
  TestQSqlQueryModel(const QString& _Table, int _Columns, int _Count)
    : mTable(_Table), mColumns(_Columns), mCount(_Count)
  { }
};
