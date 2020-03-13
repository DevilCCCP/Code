#pragma once

#include <QByteArray>
#include <QRect>

#include <LibV/Include/Frame.h>
//#include <Lib/Dispatcher/Conveyor.h>


DefineClassS(Render);
DefineClassS(Conveyor);

class Render
{
public:
  /*new */virtual bool Init() = 0;
  /*new */virtual bool SetRegion(const QRect& srcRegion, const QRect& destRegion) = 0;
  /*new */virtual bool SetWidget(QWidget* destWidget) = 0;
  /*new */virtual bool SetPause(bool paused) = 0;
  /*new */virtual void Release() = 0;

public:
  /*new */virtual void SetSource(Conveyor* source) = 0;
  /*new */virtual bool PlayFrame(const FrameS& frame) = 0;

public:
  virtual ~Render() {}
};

