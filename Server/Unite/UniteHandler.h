#pragma once

#include <QFile>

#include <Lib/Db/Db.h>
#include <Lib/NetServer/HttpHandler.h>
#include <Lib/NetServer/NetServer.h>


DefineClassS(UniteHandler);
DefineClassS(UniteInfo);

class UniteHandler: public HttpHandler
{
  UniteInfo*  mUniteInfo;

  QByteArray  mCurrentUuid;
  ObjectItemS mCurrentObject;

protected:
  /*override */virtual bool Get(const QString& path, const QList<QByteArray>& params) override;
  /*override */virtual bool Post(const QString& path, const QList<QByteArray>& params, const QList<File>& files) override;

protected:
  bool ValidateSign(const QList<QByteArray>& params, const QList<File>& files);
  bool LoadObject();
  bool VerifySign(const QByteArray& data, const QByteArray& sign);

  bool QuerySegment(const QByteArray& data);
  bool QueryObject(const QByteArray& data);
  bool QueryEvents();
  bool UpdateObject(const QByteArray& data);
  bool UpdateObjectOne();
  bool UpdateEvents(const QByteArray& data);
  bool UpdateParentConnection();
  bool RemoveUnused(const QSet<int>& usedObjs, int& objRemoved);

  bool GetObjectTypeId(const QString& typeName, int& id);
  bool CreateEvent(const QString& objUuid, const QString& eventName, int& localObjId, int& localEventId);
  bool CreateEventLogOne(int objectId, int eventId, const QDateTime& timestamp, int value, const QByteArray& data, const qint64& topEvent);
  bool CreateEventLog(const QVariantList& eventIds, const QVariantList& timestamps, const QVariantList& values, const qint64& topEvent);

public:
  UniteHandler(UniteInfo* _UniteInfo);
  /*override */virtual ~UniteHandler();
};
