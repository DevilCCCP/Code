#include <Lib/Log/Log.h>

#include "ObjectSchedule.h"
#include "DbSettings.h"


bool ObjectSchedule::LoadSchedule()
{
  DbSettings settings(*mDb);
  settings.SetSilent(true);
  if (!settings.Open(QString::number(mId))) {
    return false;
  }

  if (Load(&settings)) {
    Log.Info(QString("Schedule loaded (id: %1, period: '%2'").arg(mId).arg(Dump()));
    return true;
  }
  return false;
}


ObjectSchedule::ObjectSchedule(Db* _Db, int _Id)
  : mDb(_Db), mId(_Id)
{
}
