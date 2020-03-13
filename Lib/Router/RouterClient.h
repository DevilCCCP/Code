#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QElapsedTimer>

#include <Lib/Include/Common.h>

#include "Packet.h"


class RouterClient: public QObject
{
  PROPERTY_GET_SET(bool,        Debug)

  QTcpSocket*                   mServer;
  QElapsedTimer                 mServerHello;
  QElapsedTimer                 mServerTimeout;
  int                           mPort;
  QString                       mHost;
  int                           mHostPort;
  QTimer*                       mConnectTimer;

  QMap<int, QTcpSocket*>        mClientMap;
  QMap<QTcpSocket*, int>        mClientIdMap;
  QMap<QTcpSocket*, QByteArray> mClientData;
  Packet                        mServerPacket;

  Q_OBJECT

public:
  bool Start(int port, const QString& host, int hostPort);

private:
  void ConnectToHost();
  bool ProcessServerPacket();
  void ProcessServerGrant();
  void ProcessServerConnection(int id);
  void ProcessServerDisconnection(int id);
  void ProcessServerClientData(int id, const QByteArray& data);
  void ProcessClientPacket(QTcpSocket* socket, const QByteArray& data);

private slots:
  void OnServerConnected();
  void OnServerDisconnected();
  void OnServerReadyRead();
  void OnClientConnected();
  void OnClientDisconnected();
  void OnClientReadyRead();
  void OnConnectTimerTimeout();

public:
  RouterClient(QObject* parent = nullptr);
};
