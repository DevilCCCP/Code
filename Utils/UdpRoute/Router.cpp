#include <QNetworkDatagram>

#include "Router.h"


Router::Router(QObject* parent)
  : QObject(parent)
  , mBindSocket(new QUdpSocket(this))
{
  connect(mBindSocket, &QUdpSocket::readyRead, this, &Router::OnReadyRead);
}

Router::~Router()
{
}


void Router::Start(int bindPort, const QVector<int>& sendPortList)
{
  if (mBindSocket->state() != QUdpSocket::UnconnectedState) {
    Stop();
  }

  if (!mBindSocket->bind(bindPort)) {
    emit Error(mBindSocket->errorString());
    return;
  }

  mSendPortList = sendPortList;
  for (int i = mSendSocket.size(); i < mSendPortList.size(); i++) {
    mSendSocket.append(new QUdpSocket(this));
  }

  emit Started();
}

void Router::Stop()
{
  if (mBindSocket->state() == QUdpSocket::UnconnectedState) {
    return;
  }

  mBindSocket->disconnectFromHost();
}

void Router::OnReadyRead()
{
  while (mBindSocket->hasPendingDatagrams()) {
    QNetworkDatagram datagram = mBindSocket->receiveDatagram();

    for (int i = 0; i < mSendPortList.size(); i++) {
      mSendSocket[i]->writeDatagram(datagram.data(), QHostAddress(QHostAddress::LocalHost), mSendPortList[i]);
    }
  }

  emit ProcessFrame();
}
