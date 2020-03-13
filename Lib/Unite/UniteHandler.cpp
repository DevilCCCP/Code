#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>

#include <Lib/Db/ObjectType.h>
#include <Lib/Db/Event.h>
#include <Lib/Db/ObjectLog.h>
#include <Lib/Db/Variables.h>
#include <Lib/Crypto/Rsa.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Log/Log.h>

#include "UniteHandler.h"
#include "UniteInfo.h"
#include "UniteObject.h"


bool UniteHandler::Get(const QString& path, const QList<QByteArray>& params)
{
  Q_UNUSED(params);

  if (path.size() >= 1 && path[0].toLatin1() == '/') {
    if (path.size() == 1) {
      HttpResult(401, "Unauthorized", true);
      return true;
    } else switch (path[1].toLatin1()) {
    case 'U':
      if (path == "/Unite") {
        HttpResultOk(true);
        return true;
      }
      break;
    }
  }

  HttpResultFail(true);
  return true;
}

bool UniteHandler::Post(const QString& path, const QList<QByteArray>& params, const QList<File>& files)
{
  if (path.size() >= 1 && path[0].toLatin1() == '/') {
    if (path.size() == 1) {
      HttpResult(401, "Unauthorized", true);
      return true;
    } else switch (path[1].toLatin1()) {
    case 'Q':
      if (path == "/QuerySegment") {
        if (ValidateSign(params, files) && files.size() == 1) {
          if (QuerySegment(files.first().Data)) {
            return true;
          }
        }
      } else if (path == "/QueryObject") {
        if (ValidateSign(params, files) && files.size() == 1) {
          if (QueryObject(files.first().Data)) {
            return true;
          }
        }
      } else if (path == "/QueryEvents") {
        if (ValidateSign(params, files) && files.size() == 1) {
          if (QueryEvents()) {
            return true;
          }
        }
      } else if (path == "/QueryLogs") {
        if (ValidateSign(params, files) && files.size() == 1) {
          if (QueryLogs()) {
            return true;
          }
        }
      }
      break;
    case 'U':
      if (path == "/UpdateObject") {
        if (ValidateSign(params, files) && files.size() == 1) {
          if (UpdateObject(files.first().Data)) {
            return true;
          }
        }
      } else if (path == "/UpdateEvents") {
        if (ValidateSign(params, files) && files.size() == 1) {
          if (UpdateEvents(files.first().Data)) {
            return true;
          } else if (mCurrentObject) {
            mUniteInfo->SetUniteStage(mCurrentObject->Id, eUniteEvents);
          }
        }
      } else if (path == "/UpdateLogs") {
        if (ValidateSign(params, files) && files.size() == 1) {
          if (UpdateLogs(files.first().Data)) {
            return true;
          } else if (mCurrentObject) {
            mUniteInfo->SetUniteStage(mCurrentObject->Id, eUniteLogs);
          }
        }
      }
      break;
    }
  }

  HttpResultFail(true);
  return true;
}

bool UniteHandler::InitDb()
{
  if (!mDb) {
    mDb.reset(new Db());
    if (!mDb->OpenDefault()) {
      mDb.clear();
      return false;
    }
  }

  return mDb->Connect();
}

bool UniteHandler::ValidateSign(const QList<QByteArray>& params, const QList<File>& files)
{
  mCurrentUuid.clear();
  QByteArray uuid;
  QByteArray pkey;
  QByteArray sign;
  for (auto itr = params.begin(); itr != params.end(); itr++) {
    const QByteArray& param = *itr;
    QList<QByteArray> pair = param.split('=');
    if (pair.size() != 2) {
      continue;
    }
    const QByteArray& key = pair[0];
    const QByteArray& value = pair[1];

    if (key == "uuid") {
      uuid = QByteArray::fromPercentEncoding(value);
    } else if (key == "key") {
      pkey = QByteArray::fromPercentEncoding(value);
    } else if (key == "sign") {
      sign = QByteArray::fromHex(value);
    }
  }

  if (uuid.isEmpty() || pkey.isEmpty() || sign.isEmpty()) {
    return false;
  }

  QByteArray data = uuid;
  for (auto itr = files.begin(); itr != files.end(); itr++) {
    const File& file = *itr;
    data.append(file.Type);
    data.append(file.Name.toUtf8());
    data.append(file.Data);
  }

  mCurrentUuid = uuid;
  mCurrentPkey = pkey;
  if (!VerifySign(data, sign) || !VerifyPkey()) {
    return false;
  }

  return true;
}

bool UniteHandler::LoadObject()
{
  mCurrentObject.clear();
  if (!mDb->getObjectTable()->GetObjectByGuid(QString::fromUtf8(mCurrentUuid), mCurrentObject)) {
    return false;
  }
  return true;
}

bool UniteHandler::VerifySign(const QByteArray& data, const QByteArray& sign)
{
  RsaS rsa = Rsa::FromPemPub(mCurrentPkey);
  if (!rsa) {
    mUniteInfo->WarnOnce(QString("Bad public key (key: '%1')").arg(mCurrentPkey.constData()));
    return false;
  }

  if (!rsa->VerifySha256(data, sign)) {
    Log.Debug(QString("key\n") + mCurrentPkey);
    Log.Debug(QString("data\n") + data);
    Log.Debug(QString("sign\n") + sign.toHex());
    mUniteInfo->WarnOnce(QString("Bad message sign (key: '%1')").arg(mCurrentPkey.constData()));
    return false;
  }
  return true;
}

bool UniteHandler::VerifyPkey()
{
  if (!InitDb()) {
    return false;
  }

  if (!LoadObject()) {
    return false;
  }
  if (!mCurrentObject) {
    return true;
  }
  VariablesS variable;
  if (!mDb->getVariablesTable()->SelectVariable(mCurrentObject->Id, "Pkey", variable)) {
    return false;
  }
  if (!variable) {
    // creation of object broken
    if (mCurrentObject->Revision > 0) {
      mCurrentObject->Revision = 0;
      if (!mDb->getObjectTable()->UpdateItem(mCurrentObject)) {
        return false;
      }
    }
    return true;
  }
  if (variable->Value != QString::fromLatin1(mCurrentPkey)) {
    mUniteInfo->SetUniteStageFail(mCurrentObject->Id, eHandShake);
    return false;
  }
  mUniteInfo->SetUniteStage(mCurrentObject->Id, eHandShake);
  return true;
}

bool UniteHandler::QuerySegment(const QByteArray& data)
{
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError) {
    Log.Error(QString("Parse query json fail (err: '%1')").arg(err.errorString()));
    return false;
  }

  QJsonObject root = doc.object();
  QString uuid = root.value("seg").toString();

  VariablesS variable;
    HttpResult(500, "Internal Server Error", true);
    if (!mDb->getVariablesTable()->SelectVariable("Segment", variable)) {
    return true;
  }
  if (!variable || variable->Value != uuid) {
    if (mCurrentObject) {
      mUniteInfo->SetUniteStage(mCurrentObject->Id, eValidateSegment);
    }
    HttpResult(200, "Ok", true);
    return true;
  }
  if (mCurrentObject) {
    mUniteInfo->SetUniteStageFail(mCurrentObject->Id, eValidateSegment);
  }
  HttpResult(403, "Forbidden", true);
  return true;
}

bool UniteHandler::QueryObject(const QByteArray& data)
{
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError) {
    Log.Error(QString("Parse query json fail (err: '%1')").arg(err.errorString()));
    return false;
  }

  QJsonObject root = doc.object();
  if (mCurrentObject) {
    if (!mUniteInfo->IsTranslatedObject(mCurrentObject)) {
      HttpResult(406, "Not Acceptable", true);
      mUniteInfo->SetUniteStageFail(mCurrentObject->Id, eQueryObjects);
      return true;
    } else if (root.value("rev").toInt() == mCurrentObject->Revision) {
      mUniteInfo->SetUniteStage(mCurrentObject->Id, eQueryObjects);
      HttpResult(201, "Created", true);
      return true;
    } else if (root.value("rev").toInt() < mCurrentObject->Revision && root.value("back").toBool()) {
      mUniteObject.reset(new UniteObject(*mDb, mUniteInfo));
      QByteArray respondJson;
      if (!mUniteObject->ToJson(mCurrentObject, respondJson)) {
        Log.Warning(QString("Request object failed (name: '%1', uuid: '%2')").arg(mCurrentObject->Name, mCurrentObject->Guid));
        return false;
      }
      mUniteInfo->SetUniteStage(mCurrentObject->Id, eQueryObjects);
      HttpResult(205, "Reset Content", false);
      HttpAddContent("application/json; charset=utf-8", respondJson);
      return true;
    }
  }
  HttpResult(202, "Accepted", true);
  return true;
}

bool UniteHandler::QueryEvents()
{
  VariablesS variable;
  QString topVarName = QString::fromUtf8(mCurrentUuid) + "-TopEvent";
  if (!mDb->getVariablesTable()->SelectVariable(mUniteInfo->getServiceId(), topVarName, variable)) {
    HttpResult(500, "Internal Server Error", true);
    return true;
  }
  if (!variable) {
    variable.reset(new Variables());
    variable->Object = mUniteInfo->getServiceId();
    variable->Key = topVarName;
    variable->Value = "0";

    if (!mDb->getVariablesTable()->Insert(variable)) {
      HttpResult(500, "Internal Server Error", true);
      return true;
    }
  }

  QJsonDocument doc;

  QJsonObject root;
  root.insert("TopEvent", QJsonValue(variable->Value.toLongLong()));

  doc.setObject(root);
  QByteArray data = doc.toJson();

  HttpResult(200, "OK", false);
  HttpAddContent("application/json; charset=utf-8", data);
  return true;
}

bool UniteHandler::QueryLogs()
{
  VariablesS variable;
  QString topVarName = QString::fromUtf8(mCurrentUuid) + "-TopLog";
  if (!mDb->getVariablesTable()->SelectVariable(mUniteInfo->getServiceId(), topVarName, variable)) {
    HttpResult(500, "Internal Server Error", true);
    return true;
  }
  if (!variable) {
    variable.reset(new Variables());
    variable->Object = mUniteInfo->getServiceId();
    variable->Key = topVarName;
    variable->Value = "0";

    if (!mDb->getVariablesTable()->Insert(variable)) {
      HttpResult(500, "Internal Server Error", true);
      return true;
    }
  }

  QJsonDocument doc;

  QJsonObject root;
  root.insert("TopLog", QJsonValue(variable->Value.toLongLong()));

  doc.setObject(root);
  QByteArray data = doc.toJson();

  HttpResult(200, "OK", false);
  HttpAddContent("application/json; charset=utf-8", data);
  return true;
}

bool UniteHandler::UpdateObject(const QByteArray& data)
{
  mUniteObject.reset(new UniteObject(*mDb, mUniteInfo));
  if (mCurrentUuid.isEmpty()) {
    LOG_WARNING_ONCE("No root object uuid");
    return false;
  }
  if (!mUniteObject->FromJson(mCurrentUuid, mCurrentPkey, data)) {
    if (mCurrentObject) {
      mUniteInfo->SetUniteStageFail(mCurrentObject->Id, eUniteObjects);
    }
    return false;
  }

  HttpResultOk(true);
  return true;
}

bool UniteHandler::UpdateEvents(const QByteArray& data)
{
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError) {
    Log.Warning(QString("UpdateEvents: parse query json fail (err: '%1')").arg(err.errorString()));
    return false;
  }

  QJsonObject root = doc.object();
  qint64 topEvent = (qint64)root.value("TopEvent").toDouble();
  if (!topEvent) {
    Log.Warning(QString("UpdateEvents: parse json failed at TopEvent"));
    return false;
  }
  qint64 baseTimestamp = (qint64)root.value("BaseTimestamp").toDouble();
  if (!baseTimestamp) {
    Log.Warning(QString("UpdateEvents: parse json failed at BaseTimestamp"));
    return false;
  }

  QJsonValue jsonEvents = root.value("Events");
  if (!jsonEvents.isArray()) {
    Log.Warning(QString("UpdateEvents: parse json failed at Events"));
    return false;
  }

  QMap<int, QPair<int, int> > eventsMap;
  int eventTotal = 0;
  int eventTotalBad = 0;
  int logTotal = 0;
  int logTotalBad = 0;
  QJsonArray jsonEventsArray = jsonEvents.toArray();
  for (auto itr = jsonEventsArray.begin(); itr != jsonEventsArray.end(); itr++) {
    const QJsonValue& jsonEvent = *itr;
    if (!jsonEvent.isObject()) {
      Log.Warning(QString("UpdateEvents: parse json failed at Events array"));
      return false;
    }

    QJsonObject jsonEventNode = jsonEvent.toObject();
    int remoteId      = jsonEventNode.value("Id").toInt();
    QString objUuid   = jsonEventNode.value("Guid").toString();
    QString eventName = jsonEventNode.value("Name").toString();
    int localObjId, localEventId;
    if (!CreateEvent(objUuid, eventName, localObjId, localEventId)) {
      Log.Warning(QString("UpdateEvents: create event fail (uuid: '%1', name: '%2')")
                  .arg(objUuid, eventName));
      return false;
    }
    eventsMap[remoteId] = qMakePair(localObjId, localEventId);

    localObjId? eventTotal++: eventTotalBad++;
  }

  QJsonValue jsonLog = root.value("Log");
  if (!jsonLog.isArray()) {
    Log.Warning(QString("UpdateEvents: parse json failed at Log"));
    return false;
  }

//  QVariantList eventIds;
//  QVariantList timestamps;
//  QVariantList values;
//  QVariantList files;
  QJsonArray jsonLogArray = jsonLog.toArray();
  for (auto itr = jsonLogArray.begin(); itr != jsonLogArray.end(); itr++) {
    const QJsonValue& jsonLog = *itr;
    if (!jsonLog.isObject()) {
      Log.Warning(QString("UpdateEvents: parse json failed at Log array"));
      return false;
    }

    QJsonObject jsonLogNode = jsonLog.toObject();
    int eventId      =         jsonLogNode.value("e").toInt();
    qint64 timestamp = (qint64)jsonLogNode.value("t").toDouble();
    int value        =         jsonLogNode.value("v").toInt();
    QByteArray file  = QByteArray::fromBase64(jsonLogNode.value("f").toString().toLatin1());

    const QPair<int, int>& p = eventsMap[eventId];
    if (p.first) {
      if (!CreateEventLogOne(p.first, p.second, QDateTime::fromMSecsSinceEpoch(baseTimestamp + timestamp), value, file, topEvent)) {
        Log.Warning(QString("Create event log fail one"));
        return false;
      }
//      eventIds.append(eventsMap[eventId]);
//      timestamps.append(QDateTime::fromMSecsSinceEpoch(baseTimestamp + timestamp));
//      values.append(value);


      logTotal++;
    } else {
      logTotalBad++;
    }
  }

  QString topVarName = QString::fromUtf8(mCurrentUuid) + "-TopEvent";
  if (!mDb->getVariablesTable()->UpdateVariable(mUniteInfo->getServiceId(), topVarName, QString::number(topEvent))) {
    HttpResult(500, "Internal Server Error", true);
    return true;
  }

  //if (logTotal > 0 && !CreateEventLog(eventIds, timestamps, values, topEvent)) {
  //  Log.Warning(QString("UpdateEvents: create event log fail (events: %1/%2, log: %3)").arg(eventTotal).arg(eventTotalBad).arg(logTotal));
  //  return false;
  //}

//  Log.Info(QString("Update events log (events: %1/%2, log: %3/%4, top: %5)")
//           .arg(eventTotal).arg(eventTotalBad).arg(logTotal).arg(logTotalBad).arg(topEvent));
  HttpResultOk(true);
  return true;
}

bool UniteHandler::UpdateLogs(const QByteArray& data)
{
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError) {
    Log.Warning(QString("UpdateLogs: parse query json fail (err: '%1')").arg(err.errorString()));
    return false;
  }

  QJsonObject root = doc.object();
  qint64 topLog = (qint64)root.value("TopLog").toDouble();
  if (!topLog) {
    Log.Warning(QString("UpdateLogs: parse json failed at TopLog"));
    return false;
  }
  qint64 baseTimestamp = (qint64)root.value("BaseTimestamp").toDouble();
  if (!baseTimestamp) {
    Log.Warning(QString("UpdateLogs: parse json failed at BaseTimestamp"));
    return false;
  }

  QJsonValue jsonObjects = root.value("Objects");
  if (!jsonObjects.isArray()) {
    Log.Warning(QString("UpdateLogs: parse json failed at Objects"));
    return false;
  }

  QMap<int, int> objectsMap;
//  int objectTotal = 0;
//  int objectTotalBad = 0;
  int logTotal = 0;
  int logTotalBad = 0;
  QJsonArray jsonObjectsArray = jsonObjects.toArray();
  for (auto itr = jsonObjectsArray.begin(); itr != jsonObjectsArray.end(); itr++) {
    const QJsonValue& jsonObject = *itr;
    if (!jsonObject.isObject()) {
      Log.Warning(QString("UpdateLogs: parse json failed at Objects array"));
    }

    QJsonObject jsonObjectNode = jsonObject.toObject();
    int remoteId      = jsonObjectNode.value("Id").toInt();
    QString objUuid   = jsonObjectNode.value("Guid").toString();
    int localObjId;
    if (!FindObject(objUuid, localObjId)) {
      Log.Warning(QString("UpdateLogs: find object fail (id: %1, uuid: '%2')")
                  .arg(remoteId).arg(objUuid));
      return false;
    }
    objectsMap[remoteId] = localObjId;

//    localObjId? objectTotal++: objectTotalBad++;
  }

  QJsonValue jsonLog = root.value("Log");
  if (!jsonLog.isArray()) {
    Log.Warning(QString("UpdateLogs: parse json failed at Log"));
    return false;
  }

  QJsonArray jsonLogArray = jsonLog.toArray();
  for (auto itr = jsonLogArray.begin(); itr != jsonLogArray.end(); itr++) {
    const QJsonValue& jsonLog = *itr;
    if (!jsonLog.isObject()) {
      Log.Warning(QString("UpdateLogs: parse json failed at Log array"));
      return false;
    }

    QJsonObject jsonLogNode = jsonLog.toObject();
    ObjectLogS log(new ObjectLog());

    int remoteObjectId = jsonLogNode.value("i").toInt();
    qint64 timestampStart = (qint64)jsonLogNode.value("s").toDouble();
    qint64 timestampEnd = (qint64)jsonLogNode.value("e").toDouble();
    log->ThreadName = jsonLogNode.value("N").toString();
    log->WorkName = jsonLogNode.value("n").toString();
    log->TotalTime = jsonLogNode.value("t").toInt();
    log->Circles = jsonLogNode.value("c").toInt();
    log->WorkTime = jsonLogNode.value("w").toInt();
    log->LongestWork = jsonLogNode.value("l").toInt();

    log->ObjectId    = objectsMap[remoteObjectId];
    log->PeriodStart = QDateTime::fromMSecsSinceEpoch(baseTimestamp + timestampStart);
    log->PeriodEnd   = QDateTime::fromMSecsSinceEpoch(baseTimestamp + timestampEnd);
    if (log->ObjectId) {
      if (!mDb->getObjectLogTable()->Insert(log)) {
        HttpResult(500, "Internal Server Error", true);
        return true;
      }
      logTotal++;
    } else {
      logTotalBad++;
    }
  }

  QString topVarName = QString::fromUtf8(mCurrentUuid) + "-TopLog";
  if (!mDb->getVariablesTable()->UpdateVariable(mUniteInfo->getServiceId(), topVarName, QString::number(topLog))) {
    HttpResult(500, "Internal Server Error", true);
    return true;
  }

  HttpResultOk(true);
  return true;
}

bool UniteHandler::CreateEvent(const QString& objUuid, const QString& eventName, int& localObjId, int& localEventId)
{
  bool valid;
  if (!mUniteInfo->ValidateId(*mDb, objUuid, valid, &localObjId)) {
    return false;
  }
  if (!valid) {
    localObjId = 0; // invalid object and event
    localEventId = 0;
    return true;
  }
  return mDb->getEventTable()->CreateEvent(objUuid, eventName, &localEventId);
}

bool UniteHandler::CreateEventLogOne(int objectId, int eventId, const QDateTime& timestamp, int value, const QByteArray& data, const qint64& topEvent)
{
  return mDb->getEventTable()->ImportEvent(objectId, eventId, timestamp, value, data, mUniteInfo->getServiceId(), QString::fromUtf8(mCurrentUuid), topEvent);
}

bool UniteHandler::CreateEventLog(const QVariantList& eventIds, const QVariantList& timestamps, const QVariantList& values, const qint64& topEvent)
{
  QString doneQuery = QString("UPDATE variables SET value='%3' WHERE _object = %1 AND key = %2;")
      .arg(mUniteInfo->getServiceId()).arg(ToSql(QString::fromUtf8(mCurrentUuid))).arg(topEvent);
  return mDb->getEventTable()->TriggerEvents(eventIds, timestamps, values, doneQuery);
}

bool UniteHandler::FindObject(const QString& objUuid, int& localObjId)
{
  bool valid;
  if (!mUniteInfo->ValidateId(*mDb, objUuid, valid, &localObjId)) {
    return false;
  }

  if (!valid) {
    localObjId = 0;
  }
  return true;
}


UniteHandler::UniteHandler(UniteInfo* _UniteInfo)
  : mUniteInfo(_UniteInfo)
{
  setDebug(mUniteInfo->getDebug());
  SetLogAll(mUniteInfo->getDebug());
}

UniteHandler::~UniteHandler()
{
}
