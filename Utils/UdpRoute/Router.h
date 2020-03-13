#pragma once

#include <QObject>
#include <QVector>
#include <QUdpSocket>


class Router: public QObject
{
  QUdpSocket*          mBindSocket;
  QVector<int>         mSendPortList;
  QVector<QUdpSocket*> mSendSocket;

  Q_OBJECT

public:
  explicit Router(QObject* parent = 0);
  ~Router();

public slots:
  void Start(int bindPort, const QVector<int>& sendPortList);
  void Stop();

private slots:
  void OnReadyRead();

signals:
  void Started();
  void Error(const QString& text);
  void ProcessFrame();
};
