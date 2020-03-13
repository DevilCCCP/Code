#pragma once

#include <Lib/Include/Common.h>

DefineClassS(NetMessage);
DefineClassS(Chater);

class Receiver
{
  Chater* mChater;

protected:
  Chater* GetChater() { return mChater; }
private:/*internal*/
  void SetChater(Chater* _Chater) { mChater = _Chater; }

public:
  /*new */virtual void DoCircle();

  /*new */virtual bool ReceiveRequest(NetMessageS& msg, int& rspMsgId, QByteArray& rspMsgData); /*return true: send respond, false: skip*/
  /*new */virtual bool ReceiveMessage(NetMessageS& msg);
  /*new */virtual void OnDisconnected();

public:
  Receiver();
  /*new */virtual ~Receiver();

  friend class Chater;
};
