#pragma once

#include <QImage>

#include <LibV/Include/ConveyorV.h>


class SourceConsumer
{
public:
  /*new */virtual void OnFrame(FrameS frame) = 0;

public:
  virtual ~SourceConsumer() { }
};

class SourceReceiver: public ConveyorV
{
  SourceConsumer*  mSourceConsumer;

public:
  explicit SourceReceiver(SourceConsumer* _SourceConsumer);
  ~SourceReceiver();

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "SourceReceiver"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "Sr"; }

  /*override */virtual void Stop() Q_DECL_OVERRIDE;

protected:
  /*override */virtual bool ProcessFrame() Q_DECL_OVERRIDE;
};

