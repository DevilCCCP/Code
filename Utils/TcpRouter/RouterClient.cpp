#include <QTcpSocket>
#include <QHostAddress>
#include <QDataStream>
#include <QDebug>

#include "RouterClient.h"


bool RouterClient::Start(int port, const QString& host, int hostPort)
{
  mPort = port;
  mHost = host;
  mHostPort = hostPort;
  ConnectToHost();

  mConnectTimer->start();
  return true;
}

void RouterClient::ConnectToHost()
{
  mServer->connectToHost(mHost, mHostPort);
  mGranted = false;
}

void RouterClient::ProcessServerPacket(QDataStream* stream)
{
  QByteArray header(kRecepientMagic.size(), (char)0);
  stream->readRawData(header.data(), header.size());
  if (header != kRecepientMagic) {
    mServer->disconnectFromHost();
    qDebug() << "server header fail";
    return;
  }

  int t;
  *stream >> t;
  PacketType type = (PacketType)t;
  int id = 0;
  *stream >> id;
  QByteArray data;
  *stream >> data;
  if (stream->status() != QDataStream::Ok) {
    mServer->disconnectFromHost();
    qDebug() << "server packet fail";
    return;
  }

  if (type == eConnection) {
    qDebug() << id << "connect";
    QTcpSocket* client = new QTcpSocket(this);
    connect(client, &QTcpSocket::connected, this, &RouterClient::OnClientConnected);
    connect(client, &QTcpSocket::disconnected, this, &RouterClient::OnClientDisconnected);
    connect(client, &QTcpSocket::readyRead, this, &RouterClient::OnClientReadyRead);
    mClientMap[id] = client;
    mClientIdMap[client] = id;
    client->connectToHost(QHostAddress(QHostAddress::LocalHost), mPort);
  } else if (type == eDisconnection) {
    qDebug() << id << "disconnect";
    auto itr = mClientMap.find(id);
    if (itr != mClientMap.end()) {
      QTcpSocket* clientSocket = itr.value();
      clientSocket->disconnect();
      clientSocket->disconnectFromHost();
      clientSocket->deleteLater();
      mClientMap.remove(id);
      mClientIdMap.remove(clientSocket);
    }
  } else if (type == eClientData) {
    auto itr = mClientMap.find(id);
    if (itr != mClientMap.end()) {
      QTcpSocket* clientSocket = itr.value();
      if (clientSocket->state() == QTcpSocket::ConnectedState) {
        qDebug() << id << "<<" << data.size();
        clientSocket->write(data);
      } else {
        mClientData[clientSocket].append(data);
      }
    }
  }
}

void RouterClient::ProcessClientPacket(QTcpSocket* socket, const QByteArray& packet)
{
  auto itr = mClientIdMap.find(socket);
  if (itr != mClientIdMap.end()) {
    int id = itr.value();
    QByteArray recepientPacket;
    QDataStream outStream(&recepientPacket, QIODevice::WriteOnly);
    outStream.writeRawData(kRecepientMagic.constData(), kRecepientMagic.size());
    outStream << (int)eServerData;
    outStream << id;
    outStream << packet;
    mServer->write(recepientPacket);
    qDebug() << id << ">>" << packet.size();
  }
}

void RouterClient::OnServerConnected()
{
  qDebug() << "server connected";
  mServer->write(kRecepientMagic);
}

void RouterClient::OnServerDisconnected()
{
  qDebug() << "server disconnected";
  QList<QTcpSocket*> clientList = mClientMap.values();
  mClientMap.clear();
  mClientIdMap.clear();
  mClientData.clear();
  for (auto itr = clientList.begin(); itr != clientList.end(); itr++) {
    QTcpSocket* socket = *itr;
    socket->disconnect();
    socket->disconnectFromHost();
    socket->deleteLater();
  }
}

void RouterClient::OnServerReadyRead()
{
  while (mServer->bytesAvailable() > 0) {
    if (!mGranted) {
      QByteArray packet = mServer->read(kRecepientGrant.size());
      if (packet == kRecepientGrant) {
        mGranted = true;
      } else {
        mServer->disconnectFromHost();
      }
    } else {
      QDataStream stram(mServer);
      ProcessServerPacket(&stram);
    }
  }
}

void RouterClient::OnClientConnected()
{
  QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
  if (!socket) {
    return;
  }
  qDebug() << mClientIdMap[socket] << "connected" << socket->peerName() << ":" << socket->peerPort();

  auto itr = mClientData.find(socket);
  if (itr != mClientData.end()) {
    const QByteArray& data = itr.value();
    qDebug() << mClientIdMap[socket] << "<<" << data.size();
    socket->write(data);
    mClientData.erase(itr);
  }
}

void RouterClient::OnClientDisconnected()
{
  QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
  if (!socket) {
    return;
  }

  qDebug() << mClientIdMap[socket] << "disconnected";
  mClientData.remove(socket);
  auto itr = mClientIdMap.find(socket);
  if (itr != mClientIdMap.end()) {
    int id = itr.value();

    QByteArray recepientPacket;
    QDataStream outStream(&recepientPacket, QIODevice::WriteOnly);
    outStream.writeRawData(kRecepientMagic.constData(), kRecepientMagic.size());
    outStream << (int)eDisconnection;
    outStream << id;
    outStream << QByteArray();
    mServer->write(recepientPacket);
    mClientIdMap.erase(itr);
    mClientMap.remove(id);
  }
}

void RouterClient::OnClientReadyRead()
{
  QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
  if (!socket) {
    return;
  }

  while (socket->bytesAvailable() > 0) {
    QByteArray packet = socket->readAll();
    if (packet.isEmpty()) {
      return;
    }
    ProcessClientPacket(socket, packet);
  }
}

void RouterClient::OnConnectTimerTimeout()
{
  if (mServer->state() != QTcpSocket::ConnectedState && mServer->state() != QTcpSocket::ConnectingState) {
    ConnectToHost();
  }

  for (auto itr = mClientMap.begin(); itr != mClientMap.end(); ) {
    QTcpSocket* client = itr.value();
    int id = itr.key();
    if (client->state() != QTcpSocket::ConnectedState && client->state() != QTcpSocket::ConnectingState) {
      qDebug() << id << "client lost";
      itr = mClientMap.erase(itr);
      mClientIdMap.remove(client);
      mClientData.remove(client);
      client->deleteLater();

      QByteArray recepientPacket;
      QDataStream outStream(&recepientPacket, QIODevice::WriteOnly);
      outStream.writeRawData(kRecepientMagic.constData(), kRecepientMagic.size());
      outStream << (int)eDisconnection;
      outStream << id;
      outStream << QByteArray();
      mServer->write(recepientPacket);
    } else {
      itr++;
    }
  }
}


RouterClient::RouterClient(QObject* parent)
  : QObject(parent)
  , mServer(new QTcpSocket(this)), mPort(0), mConnectTimer(new QTimer(this))
{
  connect(mServer, &QTcpSocket::connected, this, &RouterClient::OnServerConnected);
  connect(mServer, &QTcpSocket::disconnected, this, &RouterClient::OnServerDisconnected);
  connect(mServer, &QTcpSocket::readyRead, this, &RouterClient::OnServerReadyRead);
  connect(mConnectTimer, &QTimer::timeout, this, &RouterClient::OnConnectTimerTimeout);
  mConnectTimer->setInterval(500);
}
