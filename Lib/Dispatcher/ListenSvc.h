#pragma once

#include <Lib/Net/Listener.h>
#include <Lib/Db/Db.h>


DefineClassS(ListenSvc);

class ListenSvc: public Listener
{
  DbS mDb;

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "ListenerSvc"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "L"; }

protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;

public:
  ListenSvc(int _Port, ChaterManagerS _ChaterManager = ChaterManagerS(new ChaterManager()), DbS _Db = DbS(), int _ConnectionsLimit = 100);
  /*override */virtual ~ListenSvc();
};

