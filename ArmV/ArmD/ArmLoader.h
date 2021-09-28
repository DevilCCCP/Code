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
  /*override */virtual bool DoInit() override;

protected:
  /*override */virtual bool IsNeedIp() override;
  /*override */virtual void RegisterStatic() override;
  /*override */virtual bool InitializeExtra() override;
  /*override */virtual bool CheckUpdateObject() override;
  /*override */virtual bool LoadObject() override;
//  /*override */virtual bool CheckUpdateState() override;
//  /*override */virtual bool UpdateState() override;
//  /*override */virtual EModuleState GetObjectState() override;

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

