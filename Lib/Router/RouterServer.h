#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include <Lib/Include/Common.h>

#include "Packet.h"


enum EType {
  eNew,
  eClient,
  eRecepient
};

class RouterServer: public QObject
{
  PROPERTY_GET_SET(bool,    Debug)

  QTcpServer*               mServer;

  QMap<int, QTcpSocket*>    mClientMap;
  QMap<QTcpSocket*, int>    mClientIdMap;
  QMap<QTcpSocket*, Packet> mRecepientPacketMap;
  QMap<QTcpSocket*, EType>  mSocketMap;
  int                       mLastClientId;

  Q_OBJECT

public:
  bool Start(int port);

private:
  void ProcessNew(QTcpSocket* socket);
  void ProcessNewRecepient(QTcpSocket* socket);
  void ProcessNewClient(QTcpSocket* socket, const QByteArray& data);
  void ProcessClient(QTcpSocket* socket);
  void ProcessRecepient(QTcpSocket* socket);
  bool ProcessRecepientPacket(QTcpSocket* socket, const Packet& packet);
  void ProcessRecepientHello(QTcpSocket* socket);
  void ProcessRecepientDisconnection(int id);
  void ProcessRecepientServerData(int id, const QByteArray& data);
  void ProcessClientNewConnection(int id);
  void SendAllRecepient(const Packet& packet);

private slots:
  void OnNewConnection();
  void OnReadyRead();
  void OnDisconnected();

public:
  RouterServer(QObject* parent = nullptr);
};
