#pragma once

#include <QByteArray>
#include <QRect>

#include <LibV/Include/Frame.h>


DefineClassS(OmxRender);
DefineClassS(Thumbnail);
DefineClassS(IlComponents);
DefineClassS(IlTunnels);
DefineStructS(_ILCLIENT_T);
DefineStructS(_COMPONENT_T);

class OmxRender
{
  _ILCLIENT_TS    mIlClient;
  IlComponentsS   mIlDecoder;
  IlTunnelsS      mIlTunnels;
  bool            mInit;
  bool            mFirstFrame;
  bool            mAfterReadSetupDone;
  bool            mPause;

public:
  bool InitDecoder();
  bool SetRenderRegion(const QRect& region, const QSize& imgSize);
  bool SetRenderPause(bool paused);
  bool CloseDecoder();
  void DeinitDecoder();

public:
  bool DecodeIn(char* frameData, int frameSize);

public:
  OmxRender();
  ~OmxRender();
};

