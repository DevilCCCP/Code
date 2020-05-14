#pragma once

#include <Lib/Db/Db.h>

#include "Imp.h"


DefineClassS(SettingsA);

class ImpD: public Imp
{
  const Db&  mDb;

protected:
  const Db& GetDb() { return mDb; }

protected:
  /*new */virtual bool LoadSettings(SettingsA* settings) = 0;

  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;

public:
  ImpD(const Db& _Db, int _WorkPeriodMs, bool _AutoWorkCalc = true, bool _Critical = true);
  /*override */virtual ~ImpD() { }
};

