#pragma once

#include <QByteArray>

#include <LibV/Include/Frame.h>


DefineClassS(FrameM);
DefineClassS(MfxContainer);

class FrameM: public Frame
{
  MfxContainer* mMfxContainer;

public:
  FrameM(MfxContainer* _MfxContainer, char* _RawData);
  /*override */virtual ~FrameM();
};
