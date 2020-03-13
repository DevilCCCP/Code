#include <QTcpSocket>
#include <QHostAddress>
#include <QDataStream>

#include <Lib/Log/Log.h>

#include "RouterClient.h"


const int kServerHelloMs = 30 * 1000;
const int kServerTimeoutMs = 45 * 1000;

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
  mServerTimeout.restart();
  mServer->connectToHost(mHost, mHostPort);
}

bool RouterClient::ProcessServerPacket()
{
  switch (mServerPacket.getType()) {
  case eGrant        : ProcessServerGrant(); break;
  case eConnection   : ProcessServerConnection(mServerPacket.getId()); break;
  case eDisconnection: ProcessServerDisconnection(mServerPacket.getId()); break;
  case eClientData   : ProcessServerClientData(mServerPacket.getId(), mServerPacket.getData()); break;
  default:
    Log.Warning(QString("Unexpected server packet (type: %1)").arg((int)mServerPacket.getType()));
    break;
  }
  return true;
}

void RouterClient::ProcessServerGrant()
{
}

void RouterClient::ProcessServerConnection(int id)
{
  if (mDebug) {
    Log.Info(QString("%1 connect").arg(id));
  }
  QTcpSocket* client = new QTcpSocket(this);
  connect(client, &QTcpSocket::connected, this, &RouterClient::OnClientConnected);
  connect(client, &QTcpSocket::disconnected, this, &RouterClient::OnClientDisconnected);
  connect(client, &QTcpSocket::readyRead, this, &RouterClient::OnClientReadyRead);
  mClientMap[id] = client;
  mClientIdMap[client] = id;
  client->connectToHost(QHostAddress(QHostAddress::LocalHost), mPort);
}

void RouterClient::ProcessServerDisconnection(int id)
{
  if (mDebug) {
    Log.Info(QString("%1 disconnect").arg(id));
  }
  auto itr = mClientMap.find(id);
  if (itr != mClientMap.end()) {
    QTcpSocket* clientSocket = itr.value();
    clientSocket->disconnect();
    clientSocket->disconnectFromHost();
    clientSocket->deleteLater();
    mClientMap.remove(id);
    mClientIdMap.remove(clientSocket);
  }
}

void RouterClient::ProcessServerClientData(int id, const QByteArray& data)
{
  auto itr = mClientMap.find(id);
  if (itr != mClientMap.end()) {
    QTcpSocket* clientSocket = itr.value();
    if (clientSocket->state() == QTcpSocket::ConnectedState) {
      if (mDebug) {
        Log.Info(QString("%1 << %2").arg(id).arg(data.size()));
      }
      clientSocket->write(data);
    } else {
      mClientData[clientSocket].append(data);
    }
  }
}

void RouterClient::ProcessClientPacket(QTcpSocket* socket, const QByteArray& data)
{
  auto itr = mClientIdMap.find(socket);
  if (itr != mClientIdMap.end()) {
    int id = itr.value();
    QDataStream outStream(mServer);
    Packet packet;
    packet.setType(eServerData);
    packet.setId(id);
    packet.setData(data);
    packet.Write(&outStream);
    if (mDebug) {
      Log.Info(QString("%1 >> %2").arg(id).arg(data.size()));
    }
  }
}

void RouterClient::OnServerConnected()
{
  Log.Info(QString("Server connected"));
  mServerHello.restart();
  mServerTimeout.restart();
  Packet packet;
  packet.setType(eHello);
  QDataStream outStream(mServer);
  if (!packet.Write(&outStream)) {
    mServer->disconnectFromHost();
  }
}

void RouterClient::OnServerDisconnected()
{
  Log.Info(QString("Server disconnected"));
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
  while (mServer->bytesAvailable() >= mServerPacket.MinimumSize()) {
    bool ok = false;
    QDataStream stream(mServer);
    if (mServerPacket.HasHeader()) {
      ok = mServerPacket.ReadData(&stream);
      if (!ok && mDebug) {
        Log.Warning("Server data fail");
      }
      if (ok) {
        ok = ProcessServerPacket();
        mServerHello.restart();
        mServerTimeout.restart();
      }
      mServerPacket.Clear();
    } else {
      ok = mServerPacket.ReadHeader(&stream);
      if (!ok && mDebug) {
        Log.Warning("Server header fail");
      }
    }

    if (!ok) {
      mServer->disconnectFromHost();
    }
  }
}

void RouterClient::OnClientConnected()
{
  QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
  if (!socket) {
    return;
  }
  if (mDebug) {
    Log.Info(QString("%1 connected from %2:%3").arg(mClientIdMap[socket]).arg(socket->peerName()).arg(socket->peerPort()));
  }

  auto itr = mClientData.find(socket);
  if (itr != mClientData.end()) {
    const QByteArray& data = itr.value();
    if (mDebug) {
      Log.Info(QString("%1 << %2").arg(mClientIdMap[socket]).arg(data.size()));
    }
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

  if (mDebug) {
    Log.Info(QString("%1 disconnected").arg(mClientIdMap[socket]));
  }
  mClientData.remove(socket);
  auto itr = mClientIdMap.find(socket);
  if (itr != mClientIdMap.end()) {
    int id = itr.value();

    QDataStream outStream(mServer);
    Packet packet;
    packet.setType(eDisconnection);
    packet.setId(id);
    packet.Write(&outStream);

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
  if (mServerHello.elapsed() > kServerHelloMs) {
    if (mDebug) {
      Log.Info(QString("Hello"));
    }
    Packet packet;
    packet.setType(eHello);
    QDataStream outStream(mServer);
    packet.Write(&outStream);
    mServerHello.restart();
  }
  if (mServerTimeout.elapsed() > kServerTimeoutMs) {
    Log.Warning(QString("Server timeout"));
    mServer->blockSignals(true);
    mServer->disconnectFromHost();
    mServer->blockSignals(false);
    OnServerDisconnected();
    mServerTimeout.restart();
  }
  if (mServer->state() != QTcpSocket::ConnectedState && mServer->state() != QTcpSocket::ConnectingState) {
    ConnectToHost();
  }

  for (auto itr = mClientMap.begin(); itr != mClientMap.end(); ) {
    QTcpSocket* client = itr.value();
    int id = itr.key();
    if (client->state() != QTcpSocket::ConnectedState && client->state() != QTcpSocket::ConnectingState) {
      if (mDebug) {
        Log.Info(QString("%1 lost").arg(id));
      }
      itr = mClientMap.erase(itr);
      mClientIdMap.remove(client);
      mClientData.remove(client);
      client->deleteLater();

      QDataStream outStream(mServer);
      Packet packet;
      packet.setType(eDisconnection);
      packet.setId(id);
      packet.Write(&outStream);
    } else {
      itr++;
    }
  }
}


RouterClient::RouterClient(QObject* parent)
  : QObject(parent)
  , mDebug(false)
  , mServer(new QTcpSocket(this)), mPort(0), mConnectTimer(new QTimer(this))
{
  mServerTimeout.start();
  mServerHello.start();
  connect(mServer, &QTcpSocket::connected, this, &RouterClient::OnServerConnected);
  connect(mServer, &QTcpSocket::disconnected, this, &RouterClient::OnServerDisconnected);
  connect(mServer, &QTcpSocket::readyRead, this, &RouterClient::OnServerReadyRead);
  connect(mConnectTimer, &QTimer::timeout, this, &RouterClient::OnConnectTimerTimeout);
  mConnectTimer->setInterval(500);
}
