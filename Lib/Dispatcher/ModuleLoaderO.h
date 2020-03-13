#pragma once

#include <QElapsedTimer>

#include "ModuleLoaderD.h"


DefineClassS(ObjectSchedule);

class ModuleLoaderO: public ModuleLoaderD
{
  struct ObjectSchemeItem {
    QString     TypeAbbr;
    QString     ModulePath;
    QStringList ModuleParams;
    int         State;
    int         TypeId;
  };
  typedef QMap<int, ObjectSchemeItem> ObjectScheme;

  DefineStructS(ScheduleInfo);
  struct ScheduleInfo {
    ModuleDbS       Module;
    int             SchId;
    int             SchRev;
    ObjectScheduleS ModuleSchedule;

    bool IsEqual(const ScheduleInfo& other) { return SchId == other.SchId && SchRev == other.SchRev; }
  };
  typedef QMap<int, ScheduleInfoS> ScheduleInfoMap;

  ObjectScheme         mObjectScheme;
  int                  mScheduleTypeId;

  ScheduleInfoS        mMainSchedule;
  QList<ScheduleInfoS> mUpdateSchedules;
  ScheduleInfoMap      mObjectSchedules;
  QElapsedTimer        mResheduleTimer;
  qint64               mNextReshedule;

protected:
  /*new */virtual void SetDbScheme() = 0;

  /*override */virtual bool InitializeExtra() Q_DECL_OVERRIDE;
  /*override */virtual bool LoadObject() Q_DECL_OVERRIDE;
  /*override */virtual bool CheckUpdateState() Q_DECL_OVERRIDE;
  /*override */virtual bool UpdateState() Q_DECL_OVERRIDE;
  /*override */virtual EModuleState GetObjectState() Q_DECL_OVERRIDE;

protected:
  void RegisterDbModule(const QString& abbr, const QString& path, int state, QStringList params = QStringList());

private:
  bool UpdateModuleSchedule(int id, const ScheduleInfoS& schedule);
  void CalcNextReshedule();

public:
  ModuleLoaderO(SettingsAS& _Settings, const QString& _TypeName, int _TcpPortBase);
};
