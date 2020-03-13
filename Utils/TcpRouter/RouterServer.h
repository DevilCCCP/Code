#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include "Def.h"


enum EType {
  eNew,
  eClient,
  eRecepient
};

class RouterServer: public QObject
{
  QTcpServer*              mServer;

  QMap<int, QTcpSocket*>   mClientMap;
  QMap<QTcpSocket*, int>   mClientIdMap;
  QList<QTcpSocket*>       mRecepientList;
  QMap<QTcpSocket*, EType> mSocketMap;
  int                      mLastClientId;

  Q_OBJECT

public:
  bool Start(int port);

private:
  void ProcessNewPacket(QTcpSocket* socket);
  void ProcessClientPacket(QTcpSocket* socket);
  void ProcessRecepientPacket(QTcpSocket* socket);
  void ProcessClientNewConnection();
  void SendAllRecepient(const QByteArray& packet);

private slots:
  void OnNewConnection();
  void OnReadyRead();
  void OnDisconnected();

public:
  RouterServer(QObject* parent = nullptr);
};
