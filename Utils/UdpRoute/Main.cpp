#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "Router.h"


int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("UDP route");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription(QString("UDP route from one local port to any of others."));
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("source", "Local port to read from", "source");
  parser.addPositionalArgument("destination", "Local destination ports (any number)", "destination [destination2]...");

  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.size() <= 1) {
    parser.showHelp(1);
  }

  int srcPort = args.at(0).toInt();
  if (!srcPort) {
    parser.showHelp(2);
  }
  QVector<int> dstPortList;
  for (int i = 1; i < args.size(); i++) {
    int dstPort = args.at(i).toInt();
    if (!dstPort) {
      parser.showHelp(3);
    }
    dstPortList.append(dstPort);
  }

  Router* router = new Router(&app);

  router->Start(srcPort, dstPortList);
  return app.exec();
}
