#include <QDir>
#include <QStandardPaths>

#include "MainWindow.h"
#include "ui_MainWindow.h"


const int kMaxRecentList = 10;

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
  , mTcpServer(new QTcpServer(this)), mConnection(0)
{
  ui->setupUi(this);

  ui->pushButtonStop->setVisible(false);

  QString iniFilePath = QDir(QStandardPaths::writableLocation(
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
                               QStandardPaths::AppDataLocation
#else
                               QStandardPaths::DataLocation
#endif
                               )).absoluteFilePath("main_wnd.ini");
  mSettings = new QSettings(iniFilePath, QSettings::IniFormat, this);
  int proxyPort = mSettings->value("ProxyPort", 7001).toInt();

  ui->spinBoxPort->setValue(proxyPort);

  mTcpServer = new QTcpServer(this);
  connect(mTcpServer, &QTcpServer::newConnection, this, &MainWindow::OnNewConnection);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::UpdateLog()
{
  ui->textEditLog->setHtml(mLog);
  QTextCursor cursor = ui->textEditLog->textCursor();
  cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
  ui->textEditLog->setTextCursor(cursor);
}

void MainWindow::DisconnectAll()
{
  for (auto itr = mClientMap.begin(); itr != mClientMap.end(); itr++) {
    QTcpSocket* connection = itr.value();
    connection->disconnectFromHost();
    connection->deleteLater();
    mClientMap.erase(itr);
  }
  mClientMap.clear();

  for (auto itr = mProxyMap.begin(); itr != mProxyMap.end(); itr++) {
    QTcpSocket* proxy = itr.value();
    proxy->disconnectFromHost();
    proxy->deleteLater();
    mProxyMap.erase(itr);
  }
  mProxyMap.clear();
}

void MainWindow::Disconnect(int id)
{
  DisconnectClient(id);
  DisconnectProxy(id);
}

void MainWindow::DisconnectClient(int id)
{
  auto itr = mClientMap.find(id);
  if (itr != mClientMap.end()) {
    QTcpSocket* connection = itr.value();
    connection->disconnect();
    connection->disconnectFromHost();
    connection->deleteLater();
    mClientMap.erase(itr);
  }
}

void MainWindow::DisconnectProxy(int id)
{
  auto itr = mProxyMap.find(id);
  if (itr != mProxyMap.end()) {
    QTcpSocket* proxy = itr.value();
    proxy->disconnect();
    proxy->disconnectFromHost();
    proxy->deleteLater();
    mProxyMap.erase(itr);
  }
}

void MainWindow::OnNewConnection()
{
  int id = ++mConnection;
  QTcpSocket* connection = mTcpServer->nextPendingConnection();
  connection->setProperty("Id", id);
  connection->setProperty("State", (int)ePsNone);
  mClientMap[id] = connection;

  connect(connection, &QTcpSocket::readyRead, this, &MainWindow::OnClientReadyRead);
  connect(connection, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError socketError)>(&QTcpSocket::error)
          , this, &MainWindow::OnClientError);
  connect(connection, &QTcpSocket::disconnected, this, &MainWindow::OnClientDisconnected);

  mLog.append(QString("<p style=\"color:green\">%0: connection from %1:%2</p>")
              .arg(id).arg(connection->peerAddress().toString()).arg(connection->peerPort()));

  UpdateLog();
}

void MainWindow::OnClientReadyRead()
{
  QTcpSocket* connection = qobject_cast<QTcpSocket*>(sender());
  if (!connection) {
    return;
  }

  int id = connection->property("Id").toInt();
  ProxyState state = (ProxyState)connection->property("State").toInt();

  QByteArray data = connection->readAll();
  if (state == ePsNone) {
    if (data.size() < 3 || data.at(0) != 0x05) {
      Disconnect(id);
      return;
    }
    QByteArray grant0("\x05\x00", 2);
    connection->setProperty("State", (int)ePsHello);
    connection->write(grant0);
    mLog.append(QString("<p>%0: hello</p>").arg(id));
  } else if (state == ePsHello) {
    if (data.size() < 4 || data.at(0) != 0x05) {
      Disconnect(id);
      return;
    }
    int type = (uchar)data.at(1);
    int atype = (uchar)data.at(3);
    if (type != 1) {
      Disconnect(id);
      return;
    }

    QHostAddress hostAddress;
    QString hostName;
    int index = 4;
    if (atype == 1) {
      if (data.size() < 4 + 4 + 2) {
        Disconnect(id);
        return;
      }

      quint32 ipv4 = 0;
      for (int i = 0; i < 4; i++) {
        ipv4 |= ((quint32)(uchar)data.at(index + i) << (24 - i*8));
      }
      hostAddress.setAddress(ipv4);
      index += 4;
    } else if (atype == 3) {
      int len = (uchar)data.at(index++);
      if (data.size() < 4 + 1 + len + 2) {
        Disconnect(id);
        return;
      }
      hostName = QString::fromUtf8(data.mid(index, len));
      index += len;
    } else {
      Disconnect(id);
      return;
    }

    int port = 0;
    for (int i = 0; i < 2; i++) {
      port |= ((int)(uchar)data.at(index + i) << (8 - i*8));
    }

    auto itr = mProxyMap.find(id);
    if (itr != mProxyMap.end()) {
      QTcpSocket* proxy = itr.value();
      proxy->disconnectFromHost();
      proxy->deleteLater();
    }

    QTcpSocket* proxy = new QTcpSocket(this);
    proxy->setProperty("Id", id);
    mProxyMap[id] = proxy;

    connect(proxy, &QTcpSocket::connected, this, &MainWindow::OnProxyConnected);
    connect(proxy, &QTcpSocket::readyRead, this, &MainWindow::OnProxyReadyRead);
    connect(proxy, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError socketError)>(&QTcpSocket::error)
            , this, &MainWindow::OnProxyError);
    connect(proxy, &QTcpSocket::disconnected, this, &MainWindow::OnProxyDisconnected);

    if (!hostName.isEmpty()) {
      proxy->connectToHost(hostName, port);
    } else {
      proxy->connectToHost(hostAddress, port);
    }

    connection->setProperty("State", (int)ePsConnect);
    proxy->setProperty("State", (int)ePsConnect);
    mLog.append(QString("<p>%0: connecting</p>").arg(id));
  } else if (state == ePsConnected) {
    auto itr = mProxyMap.find(id);
    if (itr == mProxyMap.end()) {
      DisconnectClient(id);
      return;
    }

    QTcpSocket* proxy = itr.value();
    proxy->write(data);
    mLog.append(QString("<p>%0: write %1</p>").arg(id).arg(data.size()));
  } else {
    mLog.append(QString("<p>%0: bad state write</p>").arg(id));
  }

  UpdateLog();
}

void MainWindow::OnClientError(QAbstractSocket::SocketError)
{
  QTcpSocket* connection = qobject_cast<QTcpSocket*>(sender());
  if (!connection) {
    return;
  }

  int id = connection->property("Id").toInt();

  mLog.append(QString("<p>%0: client error ('%1')</p>").arg(id).arg(connection->errorString()));
  UpdateLog();

  Disconnect(id);
}

void MainWindow::OnClientDisconnected()
{
  QTcpSocket* connection = qobject_cast<QTcpSocket*>(sender());
  if (!connection) {
    return;
  }

  int id = connection->property("Id").toInt();

  mLog.append(QString("<p>%0: client disconnected ('%1')</p>").arg(id));
  UpdateLog();

  Disconnect(id);
}

void MainWindow::OnProxyConnected()
{
  QTcpSocket* proxy = qobject_cast<QTcpSocket*>(sender());
  if (!proxy) {
    return;
  }

  int id = proxy->property("Id").toInt();
  ProxyState state = (ProxyState)proxy->property("State").toInt();

  if (state == ePsConnect) {
    auto itr = mClientMap.find(id);
    if (itr == mClientMap.end()) {
      DisconnectProxy(id);
      return;
    }

    QTcpSocket* connection = itr.value();
    QByteArray grant1("\x05\x00\x00", 3);
    QHostAddress peerAddr = proxy->peerAddress();
    bool ok = false;
    quint32 ipv4 = peerAddr.toIPv4Address(&ok);
    if (ok) {
      grant1.append(0x01);
      for (int i = 0; i < 4; i++) {
        uchar b = (uchar)(ipv4 >> (24 - i*8));
        grant1.append(b);
      }
    } else {
      grant1.append(0x04);
      Q_IPV6ADDR ipv6 = peerAddr.toIPv6Address();
      for (int i = 0; i < 16; i++) {
        grant1.append(ipv6[i]);
      }
    }

    int port = proxy->peerPort();
    for (int i = 0; i < 2; i++) {
      uchar b = (uchar)(port >> (8 - i*8));
      grant1.append(b);
    }

    connection->write(grant1);
    connection->setProperty("State", (int)ePsConnected);
    proxy->setProperty("State", (int)ePsConnected);

    mLog.append(QString("<p>%0: proxy connected</p>").arg(id));
  } else {
    mLog.append(QString("<p>%0: proxy bas state connect</p>").arg(id));
  }

  UpdateLog();
}

void MainWindow::OnProxyReadyRead()
{
  QTcpSocket* proxy = qobject_cast<QTcpSocket*>(sender());
  if (!proxy) {
    return;
  }

  int id = proxy->property("Id").toInt();
  ProxyState state = (ProxyState)proxy->property("State").toInt();

  if (state == ePsConnected) {
    auto itr = mClientMap.find(id);
    if (itr == mClientMap.end()) {
      DisconnectProxy(id);
      return;
    }
    QTcpSocket* connection = itr.value();

    QByteArray data = proxy->readAll();
    mLog.append(QString("<p>%0: read %1</p>").arg(id).arg(data.size()));
    connection->write(data);
  } else {
    mLog.append(QString("<p>%0: bad state read</p>").arg(id));
  }
  UpdateLog();
}

void MainWindow::OnProxyError(QAbstractSocket::SocketError)
{
  QTcpSocket* proxy = qobject_cast<QTcpSocket*>(sender());
  if (!proxy) {
    return;
  }

  int id = proxy->property("Id").toInt();

  mLog.append(QString("<p>%0: host error ('%1')</p>").arg(id).arg(proxy->errorString()));
  UpdateLog();

  Disconnect(id);
}

void MainWindow::OnProxyDisconnected()
{
  QTcpSocket* proxy = qobject_cast<QTcpSocket*>(sender());
  if (!proxy) {
    return;
  }

  int id = proxy->property("Id").toInt();

  mLog.append(QString("<p>%0: host disconnected</p>").arg(id));
  UpdateLog();

  Disconnect(id);
}

void MainWindow::on_pushButtonStart_clicked()
{
  int proxyPort = ui->spinBoxPort->value();
  mSettings->setValue("ProxyPort", proxyPort);
  mSettings->sync();

  bool ok = mTcpServer->listen(QHostAddress::Any, proxyPort);

  mLog.clear();
  mLog.append(QString("<p>listen %1</p>").arg(ok? "ok": "fail"));
  UpdateLog();

  ui->pushButtonStart->setVisible(false);
  ui->pushButtonStop->setVisible(true);
}

void MainWindow::on_pushButtonStop_clicked()
{
  DisconnectAll();

  mTcpServer->close();

  ui->pushButtonStart->setVisible(true);
  ui->pushButtonStop->setVisible(false);
}
