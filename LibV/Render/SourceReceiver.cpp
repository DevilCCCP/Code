#include <Lib/Log/Log.h>

#include "SourceReceiver.h"


SourceReceiver::SourceReceiver(SourceConsumer* _SourceConsumer)
  : mSourceConsumer(_SourceConsumer)
{
}

SourceReceiver::~SourceReceiver()
{
}


void SourceReceiver::Stop()
{
  mSourceConsumer = nullptr;

  ConveyorV::Stop();
}

bool SourceReceiver::ProcessFrame()
{
  FrameS lastFrame = CurrentVFrame();

  if (!lastFrame->IsFrame()) {
    return false;
  }

  if (mSourceConsumer) {
    mSourceConsumer->OnFrame(lastFrame);
    return true;
  }
  return false;
}
