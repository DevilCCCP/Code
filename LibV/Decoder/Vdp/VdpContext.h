#pragma once

#include <QString>
#include <vdpau/vdpau_x11.h>

#include "VdpDef.h"


class VdpContext
{
  Display*  mDisplay;
  int       mScreen;

  VdpDevice mVdpDevice;
  QString   mErrorText;

  VdpGetProcAddress* mVdpGetProcAddress;

public:
  enum Profiles {
    eProfileMPEG2Main = 1,
    eProfileH264Main  = 2,
    eProfileH264High  = 4,
    eProfileVC1Main   = 8,
    eProfileMPEG4ASP  = 16
  };

public:
  const VdpDevice& Device() const { return mVdpDevice; }
#define UseVdpFunctionOne(VdpId, VdpFunc) VdpFunc* func##VdpFunc;
  UseVdpFunctionList
#undef UseVdpFunctionOne

public:
  bool Init();

private:
  void* GetFunc(int funcId);

public:
  VdpContext(Display* _Display, int _Screen);
};
