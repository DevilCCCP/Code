#pragma once

#include "AnalyticsA.h"


class SceneAnalizer
{
  const AnalyticsA& mAnalytics;

protected:
  int Width()       const { return mAnalytics.getWidth(); }
  int Height()      const { return mAnalytics.getHeight(); }
  int Stride()      const { return mAnalytics.getStride(); }

  int    CurrentFrame() const { return mAnalytics.getCurrentFrame(); }
  qint64 CurrentMs()    const { return mAnalytics.getCurrentMs(); }
  qint64 CurTimestamp() const { return mAnalytics.getLastTimestamp(); }
  int FrameMs()    const { return mAnalytics.getFrameMs(); }
  float FrameSec() const { return mAnalytics.getFrameSec(); }

  const uchar* SourceImageData() const { return mAnalytics.SourceImageData(); }

public:
  SceneAnalizer(const AnalyticsA& _Analytics)
    : mAnalytics(_Analytics)
  { }
};

