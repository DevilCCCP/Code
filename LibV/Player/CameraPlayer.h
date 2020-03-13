#pragma once

#include <QSharedPointer>
#include <QElapsedTimer>

DefineClassS(Chater);
DefineClassS(CameraPlayer);

class CameraPlayer
{
  ChaterS       mChater;
  int           mCameraId;
  qint64        mStartTimestamp;
  QElapsedTimer mConfirmTimer;

public:
  int CameraId() const { return mCameraId; }
  ChaterS& Chat() { return mChater; }
  void SetChat(const ChaterS& chater) { mChater = chater; }
  const qint64& StartTimestamp() const { return mStartTimestamp; }
  qint64& StartTimestamp() { return mStartTimestamp; }

public:
  void Clear();

  bool NeedConfirm();
  bool NeedTest();

  CameraPlayer(ChaterS& _Chater, int _CameraId);
  ~CameraPlayer();
};

