#pragma once

#include <QFile>

#include <Lib/Db/Db.h>
#include <Lib/NetServer/HttpHandler.h>
#include <Lib/NetServer/NetServer.h>


DefineClassS(UniteHandler);
DefineClassS(UniteInfo);
DefineClassS(UniteObject);

class UniteHandler: public HttpHandler
{
protected:
  UniteInfo*             mUniteInfo;
  DbS                    mDb;
  UniteObjectS           mUniteObject;

private:
  QByteArray             mCurrentUuid;
  QByteArray             mCurrentPkey;
  ObjectItemS            mCurrentObject;

protected:
  /*override */virtual bool Get(const QString& path, const QList<QByteArray>& params) Q_DECL_OVERRIDE;
  /*override */virtual bool Post(const QString& path, const QList<QByteArray>& params, const QList<File>& files) Q_DECL_OVERRIDE;

protected:
  bool InitDb();

  bool ValidateSign(const QList<QByteArray>& params, const QList<File>& files);
  bool LoadObject();
  bool VerifySign(const QByteArray& data, const QByteArray& sign);
  bool VerifyPkey();

  bool QuerySegment(const QByteArray& data);
  bool QueryObject(const QByteArray& data);
  bool QueryEvents();
  bool QueryLogs();
  bool UpdateObject(const QByteArray& data);
  bool UpdateEvents(const QByteArray& data);
  bool UpdateLogs(const QByteArray& data);

  bool CreateEvent(const QString& objUuid, const QString& eventName, int& localObjId, int& localEventId);
  bool CreateEventLogOne(int objectId, int eventId, const QDateTime& timestamp, int value, const QByteArray& data, const qint64& topEvent);
  bool CreateEventLog(const QVariantList& eventIds, const QVariantList& timestamps, const QVariantList& values, const qint64& topEvent);
  bool FindObject(const QString& objUuid, int& localObjId);

public:
  UniteHandler(UniteInfo* _UniteInfo);
  /*override */virtual ~UniteHandler();
};
