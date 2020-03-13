#include <Lib/Log/Log.h>

#include "DbTransaction.h"
#include "Db.h"


bool DbTransaction::Commit()
{
  Log.Trace(QString("Commit transaction"));
  mDone = mDb.CommitTransaction();
  return mDone;
}

bool DbTransaction::Rollback()
{
  Log.Trace(QString("Rollback transaction"));
  mDone = mDb.RollbackTransaction();
  return mDone;
}


DbTransaction::DbTransaction(const Db& _Db)
  : mDb(_Db), mDone(false)
{
  Log.Trace(QString("Begin transaction"));
}

DbTransaction::~DbTransaction()
{
  if (!mDone) {
    if (!Rollback()) {
      Log.Warning(QString("rollback transaction fail"));
    }
  }
}
