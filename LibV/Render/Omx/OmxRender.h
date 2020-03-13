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
  /*override */virtual bool Init() Q_DECL_OVERRIDE;
  /*override */virtual bool SetRegion(const QRect& srcRegion, const QRect& destRegion) Q_DECL_OVERRIDE;
  /*override */virtual bool SetWidget(QWidget* destWidget) Q_DECL_OVERRIDE;
  /*override */virtual bool SetPause(bool paused) Q_DECL_OVERRIDE;
  /*override */virtual void Release() Q_DECL_OVERRIDE;

public:
  /*override */virtual void SetSource(Conveyor* source) Q_DECL_OVERRIDE;
  /*override */virtual bool PlayFrame(const FrameS& frame) Q_DECL_OVERRIDE;

public:
  /*override */virtual void OnFrame(FrameS frame) Q_DECL_OVERRIDE;

public:
  bool DecodeIn(char* frameData, int frameSize);

public:
  OmxRender(const OverseerS& _Overseer);
  ~OmxRender();
};

