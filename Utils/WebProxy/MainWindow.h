#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>


namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow
{
  Ui::MainWindow* ui;
  QSettings*      mSettings;

  enum ProxyState {
    ePsNone,
    ePsHello,
    ePsConnect,
    ePsConnected
  };

  typedef QMap<int, QTcpSocket*> SocketMap;

  QTcpServer*     mTcpServer;
  SocketMap       mClientMap;
  SocketMap       mProxyMap;
  int             mConnection;
  QString         mLog;

  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

private:
  void UpdateLog();

  void DisconnectAll();
  void Disconnect(int id);
  void DisconnectClient(int id);
  void DisconnectProxy(int id);

private:
  void OnNewConnection();
  void OnClientReadyRead();
  void OnClientError(QAbstractSocket::SocketError);
  void OnClientDisconnected();
  void OnProxyConnected();
  void OnProxyReadyRead();
  void OnProxyError(QAbstractSocket::SocketError);
  void OnProxyDisconnected();

private slots:
  void on_pushButtonStart_clicked();
  void on_pushButtonStop_clicked();
};
