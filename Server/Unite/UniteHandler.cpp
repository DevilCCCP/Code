#include <QMutexLocker>

#include <Lib/Db/ObjectType.h>
#include <Lib/Db/Event.h>
#include <Lib/Db/Variables.h>
#include <Lib/Crypto/InnerCrypt.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Log/Log.h>

#include "UniteHandler.h"
#include "UniteInfo.h"


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
        if (ValidateSign(params, files) && LoadObject() && files.size() == 1) {
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
      }
      break;
    case 'U':
      if (path == "/UpdateObject") {
        if (ValidateSign(params, files) && LoadObject() && files.size() == 1) {
          if (UpdateObject(files.first().Data)) {
            return true;
          }
        }
      } else if (path == "/UpdateEvents") {
        if (ValidateSign(params, files) && files.size() == 1) {
          if (UpdateEvents(files.first().Data)) {
            return true;
          }
        }
      }
      break;
    }
  }

  HttpResultFail(true);
  return true;
}

bool UniteHandler::ValidateSign(const QList<QByteArray>& params, const QList<File>& files)
{
  mCurrentUuid.clear();
  QByteArray uuid;
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
    } else if (key == "sign") {
      sign = QByteArray::fromHex(value);
    }
  }

  if (uuid.isEmpty() || sign.isEmpty()) {
    return false;
  }

  QByteArray data = uuid;
  for (auto itr = files.begin(); itr != files.end(); itr++) {
    const File& file = *itr;
    data.append(file.Type);
    data.append(file.Name.toUtf8());
    data.append(file.Data);
  }

  if (!VerifySign(data, sign)) {
    return false;
  }

  mCurrentUuid = uuid;
  return true;
}

bool UniteHandler::LoadObject()
{
  mCurrentObject.clear();
  UniteObjectTable table(mUniteInfo);
  if (!table->GetObjectByGuid(QString::fromUtf8(mCurrentUuid), mCurrentObject)) {
    return false;
  }
  return true;
}

bool UniteHandler::VerifySign(const QByteArray& data, const QByteArray& sign)
{
  UniteInnerCrypt crypt(mUniteInfo);
  if (!crypt->Verify(data, sign)) {
    return false;
  }
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
  UniteVariablesTable table(mUniteInfo);
  if (!table->SelectVariable("Segment", variable)) {
    HttpResult(500, "Internal Server Error", true);
    return true;
  }
  if (!variable || variable->Value != uuid) {
    HttpResult(200, "Ok", true);
    return true;
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
      return true;
    } else if (root.value("rev").toInt() == mCurrentObject->Revision) {
      HttpResult(201, "Created", true);
      return true;
    }
  }
  HttpResult(202, "Accepted", true);
  return true;
}

bool UniteHandler::QueryEvents()
{
  VariablesS variable;
  UniteVariablesTable table(mUniteInfo);
  if (!table->SelectVariable(mUniteInfo->GetOverseer()->Id(), QString::fromUtf8(mCurrentUuid), variable)) {
    HttpResult(500, "Internal Server Error", true);
    return true;
  }
  if (!variable) {
    variable.reset(new Variables());
    variable->Object = mUniteInfo->GetOverseer()->Id();
    variable->Key = mCurrentUuid;
    variable->Value = "0";

    if (!table->Insert(variable)) {
      HttpResult(500, "Internal Server Error", true);
      return true;
    }
  }

  QJsonDocument doc;

  QJsonObject root;
  root.insert("TopEvent", QJsonValue(variable->Value));

  doc.setObject(root);
  QByteArray data = doc.toJson();

  HttpResult(200, "OK", false);
  Answer().append(QByteArray("Content-Type: application/json; charset=utf-8\r\n")
                  + "Content-Length: " + QByteArray::number(data.size()) + "\r\n"
                  + "\r\n");
  Answer().append(data);
  return true;
}

bool UniteHandler::UpdateObject(const QByteArray& data)
{
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError) {
    Log.Warning(QString("UpdateObject: parse query json fail (err: '%1')").arg(err.errorString()));
    return false;
  }

  QJsonObject root = doc.object();
  QJsonValue jsonObjects = root.value("Objects");
  if (!jsonObjects.isArray()) {
    Log.Warning(QString("UpdateObject: parse json failed at Objects"));
    return false;
  }

  int primeId = 0;
  int primeRevision = 0;
  QMap<int, ObjectItemS> objs;
  QSet<int> usedObjs;
  int objTotal = 0;
  int objRemoved = 0;
  QJsonArray jsonObjectsArray = jsonObjects.toArray();
  for (auto itr = jsonObjectsArray.begin(); itr != jsonObjectsArray.end(); itr++) {
    const QJsonValue& jsonObject = *itr;
    if (!jsonObject.isObject()) {
      Log.Warning(QString("UpdateObject: parse json failed at Objects array"));
    }

    mCurrentObject.reset(new ObjectItem());
    QJsonObject jsonObjectNode = jsonObject.toObject();
    QString typeName         = jsonObjectNode.value("Type").toString();
    if (!GetObjectTypeId(typeName, mCurrentObject->Type)) {
      return false;
    }

    int id                   = jsonObjectNode.value("Id").toInt();
    mCurrentObject->Name     = jsonObjectNode.value("Name").toString();
    mCurrentObject->Descr    = jsonObjectNode.value("Descr").toString();
    mCurrentObject->Guid     = jsonObjectNode.value("Guid").toString();
    mCurrentObject->Version  = jsonObjectNode.value("Version").toString();
    mCurrentObject->Revision = jsonObjectNode.value("Revision").toInt();
    mCurrentObject->Uri      = jsonObjectNode.value("Uri").toString();
    mCurrentObject->Status   = jsonObjectNode.value("Status").toInt();
    int masterId             = jsonObjectNode.value("Master").toInt();

    if (mCurrentObject->Guid == mCurrentUuid) {
      primeId = id;
      primeRevision = mCurrentObject->Revision;
      mCurrentObject->Revision = -200;
    }
    objs[id] = mCurrentObject;

    if (masterId) {
      auto itr = objs.find(masterId);
      if (itr == objs.end()) {
        return false;
      }
      mCurrentObject->ParentId = itr.value()->Id;
    }
    if (!UpdateObjectOne() || !UpdateParentConnection()) {
      return false;
    }
    usedObjs.insert(mCurrentObject->Id);

    objTotal++;
  }

  if (primeId) {
    auto itr = objs.find(primeId);
    if (itr != objs.end()) {
      mCurrentObject = itr.value();
      mCurrentObject->Revision = primeRevision;
      if (!RemoveUnused(usedObjs, objRemoved)) {
        return false;
      }

      UniteObjectTable table(mUniteInfo);
      if (!table->UpdateItem(mCurrentObject)) {
        return false;
      }
    }
  }

  Log.Info(QString("Update object (name: '%1', total: %2, removed: %3)").arg(mCurrentObject->Name).arg(objTotal).arg(objRemoved));
  HttpResultOk(true);
  return true;
}

bool UniteHandler::UpdateObjectOne()
{
  if (!mUniteInfo->TranslateObject(mCurrentObject)) {
    return false;
  }

  ObjectItemS localObject;
  UniteObjectTable table(mUniteInfo);
  if (!table->GetObjectByGuid(mCurrentObject->Guid, localObject)) {
    return false;
  }
  if (localObject) {
    if (mCurrentObject->Type != localObject->Type) {
      Log.Warning(QString("Unite item fail (uuid: '%1', type: %2, new type: %3)")
                  .arg(mCurrentObject->Guid).arg(localObject->Type).arg(mCurrentObject->Type));
      return false;
    }
    mCurrentObject->Id = localObject->Id;
    if (localObject->IsEqual(*mCurrentObject)) {
      return true;
    }
  }

  bool isNewItem = mCurrentObject->Id == 0;
  if (isNewItem) {
    if (!table->InsertItem(mCurrentObject)) {
      return false;
    }
  } else {
    if (!table->UpdateItem(mCurrentObject)) {
      return false;
    }
//    if (!table->RemoveSlaves(mCurrentObject->Id)) {
//      return false;
//    }
  }
  Log.Info(QString("%1 item (Name: '%2', Uuid: '%3')").arg(isNewItem? "Registered": "Update")
           .arg(mCurrentObject->Name).arg(mCurrentObject->Guid));
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
    }

    QJsonObject jsonEventNode = jsonEvent.toObject();
    int remoteId      = jsonEventNode.value("Id").toInt();
    QString objUuid   = jsonEventNode.value("Guid").toString();
    QString eventName = jsonEventNode.value("Name").toString();
    int localObjId, localEventId;
    if (!CreateEvent(objUuid, eventName, localObjId, localEventId)) {
      Log.Warning(QString("UpdateEvents: create event fail (uuid: '%1', name: '%2')"));
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

  //if (logTotal > 0 && !CreateEventLog(eventIds, timestamps, values, topEvent)) {
  //  Log.Warning(QString("UpdateEvents: create event log fail (events: %1/%2, log: %3)").arg(eventTotal).arg(eventTotalBad).arg(logTotal));
  //  return false;
  //}

//  Log.Info(QString("Update events log (events: %1/%2, log: %3/%4, top: %5)")
//           .arg(eventTotal).arg(eventTotalBad).arg(logTotal).arg(logTotalBad).arg(topEvent));
  HttpResultOk(true);
  return true;
}

bool UniteHandler::UpdateParentConnection()
{
  if (!mCurrentObject->ParentId) {
    return true;
  }

  UniteObjectTable table(mUniteInfo);
  QList<int> masters;
  if (!table->LoadMasters(mCurrentObject->Id, masters)) {
    return false;
  }
  if (masters.contains(mCurrentObject->ParentId)) {
    return true;
  }
  return table->CreateLink(mCurrentObject->ParentId, mCurrentObject->Id);
}

bool UniteHandler::RemoveUnused(const QSet<int>& usedObjs, int& objRemoved)
{
  QSet<int> existed;
  QList<int> level, nextLevel;
  UniteObjectTable table(mUniteInfo);
  for (level.append(mCurrentObject->Id); !level.isEmpty(); level.swap(nextLevel), nextLevel.clear()) {
    for (auto itr = level.begin(); itr != level.end(); itr++) {
      int id = *itr;
      table->LoadSlaves(id, nextLevel);
    }
    existed.unite(nextLevel.toSet());
  }

  existed.subtract(usedObjs);
  for (auto itr = existed.begin(); itr != existed.end(); itr++) {
    int id = *itr;
    if (!table->RemoveItem(id)) {
      return false;
    }
  }
  objRemoved = existed.size();
  return true;
}

bool UniteHandler::GetObjectTypeId(const QString& typeName, int& id)
{
  UniteObjectType table(mUniteInfo);
  if (const NamedItem* item = table->GetItemByName(typeName)) {
    id = item->Id;
    return true;
  }
  return false;
}

bool UniteHandler::CreateEvent(const QString& objUuid, const QString& eventName, int& localObjId, int& localEventId)
{
  bool valid;
  if (!mUniteInfo->ValidateId(objUuid, valid, &localObjId)) {
    return false;
  }
  if (!valid) {
    localObjId = 0; // invalid camera and event
    localEventId = 0;
    return true;
  }
  UniteEventTable table(mUniteInfo);
  return table->CreateEvent(objUuid, eventName, &localEventId);
}

bool UniteHandler::CreateEventLogOne(int objectId, int eventId, const QDateTime& timestamp, int value, const QByteArray& data, const qint64& topEvent)
{
  UniteEventTable table(mUniteInfo);
  return table->ImportEvent(objectId, eventId, timestamp, value, data, mUniteInfo->GetOverseer()->Id(), QString::fromUtf8(mCurrentUuid), topEvent);
}

bool UniteHandler::CreateEventLog(const QVariantList& eventIds, const QVariantList& timestamps, const QVariantList& values, const qint64& topEvent)
{
  UniteEventTable table(mUniteInfo);
  QString doneQuery = QString("UPDATE variables SET value='%3' WHERE _object = %1 AND key = %2;")
      .arg(mUniteInfo->GetOverseer()->Id()).arg(ToSql(QString::fromUtf8(mCurrentUuid))).arg(topEvent);
  return table->TriggerEvents(eventIds, timestamps, values, doneQuery);
}


UniteHandler::UniteHandler(UniteInfo* _UniteInfo)
  : mUniteInfo(_UniteInfo)
{
}

UniteHandler::~UniteHandler()
{
}
