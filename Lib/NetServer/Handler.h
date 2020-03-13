#pragma once

#include <QMutex>

#include <Lib/Net/SyncSocket.h>
#include <Lib/Common/Uri.h>
#include <Lib/Common/TrafficCalc.h>

DefineClassS(Handler);
DefineClassS(HandlerManager);
DefineClassS(ServerResponder);
DefineClassS(QIODevice);

class Handler
{
  const Uri*             mConnectionUri;
  PROPERTY_GET_SET(bool, Debug)

  QMutex                 mSendMutex;
  QList<QByteArray>      mSendData;
  QIODeviceS             mSendFile;
  TrafficCalc            mTrafficCalc;
  int                    mTrafficLimit;

private:/*internal*/
  void SetConnectionUri(const Uri* _ConnectionUri) { mConnectionUri = _ConnectionUri; }
protected:
  void SendFile(QIODeviceS& _SendFile, int _TrafficLimit);
  const Uri* GetConnectionUri() { return mConnectionUri; }

protected:
  /*new */virtual bool DoCircle(); /// true: send ready
  /*new */virtual bool Receive(SyncSocket* socket, bool& sendDone); /// false: disconnect
  /*new */virtual void OnDisconnected();

protected:
  void SendData(const QByteArray& data);
  void ClearData();

private:/*internal*/
  bool TakeData(QByteArray& data);

public:
  Handler();
  /*new */virtual ~Handler();

  friend class HandlerManager;
  friend class ServerResponder;
};

