#pragma once

#include <QByteArray>
#include <QRect>

#include <Lib/Dispatcher/Conveyor.h>

#include "Render.h"


DefineClassS(QtRender);
DefineClassS(Overseer);
DefineClassS(Decoder);
DefineClassS(DecodeReceiver);
DefineClassS(WidgetImageR);

class QtRender: public Render
{
  OverseerS       mOverseer;
  bool            mUseDecoder;

  DecoderS        mDecoder;
  DecodeReceiverS mDecodeReceiver;
  QWidget*        mParentWidget;
  WidgetImageR*   mDrawWidget;
  QRect           mSourceRect;
  QRect           mDestRect;

public:
  WidgetImageR*   DrawWidget() { return mDrawWidget; }

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
  void ReleaseSource(Conveyor* source);
  void ClearImage();
  void ConnectConsumer(CtrlWorker* consumer);

public:
  QtRender(const OverseerS& _Overseer, bool _UseDecoder = true);
  virtual ~QtRender();
};

