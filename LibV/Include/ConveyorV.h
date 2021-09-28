#pragma once

#include <LibV/Include/Frame.h>
#include <Lib/Dispatcher/Conveyor.h>
#include <Lib/Log/Log.h>


DefineClassS(Frame);
DefineClassS(Conveyor);

class ConveyorV: public Conveyor
{
#ifdef QT_NO_DEBUG
  static const int kFrameOverflowCount = 200;
  static const int kFrameOverflowClearCount = 100;
#else
  static const int kFrameOverflowCount = 2000;
  static const int kFrameOverflowClearCount = 100;
#endif

protected:
  FrameS CurrentVFrame() { return CurrentFrame().staticCast<Frame>(); }

protected:
#ifdef QT_NO_DEBUG
  /*override */virtual void OnOverflow(QList<FrameAS>& conveyorFrames) override
  {
    conveyorFrames.erase(conveyorFrames.begin(), conveyorFrames.begin() + kFrameOverflowClearCount);
    while (!conveyorFrames.isEmpty()) {
      const FrameAS& fr = conveyorFrames.first();
      bool key = static_cast<const Frame*>(fr.data())->GetHeader()->Key;
      if (key) {
        break;
      }
      conveyorFrames.removeFirst();
    }
  }
#else
  /*override */virtual void OnOverflow(QList<FrameAS>& conveyorFrames) override
  {
    conveyorFrames.erase(conveyorFrames.begin() + kFrameOverflowCount - kFrameOverflowClearCount, conveyorFrames.end());
    while (!conveyorFrames.isEmpty()) {
      const FrameAS& fr = conveyorFrames.takeLast();
      bool key = static_cast<const Frame*>(fr.data())->GetHeader()->Key;
      if (key) {
        break;
      }
    }
  }
#endif

protected:
  bool EmergeVFrame(const FrameS& frame)
  {
    return EmergeFrame(frame.staticCast<FrameA>());
  }

public:
  ConveyorV(int _WorkPeriodMs = -1, int _FrameOverflowCount = kFrameOverflowCount)
    : Conveyor(_FrameOverflowCount, _WorkPeriodMs)
  { }
  /*override */virtual ~ConveyorV()
  { }
};

