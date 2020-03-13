#pragma once

#include <QList>

#include "Channel.h"


class MediaPackager
{
  QList<TrFrameS> mTrFrames;

public:
  /*new */virtual void InFrame(const qint64& timestamp, const char* data, int size) = 0;

protected:
  void AddFrame(const TrFrameS& trFrame) { mTrFrames.append(trFrame); }

public:
  bool GetNextFrame(TrFrameS& trFrame)
  {
    if (!mTrFrames.isEmpty()) {
      trFrame = mTrFrames.takeFirst();
      return true;
    }
    return false;
  }

public:
  explicit MediaPackager() { }
  /*new */virtual ~MediaPackager() { }
};
