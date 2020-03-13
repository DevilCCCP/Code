#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>

#include <Def.h>


class RouterClient: public QObject
{
  QTcpSocket*                   mServer;
  int                           mPort;
  QString                       mHost;
  int                           mHostPort;
  bool                          mGranted;
  QTimer*                       mConnectTimer;

  QMap<int, QTcpSocket*>        mClientMap;
  QMap<QTcpSocket*, int>        mClientIdMap;
  QMap<QTcpSocket*, QByteArray> mClientData;

  Q_OBJECT

public:
  bool Start(int port, const QString& host, int hostPort);

private:
  void ConnectToHost();
  void ProcessServerPacket(QDataStream* stream);
  void ProcessClientPacket(QTcpSocket* socket, const QByteArray& packet);

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
