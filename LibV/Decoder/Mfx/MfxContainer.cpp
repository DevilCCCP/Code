#include <mfxvideo++.h>

#include <Lib/Log/Log.h>

#include "MfxContainer.h"
#include "FrameM.h"


bool MfxContainer::IsReady()
{
  if (mUsed) {
    return false;
  }
  return mMfxFrameSurface.Data.Locked == 0;
}

FrameS MfxContainer::CreateOutputFrame()
{
  FrameS outFrame(new FrameM(this, mFrameData.data()));
  return outFrame;
}

void MfxContainer::FreeFrame()
{
  mUsed = false;
}


MfxContainer::MfxContainer(int frameSize)
  : mUsed(false)
{
  memset(&mMfxFrameSurface, 0, sizeof(MfxFrameSurface));
  mFrameData.resize(sizeof(Frame::Header) + frameSize);
}
