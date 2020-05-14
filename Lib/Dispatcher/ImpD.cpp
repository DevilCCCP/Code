#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Settings/DbSettings.h>
#include <Lib/Log/Log.h>

#include "ImpD.h"


bool ImpD::DoInit()
{
  DbSettings settings(mDb);
  if (!settings.Open(QString::number(GetOverseer()->Id()))) {
    Log.Fatal("Can't get module settings");
    GetOverseer()->Restart();
    return false;
  }

  if (!LoadSettings(&settings)) {
    Log.Fatal("Load module settings fail");
    GetOverseer()->Restart();
    return false;
  }

  return true;
}


ImpD::ImpD(const Db& _Db, int _WorkPeriodMs, bool _AutoWorkCalc, bool _Critical)
  : Imp(_WorkPeriodMs, _AutoWorkCalc, _Critical)
  , mDb(_Db)
{
  mDb.MoveToThread(this);
}
