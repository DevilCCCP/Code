#pragma once

#include <QByteArray>

#include <LibV/Include/Frame.h>

#include "MfxDef.h"


DefineClassS(MfxContainer);
DefineClassS(FrameM);

class MfxContainer
{
  MfxFrameSurface mMfxFrameSurface;
  QByteArray      mFrameData;
  volatile bool   mUsed;

public:
  MfxFrameSurface* Surface()             { return &mMfxFrameSurface; }
  const MfxFrameSurface* Surface() const { return &mMfxFrameSurface; }
  Frame::Header* FrameHeader()             { return reinterpret_cast<Frame::Header*>(mFrameData.data()); }
  const Frame::Header* FrameHeader() const { return reinterpret_cast<const Frame::Header*>(mFrameData.constData()); }
  char* FrameData()             { return reinterpret_cast<char*>(mFrameData.data() + sizeof(Frame::Header)); }
  const char* FrameData() const { return reinterpret_cast<const char*>(mFrameData.constData() + sizeof(Frame::Header)); }
  bool IsReady();

public:
  FrameS CreateOutputFrame();
  void FreeFrame();

public:
  MfxContainer(int frameSize);
};
