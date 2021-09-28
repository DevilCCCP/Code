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
  /*override */virtual const char* Name() override { return "SourceReceiver"; }
  /*override */virtual const char* ShortName() override { return "Sr"; }

  /*override */virtual void Stop() override;

protected:
  /*override */virtual bool ProcessFrame() override;
};

