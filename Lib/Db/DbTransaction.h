#pragma once

#include <Lib/Include/Common.h>


DefineClassS(DbTransaction);
DefineClassS(Db);

class DbTransaction {
  const Db& mDb;
  bool      mDone;

public:
  bool Commit();
  bool Rollback();

public:
  DbTransaction(const Db& _Db);
  virtual ~DbTransaction();
};
