#include <Lib/Log/Log.h>
#include <Lib/Settings/ObjectSchedule.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Common/Format.h>

#include "ModuleLoaderO.h"
#include "ModuleDb.h"


bool ModuleLoaderO::InitializeExtra()
{
  if (!GetObjectTypeId("sch", &mScheduleTypeId)) {
    Log.Warning(QString("Can't load schedule type id"));
    mScheduleTypeId = -1;
  }

  SetDbScheme();
  return true;
}

bool ModuleLoaderO::LoadObject()
{
  auto q = getDb()->MakeQuery();
  q->prepare(QString("WITH sh AS (SELECT _id FROM object WHERE _otype = %2)"
                     " SELECT o._otype, o.status, o._id, o.revision, o.uri, o2._id, o2.revision FROM object_connection c"
                     " INNER JOIN object o ON o._id = c._oslave"
                     " LEFT JOIN object_connection c2 ON c2._omaster = o._id AND c2._oslave IN (SELECT _id FROM sh)"
                     " LEFT JOIN object o2 ON o2._id = c2._oslave"
                     " WHERE c._omaster = %1;").arg(MainObject()->Id).arg(mScheduleTypeId));
  if (!getDb()->ExecuteQuery(q)) {
    return false;
  }

  QList<int> lastSchedulesList = mObjectSchedules.keys();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14,0)
  QSet<int> lastSchedules(lastSchedulesList.begin(), lastSchedulesList.end());
#else
  QSet<int> lastSchedules = QSet<int>::fromList(lastSchedulesList);
#endif
  mMainSchedule.clear();
  mUpdateSchedules.clear();
  while (q->next()) {
    int type = q->value(0).toInt();
    int state = q->value(1).toInt();
    if (type == mScheduleTypeId) {
      ScheduleInfoS newSch(new ScheduleInfo());
      newSch->SchId = q->value(2).toInt();
      newSch->SchRev = q->value(3).toInt();
      if (UpdateModuleSchedule(MainObject()->Id, newSch)) {
        mMainSchedule = newSch;
      }
      continue;
    }

    for (auto itr = mObjectScheme.find(type); itr != mObjectScheme.end() && itr.key() == type; itr++) {
      const ObjectSchemeItem& objModule = itr.value();
      if (state == objModule.State) {
        int id = q->value(2).toInt();
        ModuleDbS module(new ModuleDb());
        ScheduleInfoS newSch(new ScheduleInfo());
        module->setState(state);
        module->setId(id);
        module->setPath(objModule.ModulePath);
        module->setParams(objModule.ModuleParams);
        module->setRevision(q->value(3).toInt());
        module->setUri(q->value(4).toString());
        newSch->Module = module;
        newSch->SchId = q->value(5).toInt();
        newSch->SchRev = q->value(6).toInt();
        lastSchedules.remove(id);

        if (!UpdateModuleSchedule(id, newSch)) {
          LoadModule(module);
        }
      }
    }
  }

  for (auto itr = mUpdateSchedules.begin(); itr != mUpdateSchedules.end(); itr++) {
    const ScheduleInfoS& newSch = *itr;
    if (newSch->SchId) {
      newSch->ModuleSchedule.reset(new ObjectSchedule(getDb().data(), newSch->SchId));
      if (!newSch->ModuleSchedule->LoadSchedule()) {
        return false;
      }
      if (newSch->Module) {
        mObjectSchedules[newSch->Module->getId()] = newSch;
        newSch->Module->setActive(newSch->ModuleSchedule->IsActive());
      }
    } else {
      if (newSch->Module) {
        mObjectSchedules.remove(newSch->Module->getId());
        newSch->Module->setActive(true);
      }
    }
    if (newSch->Module) {
      LoadModule(newSch->Module);
    }
  }
  if (!mMainSchedule) {
    mObjectSchedules.remove(MainObject()->Id);
  }
  mUpdateSchedules.clear();

  for (auto itr = lastSchedules.begin(); itr != lastSchedules.end(); itr++) {
    mObjectSchedules.remove(*itr);
  }
  CalcNextReshedule();
  return true;
}

bool ModuleLoaderO::CheckUpdateState()
{
  return mResheduleTimer.elapsed() > mNextReshedule;
}

bool ModuleLoaderO::UpdateState()
{
  for (auto itr = mObjectSchedules.begin(); itr != mObjectSchedules.end(); itr++) {
    const ScheduleInfoS& schedule = itr.value();
    if (schedule->Module) {
      bool active = schedule->ModuleSchedule? schedule->ModuleSchedule->IsActive(): true;
      if (active != schedule->Module->getActive()) {
        schedule->Module->setActive(active);
        ReloadModule(schedule->Module);
      }
    }
  }

  CalcNextReshedule();
  return true;
}

EModuleState ModuleLoaderO::GetObjectState()
{
  if (mMainSchedule && mMainSchedule->ModuleSchedule) {
    return mMainSchedule->ModuleSchedule->IsActive()? eNormalState: eStandByState;
  }
  return eNormalState;
}

void ModuleLoaderO::RegisterDbModule(const QString& abbr, const QString& path, int state, QStringList params)
{
  int id;
  if (!GetObjectTypeId(abbr, &id)) {
    Log.Error(QString("Register module type fail (abbr: '%1')").arg(abbr));
    return;
  }

  ObjectSchemeItem module;
  module.TypeAbbr     = abbr;
  module.ModulePath   = path;
  module.ModuleParams = params;
  module.State        = state;
  module.TypeId       = id;
  mObjectScheme[id] = module;
}

bool ModuleLoaderO::UpdateModuleSchedule(int id, const ScheduleInfoS& schedule)
{
  bool update = true;
  auto itr = mObjectSchedules.find(id);
  if (itr != mObjectSchedules.end()) {
    const ScheduleInfoS& oldSchedule = itr.value();
    if (!oldSchedule->IsEqual(*schedule) || !oldSchedule->ModuleSchedule) {
      mUpdateSchedules.append(schedule);
      return true;
    } else {
      if (schedule->Module) {
        schedule->Module->setActive(oldSchedule->ModuleSchedule->IsActive());
        update = false;
      }
    }
  } else if (schedule->SchId) {
    mUpdateSchedules.append(schedule);
    return true;
  }
  if (update && schedule->Module) {
    schedule->Module->setActive(true);
  }
  return false;
}

void ModuleLoaderO::CalcNextReshedule()
{
  mNextReshedule = Schedule::MaxNextUpdate();

  for (auto itr = mObjectSchedules.begin(); itr != mObjectSchedules.end(); itr++) {
    const ScheduleInfoS& schedule = itr.value();
    if (schedule->ModuleSchedule) {
      mNextReshedule = qMin(mNextReshedule, schedule->ModuleSchedule->NextUpdateMs());
    }
  }
  Log.Info(QString("Next schedule refresh in %1").arg(FormatTime(mNextReshedule)));
  mResheduleTimer.start();
}


ModuleLoaderO::ModuleLoaderO(SettingsAS& _Settings, const QString& _TypeName, int _TcpPortBase)
  : ModuleLoaderD(_Settings, _TypeName, _TcpPortBase)
  , mNextReshedule(0)
{
}
