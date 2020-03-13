#include <Lib/Log/Log.h>
#include <Lib/Db/Db.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Settings/FileSettings.h>
#include <Lib/Common/Var.h>
#include <Lib/Dispatcher/Overseer.h>
#include <LibV/Player/Downloader.h>
#include <LibV/Include/ModuleNames.h>

#include "DownloadDialog.h"


int DownloadPlayer(const QString& params, const DbS& db, const OverseerS& overseer)
{
  QStringList plist = params.split(";");
  if (plist.size() < 3) {
    Log.Fatal("Invalid parameters count");
    return -3001;
  }
  bool ok;
  int id = plist[0].toInt(&ok);
  QDateTime startTs = QDateTime::fromString(plist[1], Qt::ISODate);
  QDateTime endTs = QDateTime::fromString(plist[2], Qt::ISODate);
  int speed = (plist.size() >= 4)? plist[3].toInt(): 0;
  if (speed == 0) {
    speed = 4;
  }
  if (!ok || !startTs.isValid() || !endTs.isValid()) {
    Log.Fatal("Invalid parameters");
    return -3002;
  }

  ObjectType objectType(*db);
  objectType.Reload();
  const NamedItem* camType = objectType.GetItemByName("cam");
  const NamedItem* srvType = objectType.GetItemByName("srv");
  if (!camType || !srvType) {
    Log.Fatal("Can't get camera type");
    return -3101;
  }
  auto q = db->MakeQuery();
  q->prepare(QString("SELECT o._id, o.name FROM object o"
                     " INNER JOIN object_connection c ON c._oslave = o._id"
                     " INNER JOIN object om ON om._id = c._omaster"
                     " WHERE o._otype = %2 AND om._otype = %1;")
             .arg(srvType->Id).arg(camType->Id));

  if (!db->ExecuteQuery(q)) {
    Log.Fatal("Can't get cameras list");
    return -3102;
  }
  QList<QPair<int, QString> > cameras;
  while (q->next()) {
    int id = q->value(0).toInt();
    QString name = q->value(1).toString();
    QPair<int, QString> p = qMakePair(id, name);
    cameras.append(p);
  }

  FileSettings ini;
  ini.SetSilent(true);
  ini.Open(kArmDaemon);
  QString path = ini.GetValue("DownloadDir", QString(GetVarPath() + "/Downloads/")).toString();

  DownloadDialog dlg(cameras);
  dlg.SetCameraId(id);
  dlg.SetSpeed(speed);
  dlg.SetTimeStart(startTs);
  dlg.SetTimeEnd(endTs);
  dlg.SetPath(path);
  if (!dlg.exec()) {
    return 0;
  }
  id = dlg.GetCameraId();
  speed = dlg.GetSpeed();
  startTs = dlg.GetTimeStart();
  endTs = dlg.GetTimeEnd();
  path = dlg.GetPath();
  if (dlg.GetPathAsDefault()) {
    ini.SetValue("DownloadDir", path);
    ini.Sync();
  }

  qint64 start = startTs.toMSecsSinceEpoch();
  qint64 end = endTs.toMSecsSinceEpoch();
  Log.Info(QString("Downloading video (camera:%1, tsFrom: %2, tsTo: %3)").arg(id)
           .arg(QDateTime::fromMSecsSinceEpoch(start).toString(Qt::ISODate))
           .arg(QDateTime::fromMSecsSinceEpoch(end).toString(Qt::ISODate)));
  DownloaderS downloader(new Downloader(db, id, startTs.toMSecsSinceEpoch(), endTs.toMSecsSinceEpoch(), speed, path, false, true));
  overseer->RegisterWorker(downloader);
  return overseer->Run();
}
