#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTcpServer>
#include <QByteArray>
#include <QDebug>

#include "RouterServer.h"
#include "RouterClient.h"


int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  QCoreApplication::setApplicationName("TCP router");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription(QString("TCP routing work as server and client.\n"
                                           "As server it redirects any connection to client "
                                           "cloning any sent/received data.\n"
                                           "As client it connects to server and redirect connections "
                                           "with that server to local TCP port."));
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("type", "One of 'server' or 'client'");
  parser.addPositionalArgument("port", "Local server port");
  parser.addPositionalArgument("address", "Server address in host:port format (for client only)", "[address]");

  parser.process(a);

  const QStringList args = parser.positionalArguments();
  if (args.isEmpty()) {
    parser.showHelp(0);
  }
  if (args.size() < 2) {
    parser.showHelp(1);
  }
  QString type = args.at(0);
  if (type == "server") {
    if (args.size() != 2) {
      parser.showHelp(3);
    }
    int port = args.at(1).toInt();
    if (!port) {
      parser.showHelp(5);
    }
    RouterServer* router = new RouterServer(&a);
    if (!router->Start(port)) {
      qDebug() << "Server start fail, port:" << port;
      return 3;
    }
    qDebug() << "Server started at port:" << port;
  } else if (type == "client") {
    if (args.size() != 3) {
      parser.showHelp(4);
    }
    int port = args.at(1).toInt();
    if (!port) {
      parser.showHelp(6);
    }
    QRegExp hostPortRegExp("(.*):(\\d+)");
    if (!hostPortRegExp.exactMatch(args.at(2))) {
      parser.showHelp(7);
    }
    QString host = hostPortRegExp.cap(1);
    int hostPort = hostPortRegExp.cap(2).toInt();
    RouterClient* router = new RouterClient(&a);
    if (!router->Start(port, host, hostPort)) {
      qDebug() << "Client start fail, port:" << port;
      return 3;
    }
    qDebug() << "Client started with port:" << port << "server" << host << ":" << hostPort;
  } else {
    parser.showHelp(2);
  }

  return a.exec();
}
