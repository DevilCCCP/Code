#pragma once

#include <QSharedMemory>

#include <Lib/Dispatcher/ModuleLoaderD.h>
#include <LibV/Player/ArmState.h>


class ArmLoader: public ModuleLoaderD
{
  int           mSrvTypeId;
  int           mCamTypeId;
  int           mCam2TypeId;

  QSharedMemory mStateShmem;
  ArmState      mArmState;
  volatile int  mLayoutCounter;
  int           mRevision;

protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;

protected:
  /*override */virtual bool IsNeedIp() Q_DECL_OVERRIDE;
  /*override */virtual void RegisterStatic() Q_DECL_OVERRIDE;
  /*override */virtual bool InitializeExtra() Q_DECL_OVERRIDE;
  /*override */virtual bool CheckUpdateObject() Q_DECL_OVERRIDE;
  /*override */virtual bool LoadObject() Q_DECL_OVERRIDE;
//  /*override */virtual bool CheckUpdateState() Q_DECL_OVERRIDE;
//  /*override */virtual bool UpdateState() Q_DECL_OVERRIDE;
//  /*override */virtual EModuleState GetObjectState() Q_DECL_OVERRIDE;

private:
  bool LoadDbArm(int id);
  bool LoadUserArm();
  bool LoadUserCamera();

  void UpdateLayout();
  void ClearAllLayoutModules();
  void RegisterAllLayoutModules();

public:
  ArmLoader(SettingsAS& _Settings);
};

