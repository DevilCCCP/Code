#include <Lib/Log/Log.h>
#include <Lib/Dispatcher/Overseer.h>

#include "ListenSvc.h"


bool ListenSvc::DoInit()
{
  Overseer* overseer = dynamic_cast<Overseer*>(GetManager());
  if (!overseer) {
    Log.Warning("Using 'ListenSvc' not as Imp");
  }

  bool result = Listener::DoInit();
  if (ListenFail()) {
    if (!mDb) {
      mDb = DbS(new Db());
      if (!mDb->OpenDefault()) {
        Log.Fatal("Db open fail", true);
      }
    }

    if (overseer) {
      if (mDb->Connect()) {
        auto q = mDb->MakeQuery();
        q->prepare(QString("UPDATE object SET uri = substring(uri from 1 for 5), revision = revision + 1 WHERE _id = %1;"
                           " UPDATE object o SET revision = revision + 1 FROM object_connection c"
                           " WHERE c._oslave = %1 AND o._id = c._omaster;")
                   .arg(overseer->Id()));
        if (mDb->ExecuteQuery(q)) {
          Log.Info("Object uri cleared");
        } else {
          Log.Error("Object uri clear fail");
        }
      }
    }
  }

  return result;
}


ListenSvc::ListenSvc(int _Port, ChaterManagerS _ChaterManager, DbS _Db, int _ConnectionsLimit)
  : Listener(_Port, _ChaterManager, _ConnectionsLimit)
  , mDb(_Db)
{
}

ListenSvc::~ListenSvc()
{
}


