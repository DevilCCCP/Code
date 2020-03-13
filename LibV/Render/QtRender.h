#pragma once

#include <QByteArray>
#include <QRect>

#include <Lib/Dispatcher/Conveyor.h>

#include "Render.h"


DefineClassS(QtRender);
DefineClassS(Overseer);
DefineClassS(Decoder);
DefineClassS(DecodeReceiver);
DefineClassS(QWidgetB);

class QtRender: public Render
{
  OverseerS       mOverseer;
  DecoderS        mDecoder;
  DecodeReceiverS mDecodeReceiver;

  QWidget*        mParentWidget;
  QWidgetB*       mDrawWidget;
  QRect           mSourceRect;
  QRect           mDestRect;

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
  QtRender(const OverseerS& _Overseer);
  virtual ~QtRender();
};

