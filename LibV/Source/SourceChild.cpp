#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Log/Log.h>

#include "SourceChild.h"
#include "FrameChannel.h"


const int kWorkPeriodMs = 20;
bool SourceChild::DoInit()
{
  mFrameChannel.reset(new FrameChannel(GetOverseer()->Id()));
  mNextFrame.reset(new Frame());

  return Source::DoInit();
}

bool SourceChild::DoCircle()
{
  while (mFrameChannel->Pop(mNextFrame)) {
    OnFrame(mNextFrame);
    mNextFrame.reset(new Frame());
    mHasFrames = true;
  }

  if (mHasFrames) {
    OnStatus(Connection::eConnected);
  } else {
    OnStatus(Connection::eNoFrames);
  }
  return true;
}

void SourceChild::Reconnect()
{
  mHasFrames = false;
}

bool SourceChild::NeedDecoder()
{
  return true;
}


SourceChild::SourceChild()
  : Source(kWorkPeriodMs)
  , mHasFrames(false)
{
}

SourceChild::~SourceChild()
{
}
