#pragma once

#include <LibV/Include/Frame.h>
#include <LibV/Include/VideoMsg.h>
#include <Lib/Net/Receiver.h>
#include <Lib/Net/NetMessage.h>
#include <Lib/Log/Log.h>


//#define LogLive(X) Log.Trace(X)
#define LogLive(X)
DefineClassS(CameraPlayer);

template<typename RenderT>
class FrameReceiver: public Receiver
{
  CameraPlayer* mPlayer;
  RenderT*      mRender;
  int           mId;

public:
  /*override */virtual bool ReceiveMessage(NetMessageS& msg) override
  {
    switch (msg->GetMessageType()) {
    case eMsgPlayGranted:
      LogLive("eMsgPlayGranted");
      mRender->VideoGranted(mPlayer);
      return false;

    case eMsgPlayDenied:
      LogLive("eMsgPlayDenied");
      mRender->VideoDenied(mPlayer);
      return false;

    case eMsgPlayDropped:
      LogLive("eMsgPlayDropped");
      mRender->VideoDropped(mPlayer);
      return false;

    case eMsgPlayAborted:
      LogLive("eMsgPlayAborted");
      mRender->VideoAborted(mPlayer);
      return false;

    case eMsgPlayNoStorage:
      LogLive("eMsgPlayNoStorage");
      mRender->VideoNoStorage(mPlayer);
      return false;

    case eMsgVideoInfo:
      LogLive("eMsgVideoInfo");
      if (msg->GetMessageDataSize() == sizeof (VideoInfo)) {
        VideoInfo* req = reinterpret_cast<VideoInfo*>(msg->GetMessageData());
        mRender->VideoInfo(mPlayer, req->StartTimestamp);
      } else {
        Log.Warning(QString("Get video info with wrong size (size: %1)").arg(msg->GetMessageDataSize()));
      }
      return false;

    case eMsgVideoFrame:
      LogLive("eMsgVideoFrame");
      ReceiveFrame(msg);
      return false;

    default:
      LogLive(QString("Unimplemented msg received (type: %1)").arg(msg->GetMessageType()));
      LOG_WARNING_ONCE(QString("Unimplemented msg received (type: %1)").arg(msg->GetMessageType()));
      return true;
    }
  }

  /*override */virtual void OnDisconnected() override
  {
    mRender->Disconnected(mPlayer);
  }

public:
  void ConnectPlayer(CameraPlayer* _Player) { mPlayer = _Player; }

private:
  void ReceiveFrame(NetMessageS& msg)
  {
    FrameS frame(new Frame());
    frame->ReserveData(msg->GetMessageDataSize());
    memcpy(frame->Data(), msg->GetMessageData(), msg->GetMessageDataSize());
    mRender->VideoFrame(mPlayer, frame);
  }

public:
  FrameReceiver(RenderT* _Render, int _Id)
    : mPlayer(nullptr), mRender(_Render), mId(_Id)
  {
  }
  /*override */virtual ~FrameReceiver()
  {
  }
};
#undef LogLive
