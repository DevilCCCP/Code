#include <QTcpSocket>
#include <QDataStream>

#include <Lib/Log/Log.h>

#include "RouterServer.h"


bool RouterServer::Start(int port)
{
  return mServer->listen(QHostAddress::Any, port);
}

void RouterServer::ProcessNew(QTcpSocket* socket)
{
  QByteArray data = socket->readAll();
  if (data.size() == Packet::HeaderSize()) {
    QDataStream stream(data);
    Packet packet;
    if (packet.ReadHeader(&stream) && packet.getType() == eHello) {
      ProcessNewRecepient(socket);
      return;
    }
  }
  ProcessNewClient(socket, data);
}

void RouterServer::ProcessNewRecepient(QTcpSocket* socket)
{
  if (mDebug) {
    Log.Info(QString("New recepient %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
  }
  mSocketMap[socket] = eRecepient;
  mRecepientPacketMap.insert(socket, Packet());

  if (mDebug) {
    Log.Info(QString("Recepient Granted"));
  }
  Packet packet;
  packet.setType(eGrant);
  QDataStream stream(socket);
  packet.Write(&stream);

  for (auto itr = mClientIdMap.begin(); itr != mClientIdMap.end(); itr++) {
    int id = itr.value();

    Packet packet;
    packet.setType(eConnection);
    packet.setId(id);
    QDataStream stream(socket);
    packet.Write(&stream);
  }
}

void RouterServer::ProcessNewClient(QTcpSocket* socket, const QByteArray& data)
{
  mSocketMap[socket] = eClient;
  int id = ++mLastClientId;
  mClientMap[id] = socket;
  mClientIdMap[socket] = id;
  ProcessClientNewConnection(mLastClientId);

  if (mDebug) {
    Log.Info(QString("Client %1 connected").arg(id));
    Log.Info(QString("%1 << %2").arg(id).arg(data.size()));
  }

  Packet packet;
  packet.setType(eClientData);
  packet.setId(id);
  packet.setData(data);

  SendAllRecepient(packet);
}

void RouterServer::ProcessClient(QTcpSocket* socket)
{
  QByteArray data = socket->readAll();
  auto itr = mClientIdMap.find(socket);
  if (itr != mClientIdMap.end()) {
    int id = itr.value();
    if (mDebug) {
      Log.Info(QString("%1 << %2").arg(id).arg(data.size()));
    }

    Packet packet;
    packet.setType(eClientData);
    packet.setId(id);
    packet.setData(data);

    SendAllRecepient(packet);
  }
}

void RouterServer::ProcessRecepient(QTcpSocket* socket)
{
  Packet& packet = mRecepientPacketMap[socket];
  while (socket->bytesAvailable() >= packet.MinimumSize()) {
    QDataStream stream(socket);
    bool ok = true;
    if (packet.HasHeader()) {
      if (packet.ReadData(&stream)) {
        ok = ProcessRecepientPacket(socket, packet);
        packet.Clear();
      } else {
        Log.Warning(QString("Recepient data fail (%1: %2)").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
        ok = false;
      }
    } else {
      if (!packet.ReadHeader(&stream)) {
        Log.Warning(QString("Recepient header fail (%1: %2)").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
        ok = false;
      }
    }

    if (!ok) {
      socket->disconnectFromHost();
      mRecepientPacketMap.remove(socket);
      return;
    }
  }
}

bool RouterServer::ProcessRecepientPacket(QTcpSocket* socket, const Packet& packet)
{
  switch (packet.getType()) {
  case eHello        : ProcessRecepientHello(socket); break;
  case eDisconnection: ProcessRecepientDisconnection(packet.getId()); break;
  case eServerData:    ProcessRecepientServerData(packet.getId(), packet.getData()); break;
  default:
    Log.Warning(QString("Unexpected recepient packet (type: %1)").arg((int)packet.getType()));
    break;
  }
  return true;
}

void RouterServer::ProcessRecepientHello(QTcpSocket* socket)
{
  if (mDebug) {
    Log.Info(QString("Recepient Grant"));
  }
  Packet packet;
  packet.setType(eGrant);
  QDataStream stream(socket);
  packet.Write(&stream);
}

void RouterServer::ProcessRecepientDisconnection(int id)
{
  auto itr = mClientMap.find(id);
  if (itr != mClientMap.end()) {
    QTcpSocket* clientSocket = itr.value();
    if (mDebug) {
      Log.Info(QString("%1 disconnect").arg(id));
    }
    clientSocket->disconnect();
    clientSocket->disconnectFromHost();
    clientSocket->deleteLater();
    mClientMap.remove(id);
    mClientIdMap.remove(clientSocket);
  }
}

void RouterServer::ProcessRecepientServerData(int id, const QByteArray& data)
{
  auto itr = mClientMap.find(id);
  if (itr != mClientMap.end()) {
    QTcpSocket* clientSocket = itr.value();
    if (mDebug) {
      Log.Info(QString("%1 >> %2").arg(id).arg(data.size()));
    }
    clientSocket->write(data);
  } else {
    if (mDebug) {
      Log.Info(QString("%1 no client").arg(id));
    }
  }
}

void RouterServer::ProcessClientNewConnection(int id)
{
  Packet packet;
  packet.setType(eConnection);
  packet.setId(id);

  SendAllRecepient(packet);
}

void RouterServer::SendAllRecepient(const Packet& packet)
{
  for (auto itr = mRecepientPacketMap.begin(); itr != mRecepientPacketMap.end(); itr++) {
    QTcpSocket* socket = itr.key();
    QDataStream stream(socket);
    packet.Write(&stream);
  }
}

void RouterServer::OnNewConnection()
{
  while (mServer->hasPendingConnections()) {
    QTcpSocket* socket = mServer->nextPendingConnection();
    mSocketMap[socket] = eNew;

    if (mDebug) {
      Log.Info(QString("Connected %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
    }

    connect(socket, &QTcpSocket::readyRead, this, &RouterServer::OnReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &RouterServer::OnDisconnected);
  }
}

void RouterServer::OnReadyRead()
{
  QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
  if (!socket || !mSocketMap.contains(socket)) {
    return;
  }

  EType type = mSocketMap[socket];
  switch (type) {
  case eNew      : ProcessNew(socket); break;
  case eClient   : ProcessClient(socket); break;
  case eRecepient: ProcessRecepient(socket); break;
  }
}

void RouterServer::OnDisconnected()
{
  QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
  if (!socket || !mSocketMap.contains(socket)) {
    return;
  }

  EType type = mSocketMap[socket];
  if (type == eClient) {
    auto itr = mClientIdMap.find(socket);
    if (itr != mClientIdMap.end()) {
      int id = itr.value();
      if (mDebug) {
        Log.Info(QString("%1 disconnected").arg(id));
      }
      mClientIdMap.erase(itr);
      mClientMap.remove(id);

      Packet packet;
      packet.setType(eDisconnection);
      packet.setId(id);

      SendAllRecepient(packet);
    } else {
      if (mDebug) {
        Log.Warning(QString("Unknown client disconnected %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
      }
    }
  } else if (type == eRecepient) {
    if (mDebug) {
      Log.Info(QString("Recepient disconnected %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
    }
    mRecepientPacketMap.remove(socket);
  } else {
    if (mDebug) {
      Log.Warning(QString("Unknown disconnected %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
    }
  }
  mSocketMap.remove(socket);
}

RouterServer::RouterServer(QObject* parent)
  : QObject(parent)
  , mDebug(false)
  , mServer(new QTcpServer(this))
  , mLastClientId(0)
{
  connect(mServer, &QTcpServer::newConnection, this, &RouterServer::OnNewConnection);
}
