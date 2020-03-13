#include <QCoreApplication>
#include <QNetworkInterface>
#include <QTcpSocket>

#include <Lib/Log/Log.h>
#include <Lib/Include/License.h>
#undef LICENSE_MAIN
#define LICENSE_MAIN(XXX)
#include <Lib/Include/QtAppCon.h>
#include <Lib/Include/QtLog.h>
#include <Lib/Ctrl/CtrlManager.h>
#include <Lib/NetServer/NetServer.h>
#include <Lib/Db/Db.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Settings/DbSettings.h>
#include <Lib/Settings/FileSettings.h>

#include "LicenseHandler.h"


int SaveKey();
bool RegisterLis(const QString& guid, const QString& ip, int port);

int qmain(int argc, char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  if (QCoreApplication::arguments().contains("start")) {
    CtrlManager manager(true);
    FileSettings locSettings;
    locSettings.Open(".license");
    QString guid = locSettings.GetMandatoryValue("Guid").toString();
    if (guid.isEmpty()) {
      return -1;
    }
    QString ip = locSettings.GetValue("IP").toString();
    int port = locSettings.GetValue("Port", 8080).toInt();
    for (; true; port++) {
      QTcpSocket socket;
      if (socket.bind(port, QAbstractSocket::DontShareAddress)) {
        Log.Info(QString("Using port: %1").arg(port));
        break;
      }
    }

    if (!RegisterLis(guid, ip, port)) {
      Log.Fatal(QString("Register service fail"));
      return -1;
    }
    NetServerS srv = NetServerS(new NetServer(port, HandlerManagerS(new HandlerManagerB<LicenseHandler>())));
    manager.RegisterWorker(srv);
    return manager.Run();
  } else {
    return SaveKey();
  }
}

bool RegisterLis(const QString& guid, const QString& ip, int port)
{
  Db                     mDb;
  ObjectTypeTableS       mObjectTypeTable(new ObjectTypeTable(mDb));
  ObjectTableS           mObjectTable(new ObjectTable(mDb));
  int                    mLiloType;

  if (!mDb.OpenDefault()) {
    return false;
  }
  if (!mDb.Connect()) {
    return false;
  }

  if (const NamedItem* typeItem = mObjectTypeTable->GetItemByName("lis")) {
    mLiloType = typeItem->Id;
  } else {
    return false;
  }
  ObjectItemS object;
  if (!mObjectTable->GetObjectByGuid(guid, object)) {
    return false;
  }
  if (!object) {
    object.reset(new ObjectItem());
    object->Name = "Easy service";
    object->Guid = guid;
    object->Type = mLiloType;
    object->Status = 0;
    if (!mObjectTable->InsertItem(object)) {
      return false;
    }
    Log.Info(QString("Create new easy service (id: %1, guid: '%2'").arg(object->Id).arg(object->Guid));
  }

  QString definedIp;
  QHostAddress definedAddr(ip);
  QList<QHostAddress> allAddresses = QNetworkInterface::allAddresses();
  Log.Trace("IP query subnet");
  for (auto itr = allAddresses.begin(); itr != allAddresses.end(); itr++) {
    QHostAddress localAddr = *itr;
    if (localAddr.isInSubnet(definedAddr, 24)) {
      definedIp = localAddr.toString();
      Log.Info(QString("Address got from subnet '%1'").arg(definedIp));
      break;
    }
  }

  if (definedIp.isEmpty()) {
    Log.Trace("IP query any ipv4 non localhost");
    for (auto itr = allAddresses.begin(); itr != allAddresses.end(); itr++) {
      QHostAddress localAddr = *itr;
      if (localAddr.protocol() == QAbstractSocket::IPv4Protocol) {
        qint32 ipv4 = localAddr.toIPv4Address();
        if (ipv4 != 0x7f000001) {
          definedIp = localAddr.toString();
          Log.Info(QString("Address got first in the list '%1'").arg(definedIp));
          break;
        }
      }
    }
    if (definedIp.isEmpty()) {
      return false;
    }
  }

  DbSettings settings(mDb);
  if (!settings.Open(QString::number(object->Id))) {
    return false;
  }
  settings.SetValue("Uri", QString("http://%1:%2").arg(definedIp).arg(port));
  settings.Sync();
  return true;
}
