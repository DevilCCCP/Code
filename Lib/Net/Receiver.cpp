#include <Lib/Log/Log.h>

#include "Receiver.h"
#include "Chater.h"
#include "NetMessage.h"


void Receiver::DoCircle()
{
}

bool Receiver::ReceiveRequest(NetMessageS& msg, int& rspMsgId, QByteArray& rspMsgData)
{
  rspMsgId = 0;
  rspMsgData.clear();
  Log.Warning(QString("Receive request (uri: %1, type: %2)").arg(mChater->Info()).arg(msg->GetMessageType()));
  return true;
}

bool Receiver::ReceiveMessage(NetMessageS &msg)
{
  Log.Warning(QString("Receive message (uri: %1, type: %2)").arg(mChater->Info()).arg(msg->GetMessageType()));
  return false;
}

void Receiver::OnDisconnected()
{
  Log.Info(QString("Disconnected '%1'").arg(mChater->Info()));
}


Receiver::Receiver()
{
}

Receiver::~Receiver()
{
}
