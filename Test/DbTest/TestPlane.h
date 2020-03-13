#pragma once

#include <Lib/Db/Db.h>

#include "Test.h"


class TestPlaneInsert: public Test
{
  DbS     mDb;
  QString mTable;
  int     mColumns;
  int     mCount;
  QVector<QString> mTestData;

public:
  virtual const char* Name() { return "TestPlaneInsert"; }

  virtual bool Prepare();
  virtual bool Do();

public:
  TestPlaneInsert(const QString& _Table, int _Columns, int _Count)
    : mTable(_Table), mColumns(_Columns), mCount(_Count)
  { }
};

class TestPlaneSelect: public Test
{
  DbS     mDb;
  QString mTable;
  int     mColumns;
  int     mCount;

public:
  virtual const char* Name() { return "TestPlaneSelect"; }

  virtual bool Prepare();
  virtual bool Do();

public:
  TestPlaneSelect(const QString& _Table, int _Columns, int _Count)
    : mTable(_Table), mColumns(_Columns), mCount(_Count)
  { }
};

