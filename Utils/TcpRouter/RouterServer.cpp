#include <QTcpSocket>
#include <QDataStream>
#include <QDebug>

#include "RouterServer.h"


bool RouterServer::Start(int port)
{
  return mServer->listen(QHostAddress::Any, port);
}

void RouterServer::ProcessNewPacket(QTcpSocket* socket)
{
  QByteArray packet = socket->readAll();
  if (packet == kRecepientMagic) {
    qDebug() << "new recepient" << socket->peerAddress() << ":" << socket->peerPort();
    mSocketMap[socket] = eRecepient;
    mRecepientList.append(socket);
    socket->write(kRecepientGrant);
    for (auto itr = mClientIdMap.begin(); itr != mClientIdMap.end(); itr++) {
      int id = itr.value();
      QByteArray recepientPacket;
      QDataStream outStream(&recepientPacket, QIODevice::WriteOnly);
      outStream.writeRawData(kRecepientMagic.constData(), kRecepientMagic.size());
      outStream << (int)eConnection;
      outStream << id;
      outStream << QByteArray();
      socket->write(recepientPacket);
    }
  } else {
    mSocketMap[socket] = eClient;
    int id = ++mLastClientId;
    mClientMap[id] = socket;
    mClientIdMap[socket] = id;
    ProcessClientNewConnection();

    qDebug() << "client" << id << "connected";
    qDebug() << id << "<<" << packet.size();
    QByteArray recepientPacket;
    QDataStream outStream(&recepientPacket, QIODevice::WriteOnly);
    outStream.writeRawData(kRecepientMagic.constData(), kRecepientMagic.size());
    outStream << (int)eClientData;
    outStream << id;
    outStream << packet;
    SendAllRecepient(recepientPacket);
  }
}

void RouterServer::ProcessClientPacket(QTcpSocket* socket)
{
  QByteArray packet = socket->readAll();
  auto itr = mClientIdMap.find(socket);
  if (itr != mClientIdMap.end()) {
    int id = itr.value();
    qDebug() << id << "<<" << packet.size();
    QByteArray recepientPacket;
    QDataStream outStream(&recepientPacket, QIODevice::WriteOnly);
    outStream.writeRawData(kRecepientMagic.constData(), kRecepientMagic.size());
    outStream << (int)eClientData;
    outStream << id;
    outStream << packet;
    SendAllRecepient(recepientPacket);
  }
}

void RouterServer::ProcessRecepientPacket(QTcpSocket* socket)
{
  QDataStream inStream(socket);
  QByteArray header(kRecepientMagic.size(), (char)0);
  inStream.readRawData(header.data(), header.size());
  if (header != kRecepientMagic) {
    socket->disconnectFromHost();
    mRecepientList.removeAll(socket);
    qDebug() << "recepient header fail" << socket->peerAddress() << ":" << socket->peerPort();
    return;
  }

  int t;
  inStream >> t;
  PacketType type = (PacketType)t;
  int id = 0;
  inStream >> id;
  QByteArray data;
  inStream >> data;
  if (inStream.status() != QDataStream::Ok) {
    socket->disconnectFromHost();
    mRecepientList.removeAll(socket);
    qDebug() << "recepient packet fail" << socket->peerAddress() << ":" << socket->peerPort();
    return;
  }

  if (type == eDisconnection) {
    auto itr = mClientMap.find(id);
    if (itr != mClientMap.end()) {
      QTcpSocket* clientSocket = itr.value();
      qDebug() << "client" << id << "disconnect";
      clientSocket->disconnect();
      clientSocket->disconnectFromHost();
      clientSocket->deleteLater();
      mClientMap.remove(id);
      mClientIdMap.remove(clientSocket);
    }
  } else if (type == eServerData) {
    auto itr = mClientMap.find(id);
    if (itr != mClientMap.end()) {
      QTcpSocket* clientSocket = itr.value();
      qDebug() << id << ">>" << data.size();
      clientSocket->write(data);
    } else {
      qDebug() << id << "no client";
    }
  }
}

void RouterServer::ProcessClientNewConnection()
{
  int id = mLastClientId;
  QByteArray recepientPacket;
  QDataStream outStream(&recepientPacket, QIODevice::WriteOnly);
  outStream.writeRawData(kRecepientMagic.constData(), kRecepientMagic.size());
  outStream << (int)eConnection;
  outStream << id;
  outStream << QByteArray();
  SendAllRecepient(recepientPacket);
}

void RouterServer::SendAllRecepient(const QByteArray& packet)
{
  for (auto itr = mRecepientList.begin(); itr != mRecepientList.end(); itr++) {
    QTcpSocket* socket = *itr;
    socket->write(packet);
  }
}

void RouterServer::OnNewConnection()
{
  while (mServer->hasPendingConnections()) {
    QTcpSocket* socket = mServer->nextPendingConnection();
    mSocketMap[socket] = eNew;

    qDebug() << "connected" << socket->peerAddress() << ":" << socket->peerPort();

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
  while (socket->bytesAvailable() > 0) {
    switch (type) {
    case eNew      : ProcessNewPacket(socket); break;
    case eClient   : ProcessClientPacket(socket); break;
    case eRecepient: ProcessRecepientPacket(socket); break;
    }
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
      qDebug() << "client" << id << "disconnected";
      mClientIdMap.erase(itr);
      mClientMap.remove(id);

      QByteArray recepientPacket;
      QDataStream outStream(&recepientPacket, QIODevice::WriteOnly);
      outStream.writeRawData(kRecepientMagic.constData(), kRecepientMagic.size());
      outStream << (int)eDisconnection;
      outStream << id;
      outStream << QByteArray();
      SendAllRecepient(recepientPacket);
    }
  }
  mSocketMap.remove(socket);
}

RouterServer::RouterServer(QObject* parent)
  : QObject(parent)
  , mServer(new QTcpServer(this))
  , mLastClientId(0)
{
  connect(mServer, &QTcpServer::newConnection, this, &RouterServer::OnNewConnection);
}
