#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

#include <Lib/Crypto/InnerCrypt.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Db/Variables.h>
#include <Lib/Dispatcher/OverseerState.h>
#include <Lib/Log/Log.h>

#include "UniteAgent.h"


const int kWorkPeriodMs = 500;
const int kLogPerIteration = 2;

bool UniteAgent::DoInit()
{
  mNetManager = new QNetworkAccessManager();
  mEventLoop = new QEventLoop();

  if (!mDb) {
    mDb.reset(new Db());
    if (!mDb->OpenDefault()) {
      return false;
    }
  }

  mObjectType.reset(new ObjectType(*mDb));
  mObjectTable.reset(new ObjectTable(*mDb));
  mVariablesTable.reset(new VariablesTable(*mDb));

  mObjectType->Reload();
  do {
    if (const NamedItem* item = mObjectType->GetItemByName("srv")) {
      mServerTypeId = item->Id;
    } else {
      Log.Fatal("Can't get server type Id");
      break;
    }
    QStringList cameraTypesList;
    if (const NamedItem* item = mObjectType->GetItemByName("cam")) {
      mCameraTypeId = item->Id;
      cameraTypesList << QString::number(item->Id);
    } else {
      Log.Fatal("Can't get camera type Id");
      break;
    }
    if (const NamedItem* item = mObjectType->GetItemByName("aca")) {
      mCamera2TypeId = item->Id;
      cameraTypesList << QString::number(item->Id);
    } else {
      Log.Fatal("Can't get analog camera type Id");
      break;
    }
    mCameraTypes = cameraTypesList.join(",");

    mInnerCrypt.reset(new InnerCrypt());
    return true;
  } while (false);

  GetOverseer()->Done();
  return false;
}

bool UniteAgent::DoCircle()
{
  if (mDb->Connect() && CheckUpdateTime()) {
    bool ok = mValidated || DoValidateSegment();
    if (ok && CheckUpdateObjects()) {
      ok = DoUpdateObjects();
    }
    if (ok && CheckUpdateEvents()) {
      ok = DoUpdateEvents1st();
      while (ok && mTopNextEventId < mTopLocalEventId && IsAlive()) {
        mTopNextEventId = mTopLocalEventId;
        if (mTopNextEventId > mTopHostEventId + kLogPerIteration) {
          mTopNextEventId = mTopHostEventId + kLogPerIteration;
        }
        if (mTopHostEventId >= mTopNextEventId) {
          break;
        }

        ok = DoUpdateEventsNext();
      }
    }
    if (ok) {
      mCheckTimer.start();
    }
    if (mNotifier) {
      (ok)? mNotifier->NotifyGood(): mNotifier->NotifyWarning();
    }
  }
  return true;
}

void UniteAgent::DoRelease()
{
  delete mNetManager;
  delete mEventLoop;
}

bool UniteAgent::CheckUpdateTime()
{
  return !mCheckTimer.isValid() || mCheckTimer.elapsed() >= mPeriodMs;
}

bool UniteAgent::CheckUpdateObjects()
{
  QList<QPair<int, int> > revisions;
  if (!mObjectTable->LoadObjectRevisions(mServerTypeId, revisions)) {
    return false;
  }

  for (auto itr = revisions.begin(); itr != revisions.end(); itr++) {
    const QPair<int, int>& pair = *itr;
    int id = pair.first;
    int rev = pair.second;
    CheckUpdateObjectsOne(id, rev);
    if (IsStop()) {
      return false;
    }
  }
  return !mUpdateServers.isEmpty();
}

bool UniteAgent::CheckUpdateObjectsOne(int id, int rev)
{
  auto itr = mLocalServers.find(id);
  if (itr == mLocalServers.end()) {
    TableItemS item = mObjectTable->GetItem(id, false);
    if (!item) {
      return false;
    }
    ObjectItemS srv = item.staticCast<ObjectItem>();
    mLocalServers[srv->Id] = srv;
    mUpdateServers.append(srv);
    return true;
  } else {
    ObjectItemS srv = itr.value();
    if (srv->Revision != rev) {
      srv->Revision = rev;
      mUpdateServers.append(itr.value());
    }
    return true;
  }
  return false;
}

bool UniteAgent::CheckUpdateEvents()
{
  auto q = mDb->MakeQuery();
  q->prepare("SELECT max(_id) FROM event_log");
  if (!mDb->ExecuteQuery(q)) {
    return false;
  }
  if (!q->next()) {
    return false;
  }
  mTopLocalEventId = q->value(0).toLongLong();
  return mTopHostEventId < mTopLocalEventId;
}

bool UniteAgent::DoValidateSegment()
{
  if (!mSegment) {
    mVariablesTable->SelectVariable("Segment", mSegment);
    if (!mSegment) {
      mVariablesTable->InsertVariable("Segment", QUuid::createUuid().toString());
      mVariablesTable->SelectVariable("Segment", mSegment);
      if (!mSegment) {
        return false;
      }
    }
  }
  mRequestUuid = mSegment->Value.toUtf8();

  bool ok = PackValidate() && SendValidate(mValidated);
  if (!ok) {
    return false;
  }
  if (!mValidated) {
    GetOverseer()->Done();
  }

  return true;
}

bool UniteAgent::DoUpdateObjects()
{
  bool ok = true;
  for (auto itr = mUpdateServers.begin(); itr != mUpdateServers.end(); ) {
    mCurrentObject = *itr;
    if (DoUpdateObjectsOne()) {
      itr = mUpdateServers.erase(itr);
    } else {
      ok = false;
      itr++;
    }
    if (IsStop()) {
      break;
    }
  }
  return ok;
}

bool UniteAgent::DoUpdateObjectsOne()
{
  mRequestUuid = mCurrentObject->Guid.toUtf8();

  bool needUpdate;
  bool ok = PackQuery() && SendQuery(needUpdate);
  if (!ok) {
    return false;
  }
  if (!needUpdate) {
    return true;
  }

  return LoadObject() && PackObjects() && SendObjects();
}

bool UniteAgent::DoUpdateEvents1st()
{
  mCurrentObject = mObjectTable->GetItem(GetOverseer()->Id()).staticCast<ObjectItem>();
  mRequestUuid = mCurrentObject->Guid.toUtf8();

  bool needUpdate;
  bool ok = PackLogQuery() && SendLogQuery(needUpdate);
  if (!ok) {
    return false;
  }
  if (!needUpdate) {
    return true;
  }

  return true;
}

bool UniteAgent::DoUpdateEventsNext()
{
  if (!LoadEvents()) {
    return false;
  }
  if (mEventInfo.isEmpty()) {
    return UpdateTopLog();
  }

  if (!LoadLog()) {
    return false;
  }
  if (mEventLog.isEmpty()) {
    UpdateTopLog();
  }

  return PackEvents() && SendEvents() && UpdateTopLog();
}

bool UniteAgent::LoadObject()
{
  mUpdateObjects.clear();
  QMap<int, ObjectItemS> items;
  if (!mObjectTable->LoadSlavesOfTypes(mCurrentObject->Id, mCameraTypes, items)) {
    return false;
  }
  for (auto itr = items.begin(); itr != items.end(); itr++) {
    const ObjectItemS& item = itr.value();
    mUpdateObjects.append(UpdateObject(item, mCurrentObject->Id));
  }

  QList<ObjectItemS> parents = items.values();
  while (!parents.isEmpty()) {
    QList<ObjectItemS> nextParents;
    for (auto itr = parents.begin(); itr != parents.end(); itr++) {
      const ObjectItemS& parent = *itr;
      QMap<int, ObjectItemS> slaves;
      if (!mObjectTable->LoadSlaves(parent->Id, slaves)) {
        return false;
      }
      for (auto itr = slaves.begin(); itr != slaves.end(); itr++) {
        const ObjectItemS& item = itr.value();
        mUpdateObjects.append(UpdateObject(item, parent->Id));
      }
      nextParents.append(slaves.values());
    }
    parents = nextParents;
  }

  mUpdateObjects.prepend(UpdateObject(mCurrentObject, 0));
  return true;
}

bool UniteAgent::LoadEvents()
{
  mEventInfo.clear();
  auto q = mDb->MakeQuery();
  q->prepare(QString("SELECT DISTINCT e._id, o.guid, t.name FROM event e"
                     " INNER JOIN object o ON e._object = o._id"
                     " INNER JOIN event_type t ON e._etype = t._id"
                     " WHERE e._id IN (SELECT DISTINCT _event FROM event_log WHERE _id > %1 AND _id <= %2);")
             .arg(mTopHostEventId).arg(mTopNextEventId));
  if (!mDb->ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    EventInfo info;
    info.Id         = q->value(0).toInt();
    info.ObjectUuid = q->value(1).toString();
    info.EventName  = q->value(2).toString();
    mEventInfo.append(info);
  }
  return true;
}

bool UniteAgent::LoadLog()
{
  mEventLog.clear();
  auto q = mDb->MakeQuery();
  q->prepare(QString("SELECT DISTINCT l._event, l.triggered_time, l.value, l.info, f.data FROM event_log l"
                     " JOIN files f ON l._file = f._id WHERE l._id > %1 AND l._id <= %2;")
             .arg(mTopHostEventId).arg(mTopNextEventId));
  if (!mDb->ExecuteQuery(q)) {
    return false;
  }

  mEventsBaseTimestamp = 0;
  while (q->next()) {
    EventLog log;
    log.EventId       = q->value(0).toInt();
    log.TriggeredTime = q->value(1).toDateTime().toMSecsSinceEpoch();
    log.Value         = q->value(2).toInt();
    log.Info          = q->value(3).toString();
    log.ScreenShot    = q->value(4).toByteArray();

    if (!mEventsBaseTimestamp) {
      mEventsBaseTimestamp = log.TriggeredTime;
    }
    log.TriggeredTime -= mEventsBaseTimestamp;
    mEventLog.append(log);
  }
  return true;
}

bool UniteAgent::PackValidate()
{
  QJsonDocument doc;

  QJsonObject root;
  root.insert("seg", QJsonValue(mSegment->Value));
  doc.setObject(root);
  mFileData = doc.toJson();
  return !mFileData.isEmpty();
}

bool UniteAgent::PackQuery()
{
  QJsonDocument doc;

  QJsonObject root;
  root.insert("rev", QJsonValue(mCurrentObject->Revision));
  doc.setObject(root);
  mFileData = doc.toJson();
  return !mFileData.isEmpty();
}

bool UniteAgent::PackLogQuery()
{
  // not used
  QJsonDocument doc;

  QJsonObject root;
  root.insert("TopEvent", QJsonValue(mTopLocalEventId));
  doc.setObject(root);
  mFileData = doc.toJson();
  return !mFileData.isEmpty();
}

bool UniteAgent::PackObjects()
{
  QJsonDocument doc;

  QJsonObject root;
  QJsonArray jsonObjectsArray;
  for (auto itr = mUpdateObjects.begin(); itr != mUpdateObjects.end(); itr++) {
    const UpdateObject& upInfo = *itr;
    const ObjectItem* obj = upInfo.Object.data();
    QJsonObject jsonObjectNode;
    TableItemS typeObj = mObjectType->GetItem(obj->Type);
    if (!typeObj) {
      return false;
    }
    const ObjectTypeItem* type = static_cast<const ObjectTypeItem*>(typeObj.data());

    jsonObjectNode.insert("Id",       obj->Id);
    jsonObjectNode.insert("Type",     type->Name);
    jsonObjectNode.insert("Name",     obj->Name);
    jsonObjectNode.insert("Descr",    obj->Descr);
    jsonObjectNode.insert("Guid",     obj->Guid);
    jsonObjectNode.insert("Version",  obj->Version);
    jsonObjectNode.insert("Revision", obj->Revision);
    jsonObjectNode.insert("Uri",      obj->Uri);
    jsonObjectNode.insert("Status",   obj->Status);
    jsonObjectNode.insert("Master",   upInfo.Master);

    jsonObjectsArray.append(QJsonValue(jsonObjectNode));
  }
  root.insert("Objects", QJsonValue(jsonObjectsArray));

  doc.setObject(root);
  mFileData = doc.toJson();
  return !mFileData.isEmpty();
}

bool UniteAgent::PackEvents()
{
  QJsonDocument doc;

  QJsonObject root;
  QJsonArray jsonEventsArray;
  for (auto itr = mEventInfo.begin(); itr != mEventInfo.end(); itr++) {
    const EventInfo& info = *itr;
    QJsonObject jsonEventsNode;
    jsonEventsNode.insert("Id",   info.Id);
    jsonEventsNode.insert("Guid", info.ObjectUuid);
    jsonEventsNode.insert("Name", info.EventName);
    jsonEventsArray.append(QJsonValue(jsonEventsNode));
  }
  root.insert("Events", QJsonValue(jsonEventsArray));

  QJsonArray jsonLogArray;
  for (auto itr = mEventLog.begin(); itr != mEventLog.end(); itr++) {
    const EventLog& info = *itr;
    QJsonObject jsonLogNode;
    jsonLogNode.insert("e", info.EventId);
    jsonLogNode.insert("t", info.TriggeredTime);
    jsonLogNode.insert("v", info.Value);
    jsonLogNode.insert("i", info.Info);
    jsonLogNode.insert("f", QLatin1String(info.ScreenShot.toBase64()));
    jsonLogArray.append(QJsonValue(jsonLogNode));
  }
  root.insert("Log", QJsonValue(jsonLogArray));
  root.insert("TopEvent", QJsonValue(mTopNextEventId));
  root.insert("BaseTimestamp", QJsonValue(mEventsBaseTimestamp));

  doc.setObject(root);
  mFileData = doc.toJson();
  return !mFileData.isEmpty();
}

bool UniteAgent::SendValidate(bool& valid)
{
  if (!SignRequest()) {
    return false;
  }

  int retCode;
  if (SendRequest("QuerySegment", retCode)) {
    if (retCode == 200) {
      valid = true;
      Log.Info(QString("Segment is valid and ready for updates"));
      return true;
    } else if (retCode == 403) {
      valid = false;
      Log.Fatal(QString("Segment not valid for update"));
      return true;
    }

    Log.Warning(QString("Query fail (code: %1)").arg(retCode));
  }
  return false;
}

bool UniteAgent::SendQuery(bool& needUpdate)
{
  if (!SignRequest()) {
    return false;
  }

  int retCode;
  if (SendRequest("QueryObject", retCode)) {
    if (retCode == 201) {
      needUpdate = false;
      Log.Info(QString("Server up to date (id: %1, name: '%2')").arg(mCurrentObject->Id).arg(mCurrentObject->Name));
      return true;
    } else if (retCode == 202) {
      needUpdate = true;
      Log.Info(QString("Server need update (id: %1, name: '%2')").arg(mCurrentObject->Id).arg(mCurrentObject->Name));
      return true;
    } else if (retCode == 406) {
      needUpdate = false;
      Log.Warning(QString("Server not acceptable for update (id: %1, name: '%2')").arg(mCurrentObject->Id).arg(mCurrentObject->Name));
      return true;
    }

    Log.Warning(QString("Query fail (id: %1, name: '%2', code: %3)").arg(mCurrentObject->Id).arg(mCurrentObject->Name).arg(retCode));
  }
  return false;
}

bool UniteAgent::SendLogQuery(bool& needUpdate)
{
  if (!SignRequest()) {
    return false;
  }

  int retCode;
  if (SendRequest("QueryEvents", retCode)) {
    if (retCode == 200) {
      int size = mNetReply->header(QNetworkRequest::ContentLengthHeader).toInt();
      while (mNetReply->bytesAvailable() < size) {
        mNetReply->waitForReadyRead(kWorkPeriodMs);
        if (IsStop()) {
          return false;
        }
      }

      QByteArray data = mNetReply->readAll();
      QJsonParseError err;
      QJsonDocument doc = QJsonDocument::fromJson(data, &err);
      if (err.error != QJsonParseError::NoError) {
        Log.Error(QString("Parse query log respond json fail (err: '%1')").arg(err.errorString()));
        return false;
      }

      QJsonObject root = doc.object();
      mTopHostEventId = root.value("TopEvent").toVariant().toLongLong();
      mTopNextEventId = mTopLocalEventId;
      if (mTopNextEventId > mTopHostEventId + kLogPerIteration) {
        mTopNextEventId = mTopHostEventId + kLogPerIteration;
      }

      needUpdate = mTopHostEventId < mTopNextEventId;
      return true;
    }

    Log.Warning(QString("Query log top id fail (code: %1)").arg(retCode));
  }
  return false;
}

bool UniteAgent::SendObjects()
{
  if (!SignRequest()) {
    return false;
  }

  int retCode;
  if (SendRequest("UpdateObject", retCode)) {
    if (retCode == 200) {
      Log.Info(QString("Update server done (id: %1, name: '%2')").arg(mCurrentObject->Id).arg(mCurrentObject->Name));
      return true;
    }

    Log.Warning(QString("Update server fail (id: %1, name: '%2', code: %3)").arg(mCurrentObject->Id).arg(mCurrentObject->Name).arg(retCode));
  }
  return false;
}

bool UniteAgent::SendEvents()
{
  if (!SignRequest()) {
    return false;
  }

  int retCode;
  if (SendRequest("UpdateEvents", retCode)) {
    if (retCode == 200) {
      Log.Info(QString("Update log done (events: %1, log: %2 (%3, %4]")
               .arg(mEventInfo.size()).arg(mEventLog.size()).arg(mTopHostEventId).arg(mTopNextEventId));
      return true;
    }

    Log.Warning(QString("Update events log fail (id: %1, name: '%2', code: %3)").arg(mCurrentObject->Id).arg(mCurrentObject->Name).arg(retCode));
  }
  return false;
}

bool UniteAgent::UpdateTopLog()
{
  mTopHostEventId = mTopNextEventId;
  return true;
}

bool UniteAgent::SignRequest()
{
  QByteArray data = mRequestUuid;
  data.append(mFileType);
  data.append(mFileName);
  data.append(mFileData);

  mRequestSign = mInnerCrypt->Sign(data);
  return !mRequestSign.isEmpty();
}

bool UniteAgent::SendRequest(const QByteArray& function, int& code)
{
  QNetworkRequest request = QNetworkRequest(QUrl(QString("%1/%2?uuid=%3&sign=%4").arg(mUri, function, mRequestUuid, mRequestSign.toHex())));
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
  request.setHeader(QNetworkRequest::ContentLengthHeader, mFileData.size());
  request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/4.0");

  mNetReply = mNetManager->post(request, mFileData);

  QObject::connect(mNetReply, &QNetworkReply::finished, mEventLoop, &QEventLoop::quit);
  mEventLoop->exec();

  code = mNetReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (code == 0) {
    int err = (int)mNetReply->error();
    if (err != mNetworkError) {
      Log.Warning(QString("Connect to server error (err: '%1', code: %2)").arg(mNetReply->errorString()).arg(err));
      mNetworkError = err;
    }
    return false;
  } else if (mNetworkError != QNetworkReply::NoError) {
    Log.Info(QString("Connect to server ok"));
    mNetworkError = QNetworkReply::NoError;
  }
  return true;
}


UniteAgent::UniteAgent(const QString& _Uri, int _PeriodMs)
  : Imp(kWorkPeriodMs)
  , mUri(_Uri)
  , mPeriodMs(_PeriodMs)
  , mValidated(false), mServerTypeId(0), mCameraTypeId(0), mCamera2TypeId(0)
  , mTopHostEventId(0), mTopNextEventId(0), mTopLocalEventId(0)
  , mNetManager(nullptr), mEventLoop(nullptr), mNetworkError(QNetworkReply::NoError)
{
  mFileType = "";
  mFileName = "";
  if (mUri.endsWith('/')) {
    mUri.chop(1);
  }
}

