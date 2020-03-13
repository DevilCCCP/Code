#pragma once

#include <QMutex>
#include <QElapsedTimer>

#include <Lib/Net/Receiver.h>


DefineClassS(TrReceiver);
DefineClassS(Transmit);
DefineClassS(Storage);
DefineClassS(Frame);
DefineClassS(Decoder);
DefineStructS(PtzRequest);
DefineStructS(PtzRespond);

class TrReceiver: public Receiver
{
  Transmit*     mTransmit;

  bool          mStorageConnected;
  StorageS      mStorage;
  QMutex        mStoreMutex;
  QElapsedTimer mTimerFromStart;
  qint64        mStartTimestamp;
  qint64        mLastTimestamp;
  qint64        mNextLogTimestamp;
  int           mStorageSpeedNum;
  int           mStorageSpeedDenum;
  FrameS        mStorageFrame;
  bool          mSlowSendWarning;

  QList<FrameS> mMemoryStoreFrames;
  bool          mMemoryStoreConnected;

public:
  /*override */virtual void DoCircle() Q_DECL_OVERRIDE;

  /*override */virtual bool ReceiveRequest(NetMessageS& msg, int& rspMsgId, QByteArray& rspMsgData) Q_DECL_OVERRIDE;
  /*override */virtual bool ReceiveMessage(NetMessageS& msg) Q_DECL_OVERRIDE;
  /*override */virtual void OnDisconnected() Q_DECL_OVERRIDE;

private:
  void DispatchMsgLiveRequest(NetMessageS &msg);
  void DispatchMsgArchRequest(NetMessageS &msg);
  void DispatchThumbnailMessage();
  void DispatchThumbnailRequest(int &rspMsgId, QByteArray &rspMsgData);
  void DispatchMediaInfoMessage();
  void DispatchMediaInfoRequest(int &rspMsgId, QByteArray &rspMsgData);
  void DispatchPtzRequest(NetMessageS &msg, int &rspMsgId, QByteArray &rspMsgData);

private:
  bool InitStorage(const qint64& timestamp, int speedNum, int speedDenum);
  bool TransmitNextStorageFrame();
  bool GetNextStorageFrame();

  bool TakeThumbnail(QByteArray& rspMsgData);

  bool DoPtz(const PtzRequest& request, PtzRespond& respond);
  bool ReturnPtz(PtzRespond& respond, int flag, bool result);

public:
  TrReceiver(Transmit* _Transmit);
  /*override */virtual ~TrReceiver();
};

