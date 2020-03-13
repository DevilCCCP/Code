#include <QSettings>

#include <QtNetwork/QTcpSocket>
#include <Lib/Include/QtAppCon.h>
#include <Lib/Ctrl/CtrlManager.h>
#include <Lib/NetServer/HttpHandler.h>
#include <Lib/NetServer/NetServer.h>

class HttpHandlerB: public HttpHandler
{
protected:
  /*override */virtual bool Get(const QString& path, const QList<QByteArray>& params, QByteArray& answer) Q_DECL_OVERRIDE
  {
    answer = QByteArray(GetHtmlHeader());
    answer.append(QString("Time: %1<br>\n").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
    answer.append(QString("Path=%1<br>\nParams:<br>\n").arg(path));
    for (auto itr = params.begin(); itr != params.end(); itr++) {
      answer.append(QString("%1<br>\n").arg(itr->constData()));
    }
    answer.append("<form enctype=\"multipart/form-data\" method=\"post\">"
                  "<p>"
                  "Type some text (if you like):<br>"
                  "<input type=\"text\" name=\"textline\" size=\"30\">"
                  "</p>"
                  "<p>"
                  "Please specify a file, or a set of files:<br>"
                  "<input type=\"file\" name=\"datafile\" size=\"40\">"
                  "</p>"
                  "<div>"
                  "<input type=\"submit\" value=\"Send\">"
                  "</div>"
                  "</form>");
    return true;
  }

  /*override */virtual bool Post(const QString& path, const QList<QByteArray>& params, const QList<File>& files, QByteArray& answer) Q_DECL_OVERRIDE
  {
    Log.Trace(QString("POST path: %1, params: %2, files: %3").arg(path).arg(params.size()).arg(files.count()));
    answer = QByteArray(GetHtmlHeader());
    answer.append(QString("Time: %1<br>\n").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
    answer.append(QString("Path=%1<br>\nParams:<br>\n").arg(path));
    for (auto itr = params.begin(); itr != params.end(); itr++) {
      answer.append(QString("%1<br>\n").arg(itr->constData()));
    }
    for (int i = 0; i < files.size(); i++) {
      const File& file = files[i];
      answer.append(QString("File: Type: %1, Name: %2, Size: %3<br>\n").arg(file.Type).arg(file.Name).arg(file.Data.size()));
    }
    return true;
  }
};

int qmain(int argc, char* argv[])
{
  QString path = (argc == 2)? QString(argv[1]): qApp->applicationDirPath() + "/.NetServerSettings";
  QSettings settingFile(path, QSettings::IniFormat);

  QVariant devil = settingFile.value("Devil");
  int port = settingFile.value("Port", 0).toInt();
  if (devil.isNull()) {
    Log.Info("Create default settings");
    settingFile.setValue("Devil", "exists");
    settingFile.setValue("Port", 9999);
    settingFile.sync();
    return 0;
  }

  CtrlManager manager(true);
  NetServerS srv = NetServerS(new NetServer(port, HandlerManagerS(new HandlerManagerB<HttpHandlerB>())));
  manager.RegisterWorker(srv);
  return manager.Run();
}

