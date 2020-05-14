#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QFile>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>

#include <Lib/Crypto/Rsa.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Db/Variables.h>
#include <Lib/Db/ObjectLog.h>
#include <Lib/Dispatcher/OverseerState.h>
#include <Lib/Common/Var.h>
#include <Lib/Log/Log.h>

#include "UniteAgentA.h"
#include "UniteInfo.h"
#include "UniteObject.h"


const int kWorkPeriodMs = 500;
const int kWorkFailMs = 2 * 60 * 1000;
const int kRequestTimeoutMs = 15 * 1000;
const int kEventLogPerIteration = 40;
const int kLogPerIteration = 40;
const int kKeySize = 2048;

bool UniteAgentA::DoInit()
{
  mNetManager      = new QNetworkAccessManager();
  mEventLoop       = new QEventLoop();
  mNetAliveTimer   = new QTimer();
  mNetTimeoutTimer = new QTimer();

  QObject::connect(mNetAliveTimer, &QTimer::timeout, this, &UniteAgentA::OnAliveTimeout);
  QObject::connect(mNetTimeoutTimer, &QTimer::timeout, mEventLoop, &QEventLoop::quit);

  if (!mDb) {
    mDb.reset(new Db());
    if (!mDb->OpenDefault()) {
      GetOverseer()->Restart();
      return false;
    }
  }

  QString keyPath = GetVarFileEx("Priv.key");
  QByteArray keyData;
  QFile keyFile(keyPath);
  if (keyFile.open(QFile::ReadOnly)) {
    keyData = keyFile.readAll();
  }
  if (!keyData.isEmpty()) {
    mRsa = Rsa::FromPem(keyData);
    if (!mRsa) {
      Log.Error(QString("Create Rsa from pem fail"));
    }
  }

  if (!mRsa) {
    Log.Info(QString("Create new key (size: %1)").arg(kKeySize));
    mRsa = Rsa::Create(kKeySize);
    keyData = mRsa->PrivateToPem();
    keyFile.close();
    keyFile.open(QFile::WriteOnly);
    keyFile.write(keyData);
    keyFile.setPermissions(QFile::ReadOwner);
  }

  mRsaPubText = QString::fromLatin1(mRsa->PublicToPem());
  mUniteObject.reset(new UniteObject(*mDb, mUniteInfo.data()));
  mCheckTimer.start();
  return true;
}

bool UniteAgentA::DoCircle()
{
  if (!mDb->Connect() || !CheckUpdateTime()) {
    return true;
  }
  if (!DoValidateSegment()) {
    return true;
  }

  bool ok = DoUpdateAll();

  if (ok) {
    mNextCheck = mCheckTimer.elapsed() + mPeriodMs;
  }
  if (mNotifier) {
    (ok)? mNotifier->NotifyGood(): mNotifier->NotifyWarning();
  }
  if (ok != mHasError) {
    Log.Info(QString("Unite is %1").arg(ok? "ok": "broken"));
    mHasError = ok;
  }
  return true;
}

void UniteAgentA::DoRelease()
{
  delete mNetManager;
  delete mEventLoop;
  delete mNetAliveTimer;
  delete mNetTimeoutTimer;
}

bool UniteAgentA::CheckUpdateTime()
{
  return mCheckTimer.elapsed() >= mNextCheck;
}

bool UniteAgentA::CheckUpdateObjects(bool& needUpdate)
{
  QList<QPair<int, int> > revisions;
  if (!mDb->getObjectTable()->LoadObjectRevisions(mUniteInfo->GetSyncTypeIds(), revisions)) {
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

  needUpdate = !mUpdateServers.isEmpty();
  return true;
}

bool UniteAgentA::CheckUpdateObjectsOne(int id, int rev)
{
  auto itr = mLocalServers.find(id);
  if (itr == mLocalServers.end()) {
    TableItemS item = mDb->getObjectTable()->GetItem(id, false);
    if (!item) {
      return false;
    }
    ObjectItemS srv = item.staticCast<ObjectItem>();
    if (!mUniteBackward) {
      mLocalServers[srv->Id] = srv;
    }
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

bool UniteAgentA::CheckUpdateEvents(bool& needUpdate)
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
  needUpdate = mTopHostEventId < mTopLocalEventId;
  return true;
}

bool UniteAgentA::CheckUpdateLogs(bool& needUpdate)
{
  auto q = mDb->MakeQuery();
  q->prepare("SELECT min(_id), max(_id) FROM object_log");
  if (!mDb->ExecuteQuery(q)) {
    return false;
  }
  if (!q->next()) {
    return false;
  }
  int index = 0;
  mBottomLocalLogId = q->value(index++).toLongLong();
  mTopLocalLogId = q->value(index++).toLongLong();

  needUpdate = mTopHostLogId < mTopLocalLogId;
  return true;
}

bool UniteAgentA::DoValidateSegment()
{
  if (mValidated) {
    return true;
  }

  if (!mSegment) {
    mDb->getVariablesTable()->SelectVariable("Segment", mSegment);
    if (!mSegment) {
      mDb->getVariablesTable()->InsertVariable("Segment", QUuid::createUuid().toString());
      mDb->getVariablesTable()->SelectVariable("Segment", mSegment);
      if (!mSegment) {
        return false;
      }
    }
  }
  mRequestUuid = mSegment->Value.toUtf8();
  mDb->getObjectTable()->Load();

  bool ok = PackValidate() && SendValidate(mValidated);
  if (!ok) {
    return false;
  }
  if (!mValidated) {
    GetOverseer()->Done();
  }

  return true;
}

bool UniteAgentA::DoUpdateAll()
{
  bool needUpdate = false;
  if (!CheckUpdateObjects(needUpdate)) {
      return false;
  }
  if (needUpdate && !DoUpdateObjects()) {
    return false;
  }

  if (getUniteEvents()) {
    if (!CheckUpdateEvents(needUpdate)) {
      return false;
    }
    if (needUpdate && !DoUpdateEvents()) {
      return false;
    }
  }

  if (getUniteLogs()) {
    if (!CheckUpdateLogs(needUpdate)) {
      return false;
    }
    if (needUpdate && !DoUpdateLogs()) {
      return false;
    }
  }

  return true;
}

bool UniteAgentA::DoUpdateObjects()
{
  for (auto itr = mUpdateServers.begin(); itr != mUpdateServers.end(); ) {
    mCurrentObject = *itr;
    if (!DoUpdateObjectsOne()) {
      return false;
    }
    itr = mUpdateServers.erase(itr);
    if (IsStop()) {
      break;
    }
  }
  return true;
}

bool UniteAgentA::DoUpdateObjectsOne()
{
  mRequestUuid = mCurrentObject->Guid.toUtf8();

  bool needUpdate = false;
  bool needBackUpdate = false;
  QByteArray backFile;
  bool ok = PackQueryObject() && SendQueryObject(needUpdate, needBackUpdate, backFile);
  if (!ok) {
    return false;
  }
  if (!needUpdate) {
    return true;
  }

  if (mUniteBackward && needBackUpdate) {
    return mUniteObject->FromJson(mCurrentObject->Guid, QByteArray(), backFile);
  } else {
    return mUniteObject->ToJson(mCurrentObject, mFileData) && SendObjects();
  }
}

bool UniteAgentA::DoUpdateEvents()
{
  if (!DoUpdateEvents1st()) {
    return false;
  }

  while (mTopNextEventId < mTopLocalEventId && SayWork()) {
    mTopNextEventId = mTopLocalEventId;
    if (mTopNextEventId > mTopHostEventId + kEventLogPerIteration) {
      mTopNextEventId = mTopHostEventId + kEventLogPerIteration;
    }
    if (mTopHostEventId >= mTopNextEventId) {
      break;
    }

    if (!DoUpdateEventsNext()) {
      return false;
    }
  }
  return true;
}

bool UniteAgentA::DoUpdateEvents1st()
{
  mCurrentObject = mDb->getObjectTable()->GetItem(GetOverseer()->Id()).staticCast<ObjectItem>();
  mRequestUuid = mCurrentObject->Guid.toUtf8();

  bool needUpdate;
  bool ok = PackEventLogQuery() && SendEventLogQuery(needUpdate);
  if (!ok) {
    return false;
  }
  if (!needUpdate) {
    return true;
  }

  return true;
}

bool UniteAgentA::DoUpdateEventsNext()
{
  if (!LoadEvents()) {
    return false;
  }
  if (mEventInfo.isEmpty()) {
    return UpdateTopEventLog();
  }

  if (!LoadEventLog()) {
    return false;
  }
  if (mEventLog.isEmpty()) {
    UpdateTopEventLog();
  }

  return PackEvents() && SendEvents() && UpdateTopEventLog();
}

bool UniteAgentA::DoUpdateLogs()
{
  if (!DoUpdateLogs1st()) {
    return false;
  }

  while (mTopNextLogId < mTopLocalLogId && SayWork()) {
    mTopNextLogId = qMin(mTopLocalLogId, mTopHostLogId + kLogPerIteration);
    if (mTopHostLogId >= mTopNextLogId) {
      break;
    }

    if (!DoUpdateLogsNext()) {
      return false;
    }
  }
  return true;
}

bool UniteAgentA::DoUpdateLogs1st()
{
  mCurrentObject = mDb->getObjectTable()->GetItem(GetOverseer()->Id()).staticCast<ObjectItem>();
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

bool UniteAgentA::DoUpdateLogsNext()
{
  if (!LoadLog()) {
    return false;
  }
  if (mLogList.isEmpty()) {
    return true;
  }

  return PackLogs() && SendLogs() && UpdateTopLog();
}

bool UniteAgentA::LoadEvents()
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

bool UniteAgentA::LoadEventLog()
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

bool UniteAgentA::LoadLog()
{
  mLogObjectList.clear();
  mLogList.clear();
  if (!mDb->getObjectLogTable()->Select(QString("WHERE _id > %1 AND _id <= %2;").arg(mTopHostLogId).arg(mTopNextLogId), mLogList)) {
    return false;
  }

  QSet<int> usedObjects;
  mLogsBaseTimestamp = QDateTime::currentMSecsSinceEpoch();
  foreach (const ObjectLogS& log, mLogList) {
    mLogsBaseTimestamp = qMin(mLogsBaseTimestamp, log->PeriodStart.toMSecsSinceEpoch());
    if (!usedObjects.contains(log->ObjectId)) {
      TableItemS item = mDb->getObjectTable()->GetItem(log->ObjectId);
      ObjectItemS object = item.dynamicCast<ObjectItem>();
      if (!object) {
        Log.Warning(QString("Object for log not found (id: %1)").arg(log->ObjectId));
        return false;
      }
      mLogObjectList.append(object);
      usedObjects.insert(log->ObjectId);
    }
  }
  return true;
}

bool UniteAgentA::PackValidate()
{
  QJsonDocument doc;

  QJsonObject root;
  root.insert("seg", QJsonValue(mSegment->Value));
  doc.setObject(root);
  mFileData = doc.toJson();
  return !mFileData.isEmpty();
}

bool UniteAgentA::PackQueryObject()
{
  QJsonDocument doc;

  QJsonObject root;
  root.insert("rev", QJsonValue(mCurrentObject->Revision));
  root.insert("back", mUniteBackward);

  doc.setObject(root);
  mFileData = doc.toJson();
  return !mFileData.isEmpty();
}

bool UniteAgentA::PackEventLogQuery()
{
  // not used
  QJsonDocument doc;

  QJsonObject root;
  root.insert("TopEvent", QJsonValue(mTopLocalEventId));
  doc.setObject(root);
  mFileData = doc.toJson();
  return !mFileData.isEmpty();
}

bool UniteAgentA::PackEvents()
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

bool UniteAgentA::PackLogQuery()
{
  QJsonDocument doc;

  QJsonObject root;
  root.insert("TopLog", QJsonValue(mTopLocalLogId));
  doc.setObject(root);
  mFileData = doc.toJson();
  return !mFileData.isEmpty();
}

bool UniteAgentA::PackLogs()
{
  QJsonDocument doc;

  QJsonObject root;

  QJsonArray jsonObjectsArray;
  for (auto itr = mLogObjectList.begin(); itr != mLogObjectList.end(); itr++) {
    const ObjectItemS& object = *itr;
    QJsonObject jsonObjectsNode;
    jsonObjectsNode.insert("Id",   object->Id);
    jsonObjectsNode.insert("Guid", object->Guid);
    jsonObjectsArray.append(QJsonValue(jsonObjectsNode));
  }
  root.insert("Objects", QJsonValue(jsonObjectsArray));

  QJsonArray jsonLogArray;
  for (auto itr = mLogList.begin(); itr != mLogList.end(); itr++) {
    const ObjectLogS& log = *itr;
    QJsonObject jsonLogNode;
    jsonLogNode.insert("i", log->ObjectId);
    jsonLogNode.insert("s", log->PeriodStart.toMSecsSinceEpoch() - mLogsBaseTimestamp);
    jsonLogNode.insert("e", log->PeriodEnd.toMSecsSinceEpoch() - mLogsBaseTimestamp);
    jsonLogNode.insert("N", log->ThreadName);
    jsonLogNode.insert("n", log->WorkName);
    jsonLogNode.insert("t", log->TotalTime);
    jsonLogNode.insert("c", log->Circles);
    jsonLogNode.insert("w", log->WorkTime);
    jsonLogNode.insert("l", log->LongestWork);
    jsonLogArray.append(QJsonValue(jsonLogNode));
  }
  root.insert("Log", QJsonValue(jsonLogArray));
  root.insert("TopLog", QJsonValue(mTopNextLogId));
  root.insert("BaseTimestamp", QJsonValue(mLogsBaseTimestamp));

  doc.setObject(root);
  mFileData = doc.toJson();
  return !mFileData.isEmpty();
}

bool UniteAgentA::SendValidate(bool& valid)
{
  if (!SignRequest()) {
    return false;
  }

  int retCode;
  if (!SendRequest("QuerySegment", retCode)) {
    return false;
  }

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
  return false;
}

bool UniteAgentA::SendQueryObject(bool& needUpdate, bool& needBackUpdate, QByteArray& backFile)
{
  needUpdate = false;
  needBackUpdate = false;
  if (!SignRequest()) {
    return false;
  }

  int retCode;
  if (!SendRequest("QueryObject", retCode, &backFile)) {
    return false;
  }

  if (retCode == 201) {
    needUpdate = false;
    //      Log.Info(QString("Server up to date (id: %1, name: '%2')").arg(mCurrentObject->Id).arg(mCurrentObject->Name));
    return true;
  } else if (retCode == 202) {
    needUpdate = true;
    Log.Info(QString("Server need update (id: %1, name: '%2')").arg(mCurrentObject->Id).arg(mCurrentObject->Name));
    return true;
  } else if (retCode == 205) {
    needUpdate = true;
    needBackUpdate = true;
    Log.Info(QString("Server need backward update (id: %1, name: '%2')").arg(mCurrentObject->Id).arg(mCurrentObject->Name));
    return true;
  } else if (retCode == 406) {
    needUpdate = false;
    Log.Warning(QString("Server not acceptable for update (id: %1, name: '%2')").arg(mCurrentObject->Id).arg(mCurrentObject->Name));
    return true;
  }

  Log.Warning(QString("Query object fail (id: %1, name: '%2', code: %3)").arg(mCurrentObject->Id).arg(mCurrentObject->Name).arg(retCode));
  return false;
}

bool UniteAgentA::SendEventLogQuery(bool& needUpdate)
{
  if (!SignRequest()) {
    return false;
  }

  int retCode;
  if (!SendRequest("QueryEvents", retCode)) {
    return false;
  }

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
      Log.Error(QString("Parse query event log respond json fail (err: '%1')").arg(err.errorString()));
      return false;
    }

    QJsonObject root = doc.object();
    mTopHostEventId = root.value("TopEvent").toVariant().toLongLong();
    mTopNextEventId = mTopLocalEventId;
    if (mTopNextEventId > mTopHostEventId + kEventLogPerIteration) {
      mTopNextEventId = mTopHostEventId + kEventLogPerIteration;
    }

    needUpdate = mTopHostEventId < mTopNextEventId;
    return true;
  }

  Log.Warning(QString("Query event log top id fail (code: %1)").arg(retCode));
  return false;
}

bool UniteAgentA::SendLogQuery(bool& needUpdate)
{
  if (!SignRequest()) {
    return false;
  }

  int retCode;
  if (!SendRequest("QueryLogs", retCode)) {
    return false;
  }

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
    mTopHostLogId = root.value("TopLog").toVariant().toLongLong();
    mTopHostLogId = qMax(mTopHostLogId, mBottomLocalLogId);
    mTopNextLogId = qMin(mTopLocalLogId, mTopHostLogId + kLogPerIteration);

    needUpdate = mTopHostLogId < mTopNextLogId;
    return true;
  }

  Log.Warning(QString("Query log top id fail (code: %1)").arg(retCode));
  return false;
}

bool UniteAgentA::SendObjects()
{
  if (!SignRequest()) {
    return false;
  }

  int retCode;
  if (!SendRequest("UpdateObject", retCode)) {
    return false;
  }

  if (retCode == 200) {
    Log.Info(QString("Update server done (id: %1, name: '%2')").arg(mCurrentObject->Id).arg(mCurrentObject->Name));
    return true;
  }

  Log.Warning(QString("Update server fail (id: %1, name: '%2', code: %3)").arg(mCurrentObject->Id).arg(mCurrentObject->Name).arg(retCode));
  return false;
}

bool UniteAgentA::SendEvents()
{
  if (!SignRequest()) {
    return false;
  }

  int retCode;
  if (!SendRequest("UpdateEvents", retCode)) {
    return false;
  }

  if (retCode == 200) {
    Log.Info(QString("Update log done (events: %1, log: %2 (%3, %4])")
             .arg(mEventInfo.size()).arg(mEventLog.size()).arg(mTopHostEventId).arg(mTopNextEventId));
    return true;
  }

  Log.Warning(QString("Update events log fail (id: %1, name: '%2', code: %3)").arg(mCurrentObject->Id).arg(mCurrentObject->Name).arg(retCode));
  return false;
}

bool UniteAgentA::SendLogs()
{
  if (!SignRequest()) {
    return false;
  }

  int retCode;
  if (!SendRequest("UpdateLogs", retCode)) {
    return false;
  }

  if (retCode == 200) {
    Log.Info(QString("Update log done (log: %1 (%2, %3])")
             .arg(mLogList.size()).arg(mTopHostLogId).arg(mTopNextLogId));
    return true;
  }

  Log.Warning(QString("Update logs fail (id: %1, name: '%2', code: %3)").arg(mCurrentObject->Id).arg(mCurrentObject->Name).arg(retCode));
  return false;
}

bool UniteAgentA::SignRequest()
{
  QByteArray data = mRequestUuid;
  data.append(mFileType);
  data.append(mFileName);
  data.append(mFileData);

  mRequestSign = mRsa->SignSha256(data);
  return !mRequestSign.isEmpty();
}

bool UniteAgentA::SendRequest(const QByteArray& function, int& code, QByteArray* file)
{
  QString url = QString("%1/%2?uuid=%3&key=%4&sign=%5").arg(mUri, function, mRequestUuid, mRsaPubText, mRequestSign.toHex());
  QNetworkRequest request = QNetworkRequest(QUrl(url));
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
  request.setHeader(QNetworkRequest::ContentLengthHeader, mFileData.size());
  request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/4.0");

  if (mUniteInfo->getDebug()) {
    Log.Info(QString("=== Request: ======\n") + url);
  }

  if (mNetReply) {
    mNetReply->deleteLater();
    mNetReply = nullptr;
  }

  mNetReply = mNetManager->post(request, mFileData);
  mNetAliveTimer->start(kWorkPeriodMs);
  mNetTimeoutTimer->start(kRequestTimeoutMs);

  QObject::connect(mNetReply, &QNetworkReply::finished, mEventLoop, &QEventLoop::quit);

  mEventLoop->exec();

  code = mNetReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (code == 0) {
    int err = (int)mNetReply->error();
    if (err != mNetworkError) {
      mNetworkError = err;
      mNetworkErrorCount = 0;
      mNetworkErrorMessage = 1;
    }
    ++mNetworkErrorCount;
    if (mNetworkErrorCount >= mNetworkErrorMessage || mUniteInfo->getDebug()) {
      Log.Warning(QString("Connect to server error (function: '%1', err: '%2', code: %3, count: %4)")
                  .arg(function.constData()).arg(mNetReply->errorString()).arg(err).arg(mNetworkErrorCount));
      mNetworkErrorMessage *= 2;
    }
    SayFail("no connection with server");
    return false;
  }

  if (file) {
    *file = mNetReply->readAll();
  }

  if (mUniteInfo->getDebug()) {
    Log.Info(QString("=== Request: ======\n") + url);
  }

  if (mNetworkHasError) {
    Log.Info(QString("Connect to server ok"));
    mNetworkHasError = false;
  }
  SayOk();
  return true;
}

bool UniteAgentA::UpdateTopEventLog()
{
  mTopHostEventId = mTopNextEventId;
  return true;
}

bool UniteAgentA::UpdateTopLog()
{
  mTopHostLogId = mTopNextLogId;
  if (mRemoveLocal) {
    return mDb->getObjectLogTable()->Delete(QString("WHERE _id < %1").arg(mTopHostLogId));
  } else {
    return true;
  }
}

void UniteAgentA::OnAliveTimeout()
{
  if (!SayWork()) {
    mEventLoop->quit();
  }
}


UniteAgentA::UniteAgentA(const DbS& _Db, const UniteInfoS& _UniteInfo, const QString& _Uri, int _PeriodMs)
  : Imp(kWorkPeriodMs, true, true)
  , mDb(_Db), mUniteInfo(_UniteInfo), mUri(_Uri), mPeriodMs(_PeriodMs), mUniteBackward(false), mUniteEvents(true), mUniteLogs(true)
  , mValidated(false), mNextCheck(0), mHasError(false)
  , mTopHostEventId(0), mTopNextEventId(0), mTopLocalEventId(0)
  , mTopHostLogId(0), mTopNextLogId(0), mTopLocalLogId(0)
  , mNetManager(nullptr), mEventLoop(nullptr), mNetworkHasError(true), mNetworkError(QNetworkReply::NoError)
  , mNetworkErrorCount(0), mNetworkErrorMessage(1)
  , mNetAliveTimer(nullptr), mNetReply(nullptr), mNetTimeoutTimer(nullptr)
{
  SetCriticalWarnMs(kRequestTimeoutMs + 1000);
  SetCriticalFailMs(2 * kRequestTimeoutMs);
  SetFailStateDeadMs(kWorkFailMs);

  mFileType = "";
  mFileName = "";
  if (mUri.endsWith('/')) {
    mUri.chop(1);
  }
}

