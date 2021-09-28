#pragma once

#include <Lib/Log/Log.h>
#include <Lib/Db/Db.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Net/Receiver.h>
#include <Lib/Net/NetMessage.h>
#include <LibV/Include/VideoMsg.h>


DefineClassS(RtspCamReceiver);
DefineClassS(RtspInit);

class RtspCamReceiver: public Receiver
{
  ObjectItemS   mItem;
  RtspInit*     mRtspInit;

  bool          mWarningFail;
  bool          mWarningDisconnect;

public:
  /*override */virtual bool ReceiveMessage(NetMessageS& msg) override
  {
    switch (msg->GetMessageType()) {
    case eMsgSpsPps:
      mRtspInit->GetSpsPps(mItem, msg->GetMessageConstData(), msg->GetMessageDataSize());
      return false;

    case eMsgNoMediaInfo:
      if (!mWarningFail) {
        Log.Warning(QString("No media info (camera: %1)").arg(mItem->Id));
        mWarningFail = true;
      }
      mRtspInit->GetFail(mItem);
      return false;

    default:
      LOG_WARNING_ONCE(QString("Unimplemented msg received (type: %1)").arg(msg->GetMessageType()));
      return true;
    }
  }

  /*override */virtual void OnDisconnected() override
  {
    if (!mWarningDisconnect) {
      Log.Warning(QString("Disconnected (camera: %1)").arg(mItem->Id));
      mWarningDisconnect = true;
    }
    mRtspInit->GetFail(mItem);
  }

public:
  RtspCamReceiver(const ObjectItemS& _Item, RtspInit* _RtspInit)
    : mItem(_Item), mRtspInit(_RtspInit)
    , mWarningFail(false), mWarningDisconnect(false)
  {
  }

  /*override */virtual ~RtspCamReceiver()
  {
  }
};

