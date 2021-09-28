#pragma once

#include <QSharedPointer>

#include <Lib/Settings/SettingsA.h>
#include <LibV/Include/Frame.h>

#include "Source.h"


DefineClassS(SourceChild);
DefineClassS(FrameChannel);

class SourceChild: public Source
{
  FrameChannelS mFrameChannel;
  FrameS        mNextFrame;
  bool          mHasFrames;

public:
  /*override */virtual const char* Name() override { return "SourceChild"; }
  /*override */virtual const char* ShortName() override { return "S"; }
protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
//  /*override */virtual void DoRelease() override;

//  /*override */virtual void Stop() override;

protected:
  /*override */virtual void Reconnect() override;

public:
  /*override */virtual bool NeedDecoder() override;

private:

public:
  SourceChild();
  /*override */virtual ~SourceChild();
};
