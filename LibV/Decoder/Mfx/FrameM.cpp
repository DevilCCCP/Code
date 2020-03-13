#include "FrameM.h"
#include "MfxContainer.h"


FrameM::FrameM(MfxContainer* _MfxContainer, char* _RawData)
  : Frame(_RawData)
  , mMfxContainer(_MfxContainer)
{ }

FrameM::~FrameM()
{
  mMfxContainer->FreeFrame();
}
