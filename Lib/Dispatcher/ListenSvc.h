#pragma once

#include <Lib/Net/Listener.h>
#include <Lib/Db/Db.h>


DefineClassS(ListenSvc);

class ListenSvc: public Listener
{
  DbS mDb;

public:
  /*override */virtual const char* Name() override { return "ListenerSvc"; }
  /*override */virtual const char* ShortName() override { return "L"; }

protected:
  /*override */virtual bool DoInit() override;

public:
  ListenSvc(int _Port, ChaterManagerS _ChaterManager = ChaterManagerS(new ChaterManager()), DbS _Db = DbS(), int _ConnectionsLimit = 100);
  /*override */virtual ~ListenSvc();
};

