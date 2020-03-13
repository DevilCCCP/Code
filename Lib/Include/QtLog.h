#pragma once
#include <Lib/Log/Log.h>


const int kFatalOnTooManyQtWarnings = 200;

#if QT_VERSION >= 0x050000
void LogMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  static int count = 0;

  if (msg == QStringLiteral("QObject::connect: Cannot connect (null)::stateChanged(QNetworkSession::State) to QNetworkReplyHttpImpl::_q_networkSessionStateChanged(QNetworkSession::State)")
    || msg.startsWith("QString::arg")) {
    //known Qt 5.5 bug
    return;
  }
  QString info = QString("Qt message (text: '%1', file: '%2', line: %3, func: '%4')").arg(msg).arg(context.file).arg(context.line).arg(context.function);

  switch (type) {
  case QtDebugMsg:    Log.Trace(info);   count += 0;  break;
  case QtWarningMsg:  Log.Warning(info); count += 1;  break;
  case QtCriticalMsg: Log.Error(info);   count += 5;  break;
  case QtFatalMsg:    Log.Fatal(info);   count += 20; break;
  default:            Log.Trace(info);   count += 0;  break;
  }

  if (count > kFatalOnTooManyQtWarnings) {
    Log.Fatal(QString("Too many Qt messages, something going wrong"), true);
  }
}
#else
void LogMessageHandler(QtMsgType type, const char * msg)
{
  static int count = 0;
  if (++count > kFatalOnTooManyQtWarnings) exit(-2);
  switch (type) {
  case QtDebugMsg:    Log.Trace(msg); break;
  case QtWarningMsg:  Log.Warning(msg); break;
  case QtCriticalMsg: Log.Error(msg); break;
  case QtFatalMsg:    Log.Fatal(msg); break;
  }
}
#endif
