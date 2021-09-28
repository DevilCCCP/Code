#pragma once

#include <QByteArray>
#include <QRect>

#include <LibV/Include/Frame.h>

#include "../Render.h"
#include "../SourceReceiver.h"


DefineClassS(OmxRender);
DefineClassS(Overseer);
DefineClassS(SourceReceiver);
DefineClassS(Thumbnail);
DefineClassS(IlComponents);
DefineClassS(IlTunnels);
DefineStructS(_ILCLIENT_T);
DefineStructS(_COMPONENT_T);

class OmxRender: public Render, public SourceConsumer
{
  OverseerS       mOverseer;
  SourceReceiverS mSourceReceiver;

  _ILCLIENT_TS    mIlClient;
  IlComponentsS   mIlDecoder;
  IlTunnelsS      mIlTunnels;
  bool            mInit;
  bool            mFirstFrame;
  bool            mAfterReadSetupDone;
  bool            mPause;

public:
  /*override */virtual bool Init() override;
  /*override */virtual bool SetRegion(const QRect& srcRegion, const QRect& destRegion) override;
  /*override */virtual bool SetWidget(QWidget* destWidget) override;
  /*override */virtual bool SetPause(bool paused) override;
  /*override */virtual void Release() override;

public:
  /*override */virtual void SetSource(Conveyor* source) override;
  /*override */virtual bool PlayFrame(const FrameS& frame) override;

public:
  /*override */virtual void OnFrame(FrameS frame) override;

public:
  bool DecodeIn(char* frameData, int frameSize);

public:
  OmxRender(const OverseerS& _Overseer);
  ~OmxRender();
};

