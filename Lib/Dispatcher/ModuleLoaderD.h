#pragma once

#include <QElapsedTimer>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QSet>

#include <Lib/Db/Db.h>

#include "Dispatcher.h"


DefineClassS(DbSettings);
DefineClassS(ModuleDb);
DefineClassS(ModuleStatic);

enum EModuleState {
  eUnknownState  = 0,
  eNormalState   = 1 << 0,
  eStandByState  = 1 << 1,
  eAllState      = 0xff
};

inline QString EModuleStateToString(EModuleState state) {
  switch (state) {
  case eUnknownState : return "Unknown";
  case eNormalState  : return "Normal";
  case eStandByState : return "StandBy";
  case eAllState     : return "All";
  }
  return QString("Unknown state %1").arg((int)state);
}

class ModuleLoaderD : public ModuleLoaderB
{
  const QString           mTypeName;
  PROPERTY_GET_SET(bool,  VeryQuiet)
  PROPERTY_GET_SET(int,   RestartPostgresMs)

  PROTECTED_SGET(Db)
  PROTECTED_SGET(DbSettings)
  PROTECTED_GET(DbSettingsS, DbVariables)
  PROTECTED_SGET(ObjectTypeTable)
  PROTECTED_SGET(ObjectTable)
  PROTECTED_SGET(ObjectState)

  QElapsedTimer           mInitializeTimer;
  bool                    mInitialized;
  EModuleState            mCurrentState;
  ObjectItemS             mMainModule;
  int                     mCurrentRevision;
  QMap<int, ModuleDbS>    mModuleDbMap;
  QList<ModuleStaticS>    mModuleStaticList;
  bool                    mDbUpdated;

  QSet<int>               mUsedStaticModules;
  QSet<int>               mInsertModules;
  QSet<int>               mUpdateModules;
  QSet<int>               mRemoveModules;
  QSet<int>               mDisableModules;
  QSet<int>               mUpdatePortModules;

  bool                    mDbConnected;
  QString                 mHostIpAddress;
  QString                 mSettingIpAddress;
  int                     mTcpPortBase;
  QMap<int, bool>         mTcpPortsFree;
  QElapsedTimer           mIpTestTimer;
  bool                    mLicenseInfo;

  qint64                  mNextQuietCheckUpdate;

protected:
  const QString& IpAddress() const      { return mHostIpAddress; }
  const ObjectItemS& MainObject() const { return mMainModule; }

protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual bool UpdateModules() override;
  /*override */virtual void DoRelease() override;

protected:
  /*new */virtual bool IsNeedIp();
  /*new */virtual void RegisterStatic();
  /*new */virtual bool InitializeExtra();
  /*new */virtual bool CheckUpdateObject();
  /*new */virtual bool CheckOffline();
  /*new */virtual bool LoadObject() = 0;
  /*new */virtual bool CheckUpdateState();
  /*new */virtual bool UpdateState();
  /*new */virtual EModuleState GetObjectState();

protected:
  bool GetObjectTypeId(const QString& typeName, int* id);
  void RegisterStaticModule(const QString& path, const QStringList& params, int stateFlag);
  void LoadModule(const ModuleDbS& module);
  void ReloadModule(const ModuleDbS& module);

private:
  bool DoInitialize();
  void InitStaticModules();

  void ReloadObject();
  void UpdateObject();

  void SwitchState(EModuleState state);
  void ReloadIp();
  void PrepareLoad();
  void CommitUpdate();
  void UpdateObjectState();

  bool Connect();
  bool DbUpdate();
  bool RegisterModule();
  bool LoadLicense();
  bool LoadSettings();
  bool CheckIpChange();
  bool IsHostIpValid();
  bool LoadHostIp();
  bool LoadHostIpEqual(bool& changed);
  bool LoadHostIpSubnet(bool& changed);
  bool LoadHostIpAnyIpv4(bool& changed);
  bool LoadHostIpDbConnection(bool& changed);

  void ProvideTcpPorts();
  int GetNextFreePort();
  void FreeTcpPort(int port);

public:
  ModuleLoaderD(SettingsAS& _Settings, const QString& _TypeName, int _TcpPortBase);
};

