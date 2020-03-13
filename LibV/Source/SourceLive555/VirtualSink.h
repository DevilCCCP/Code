#pragma once

#include "Def.h"
#include <vector>

DefineClassS(FrameProcessor);

class VirtualSink : public MediaSink
{
  std::vector<char> mBuffer;
  FrameProcessorS   mFrameProcessor;

public:
  void ConnectFrameProcessor(FrameProcessorS& fp);

  /*override */virtual Boolean continuePlaying();
  /*new */virtual void AfterGettingFrame(int frameSize, int truncatedBytesCount, struct timeval presentationTime, int durationInMicroseconds);

public:
  VirtualSink(UsageEnvironment& usageEnvironment);
  virtual ~VirtualSink();
};
