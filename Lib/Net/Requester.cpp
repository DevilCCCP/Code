#include <QTcpSocket>
#include <QHostInfo>

#include <Lib/Log/Log.h>

#include "Requester.h"
#include "Chater.h"
#include "QTcpSocket2.h"


const int kConnectMaxMs = 30000;

bool Requester::DoInit()
{
  mConnection2 = QTcpSocket2S(new QTcpSocket2);
  SetTcpSocket(mConnection2.staticCast<QTcpSocket>());
  mConnection2->connectToHost(GetUri().Host(), GetUri().Port());
  mConnecting = true;
  bool connected = mConnection2->waitForConnected(kConnectMaxMs);
  mConnecting = false;
  if (connected) {
    Log.Info(QString("Connected to host '%1'").arg(GetUri().ToString()));
    return true;
  } else {
    Log.Trace(QString("Connection fail (host '%1', error: '%2')").arg(GetUri().ToString()).arg(mConnection2->errorString()));
    DoRelease();
    return false;
  }
}

void Requester::DoRelease()
{
  Messenger::DoRelease();
  mConnection2.clear();
}

void Requester::Stop()
{
  Messenger::Stop();

  if (mConnecting) {
    mConnection2->Stop();
  }
}

//void Requester::LookupHostname()
//{
//  QHostInfo infoHost = QHostInfo::fromName(mCurrentChater->Host());
//  auto list = infoHost.addresses();
//  if (list.size() > 1) {
//    QString listText;
//    auto itr = list.begin();
//    listText = list.front().toString();
//    for (itr++; itr != list.end(); itr++) {
//      listText.append(", ");
//      listText.append(list.front().toString());
//    }
//    Log.Info(QString("Host '%1' resolved with several addresses, pick 1'st from ('%2')").arg(mCurrentChater->Host()).arg(listText));
//  } else if (list.size() == 0) {
//    Log.Warning(QString("Host '%1' not resolved").arg(mCurrentChater->Host()));
//    return false;
//  } else {
//    Log.Trace(QString("Host '%1' resolved as '%2'").arg(mCurrentChater->Host()).arg(list.at(0).toString()));
//  }
//  mConnection->connectToHost(list.at(0), mCurrentChater->Port());
//}

Requester::Requester(const Uri &_Uri)
  : mConnecting(false)
{
  SetUri(_Uri);
}

Requester::~Requester()
{
}


