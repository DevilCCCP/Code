#include <QApplication>

#include "MainWindow.h"


void LogMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  if (msg == QStringLiteral("QObject::connect: Cannot connect (null)::stateChanged(QNetworkSession::State) to QNetworkReplyHttpImpl::_q_networkSessionStateChanged(QNetworkSession::State)")
    || msg.startsWith("QString::arg")) {
    //known Qt 5.5 bug
    return;
  }
  QString info = QString("Qt message (text: '%1', file: '%2', line: %3, func: '%4')").arg(msg).arg(context.file).arg(context.line).arg(context.function);

  switch (type) {
  case QtDebugMsg   : printf("Debug: %s\n", info.toLatin1().constData()); break;
  case QtWarningMsg : printf("Warng: %s\n", info.toLatin1().constData()); break;
  case QtCriticalMsg: printf("Error: %s\n", info.toLatin1().constData()); break;
  case QtFatalMsg   : printf("Fatal: %s\n", info.toLatin1().constData()); break;
  default           : printf("Trace: %s\n", info.toLatin1().constData()); break;
  }
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  qInstallMessageHandler(LogMessageHandler);

  MainWindow w;
  w.show();

  return app.exec();
}
