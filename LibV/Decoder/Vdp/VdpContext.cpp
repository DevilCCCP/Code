#include "VdpContext.h"


bool VdpContext::Init()
{
  VdpStatus err = vdp_device_create_x11(mDisplay, mScreen, &mVdpDevice, &mVdpGetProcAddress);
  if (err != VDP_STATUS_OK) {
    mErrorText = (err == VDP_STATUS_NO_IMPLEMENTATION)? "No vdpau implementation": "Fail vdpau create";
    return false;
  }
  if (mVdpDevice == VDP_INVALID_HANDLE) {
    mErrorText = "Invalid VdpDevice handle";
    return false;
  } else if (!mVdpGetProcAddress) {
    mErrorText = "VdpGetProcAddress is nullptr";
    return false;
  }

#define UseVdpFunctionOne(VdpId, VdpFunc) \
  func##VdpFunc = (VdpFunc*)GetFunc(VdpId); \
  if (!func##VdpFunc) { \
    mErrorText = QString(#VdpFunc) + " function is nullptr"; \
    return false; \
  }
  UseVdpFunctionList;
#undef UseVdpFunctionOne
  return true;
}

void* VdpContext::GetFunc(int funcId)
{
  void* func = nullptr;
  VdpStatus err = mVdpGetProcAddress(mVdpDevice, funcId, &func);
  return (err == VDP_STATUS_OK)? func: nullptr;
}


VdpContext::VdpContext(Display* _Display, int _Screen)
  : mDisplay(_Display), mScreen(_Screen)
  , mVdpDevice(VDP_INVALID_HANDLE)
  , mVdpGetProcAddress(nullptr)
#define UseVdpFunctionOne(VdpId, VdpFunc) , func##VdpFunc(nullptr)
  UseVdpFunctionList
#undef UseVdpFunctionOne
{
}
