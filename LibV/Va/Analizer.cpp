#include <Lib/Log/Log.h>
#include <Lib/Dispatcher/Overseer.h>
#ifdef USE_ANALIZER
#include <LibA/Analytics/Analytics.h>
#endif // USE_ANALIZER
#include <LibV/Va/AnalyticsA.h>
#include <Lib/Db/Event.h>
#include <Lib/Db/Files.h>
#include <Lib/Settings/DbSettings.h>
#include <Lib/Include/License.h>
#include <Lib/Common/Profiler.h>

#include "Analizer.h"


AnalyticsAS CreateAnalytics(const QString& vaType);

const int kProfileDumpPeriodMs = 30 * 60 * 1000;
const int kOptimizePeriodMs = 30 * 60 * 1000;
const int kOptimizePeriodRetryMs = 30 * 1000;

struct DbTriggeredDetector: public Detector {
  Analizer* Parent;

  /*override */virtual void Hit(const qint64& time, const char* type, qreal value) Q_DECL_OVERRIDE
  {
    Parent->TriggerEvent(Id, type, QDateTime::fromMSecsSinceEpoch(time), value);
  }

  /*override */virtual void Hit(const qint64& time, const char* type, qreal value, const QByteArray& img) Q_DECL_OVERRIDE
  {
    Parent->TriggerEvent(Id, type, QDateTime::fromMSecsSinceEpoch(time), value, img);
  }

  /*override */virtual void Stat(const qint64& time, const char* type, qreal value, int periodMs) Q_DECL_OVERRIDE
  {
    Parent->StatEvent(Id, type, QDateTime::fromMSecsSinceEpoch(time), value, periodMs);
  }

  DbTriggeredDetector(Analizer* _Parent)
    : Parent(_Parent)
  { }
};

bool Analizer::DoInit()
{
  mAnalytics = CreateAnalytics(mVaType);
  mAnalytics->Init();
  mAnalytics->setLocalSettings(mLocalSettings);
  mAnalytics->SetSettings(mSettings);
  mAnalytics->SetDetectors(mDetectors);
  mAnalytics->SetId(mVaId);
  mAnalytics->SetDb(mDb);
  if (mDebug) {
    mAnalytics->SetDebug(GetManager());
  }
  return mAnalytics;
}

void Analizer::DoRelease()
{
  mAnalytics->Finish();
  Log.Info(QString("Analytics total dump: %1").arg(mProfiler->Dump()));
}

bool Analizer::ProcessFrame()
{
  LICENSE_CIRCLE(0x49D861E);

  FrameS currentFrame = CurrentVFrame();
  if (currentFrame->GetHeader()->HeaderSize != sizeof(Frame::Header)) {
    EmergeVFrame(currentFrame);
    return false;
  }
//  static qint64 gBaseTs = currentFrame->GetHeader()->Timestamp;
//  Log.Trace(QString("new fr (ts: %1, video: %2, audio: %3, vsize: %4, asize: %5")
//            .arg(currentFrame->GetHeader()->Timestamp - gBaseTs)
//            .arg(CompressionToString(currentFrame->GetHeader()->Compression))
//            .arg(CompressionToString(currentFrame->GetHeader()->CompressionAudio))
//            .arg(currentFrame->GetHeader()->VideoDataSize)
//            .arg(currentFrame->GetHeader()->AudioDataSize)
//            );
  int cmpr = currentFrame->GetHeader()->Compression;
  int cmprType = (cmpr & eTypeMask);

  switch (cmprType) {
  case eRawVideo:       return ProcessRawFrame(currentFrame);
  case eCompessedVideo: return ProcessCodedFrame(currentFrame);
  default: LOG_WARNING_ONCE(QString("Got invalid compression type frame (type: %1, code: %2)")
                            .arg(CompressionToString((ECompression)cmpr)).arg(cmpr, 0, 16)); break;
  }
  return false;
}

bool Analizer::ProcessRawFrame(const FrameS& currentFrame)
{
  //Log.Trace(QString("decoded (%1)").arg(currentFrame->GetHeader()->Timestamp));
  if (QDateTime::currentMSecsSinceEpoch() > mNextOptimizeTime) {
    if (Optimize()) {
      mNextOptimizeTime = QDateTime::currentMSecsSinceEpoch() + kOptimizePeriodMs;
    } else {
      mNextOptimizeTime = QDateTime::currentMSecsSinceEpoch() + kOptimizePeriodRetryMs;
    }
  }

  if (!mHasCodec) {
    if (currentFrame->GetHeader()->VideoDataSize == 0) {
      return false;
    }
    mProfiler->Start();
    bool result = mAnalytics->AnalizeFrame(*currentFrame);
    mProfiler->Pause();
    mProfiler->AutoDump(kProfileDumpPeriodMs);
    return result;
  }
  if (mSourceFrames.isEmpty()) {
    mDecodedFrames.append(currentFrame);
    return false;
  }

  FrameS source = mSourceFrames.takeFirst();
  while (currentFrame->GetHeader()->Timestamp > source->GetHeader()->Timestamp) {
    EmergeVFrame(source);
    if (mSourceFrames.isEmpty()) {
      mDecodedFrames.append(currentFrame);
      return false;
    }
    source = mSourceFrames.takeFirst();
  }

  if (currentFrame->GetHeader()->Timestamp == source->GetHeader()->Timestamp) {
    FrameS analized = MakeAnalized(source, currentFrame);
    EmergeVFrame(analized);
  } else {
    mSourceFrames.prepend(source);
    mDecodedFrames.append(currentFrame);
    FixFrames();
  }
  return false;
}

bool Analizer::ProcessCodedFrame(const FrameS& currentFrame)
{
  //Log.Trace(QString("encoded (%1)").arg(currentFrame->GetHeader()->Timestamp));
  if (!mHasCodec) {
    LOG_WARNING_ONCE("Get encoded frame, with non coded analizer");
    return false;
  }

  if (mDecodedFrames.isEmpty()) {
    mSourceFrames.append(currentFrame);
    return false;
  }

  FrameS decoded = mDecodedFrames.takeFirst();
  if (decoded->GetHeader()->Timestamp > currentFrame->GetHeader()->Timestamp) {
    EmergeVFrame(currentFrame);
  } else {
    if (decoded->GetHeader()->Timestamp == currentFrame->GetHeader()->Timestamp) {
      FrameS analized = MakeAnalized(currentFrame, decoded);
      EmergeVFrame(analized);
    } else {
      mDecodedFrames.prepend(decoded);
      mSourceFrames.append(currentFrame);
      FixFrames();
    }
  }
  return false;
}

int Analizer::LoadSettings(DbS& db, int vaId)
{
  QList<QPair<QString, int> > detectors;
  auto q = db->MakeQuery();
  q->prepare(QString("SELECT t.name, o._id, o.status FROM object_connection c"
                     " INNER JOIN object o ON o._id = c._oslave"
                     " INNER JOIN object_type t ON t._id = o._otype"
                     " WHERE c._omaster = %1").arg(vaId));
  if (!db->ExecuteQuery(q)) {
    return -1;
  }
  while (q->next()) {
    QString typeName = q->value(0).toString();
    int id = q->value(1).toInt();
    int state = q->value(2).toInt();
    if (state == 0) {
      detectors.append(qMakePair(typeName, id));
    }
  }

  mSettings = SettingsAS(new DbSettings(*db));
  if (!mSettings->Open(QString::number(vaId))) {
    return -2;
  }
  mLocalSettings = SettingsAS(new DbSettings(*db, "variables", "_object"));
  if (!mLocalSettings->Open(QString::number(vaId))) {
    return -3;
  }

  mDetectors.clear();
  for (auto itr = detectors.begin(); itr != detectors.end(); itr++) {
    DetectorS detector(new DbTriggeredDetector(this));
    detector->Id = itr->second;
    detector->Name = itr->first;

    DbSettings detectorSettings(*db);
    if (!detectorSettings.Open(QString::number(detector->Id))) {
      return -12;
    }
    if (!detectorSettings.ExportAsDictionary(detector->Settings)) {
      return -13;
    }
    mDetectors.append(detector);
  }

  return 0;
}

void Analizer::TriggerEvent(int detectorId, const char* eventType, const QDateTime& timestamp, qreal value)
{
  mEventTypeTable->GetItems();
  if (const NamedItem* event = mEventTypeTable->GetItemByName(eventType)) {
    int eventId = event->Id;
    int id;
    if (mEventTable->InitEvent(detectorId, eventId, &id)) {
      if (!mEventTable->TriggerEvent(id, timestamp, value)) {
        Log.Warning(QString("Trigger event fail (detector: %1, event: %2)").arg(detectorId).arg(eventType));
      }
    } else {
      Log.Warning(QString("Init event fail (detector: %1, event: %2)").arg(detectorId).arg(eventType));
    }
  }
}

void Analizer::TriggerEvent(int detectorId, const char* eventType, const QDateTime& timestamp, qreal value, const QByteArray& img)
{
  FilesS file(new Files());
  file->ObjectId = detectorId;
  file->Data = img;
  if (!mFilesTable->Insert(file)) {
    Log.Warning(QString("Insert event image fail (detector: %1, event: %2)").arg(detectorId).arg(eventType));
  }

  mEventTypeTable->GetItems();
  if (const NamedItem* event = mEventTypeTable->GetItemByName(eventType)) {
    int eventId = event->Id;
    int id;
    if (mEventTable->InitEvent(detectorId, eventId, &id)) {
      if (!mEventTable->TriggerEvent(id, file->Id, timestamp, value)) {
        Log.Warning(QString("Trigger event fail (detector: %1, event: %2)").arg(detectorId).arg(eventType));
      }
    } else {
      Log.Warning(QString("Init event fail (detector: %1, event: %2)").arg(detectorId).arg(eventType));
    }
  }
}

void Analizer::StatEvent(int detectorId, const char* eventType, const QDateTime& timestamp, qreal value, int periodMs)
{
  mEventTypeTable->GetItems();
  if (const NamedItem* event = mEventTypeTable->GetItemByName(eventType)) {
    int eventId = event->Id;
    int id;
    if (mEventTable->InitStat(detectorId, eventId, &id)) {
      if (!mEventTable->StatEvent(id, timestamp, value, periodMs)) {
        Log.Warning(QString("Stat event fail (detector: %1, event: %2)").arg(detectorId).arg(eventType));
      }
    } else {
      Log.Warning(QString("Init stat event fail (detector: %1, event: %2)").arg(detectorId).arg(eventType));
    }
  }
}

void Analizer::FixFrames()
{
  FrameS source = mSourceFrames.first();
  FrameS decoded = mDecodedFrames.first();
  int rmSource = 0;
  int rmDecoded = 0;
  qint64 tsSource = source->GetHeader()->Timestamp;
  qint64 tsDecoded = decoded->GetHeader()->Timestamp;
  while (tsSource != tsDecoded) {
    Log.Trace(QString("(source: %1, decoded: %2)").arg(tsSource).arg(tsDecoded));
    if (tsSource < tsDecoded) {
      mSourceFrames.removeFirst();
      rmSource++;
      if (mSourceFrames.empty()) {
        break;
      }
      source = mSourceFrames.first();
    } else {
      mDecodedFrames.removeFirst();
      rmDecoded++;
      if (mDecodedFrames.empty()) {
        break;
      }
      decoded = mDecodedFrames.first();
    }
  }
  Log.Warning(QString("Source frame ts != decoded ts removing (source: %1, decoded: %2)").arg(rmSource).arg(rmDecoded));
}

FrameS Analizer::MakeAnalized(const FrameS& sourceFrame, const FrameS& decodedFrame)
{
  if (decodedFrame->GetHeader()->VideoDataSize == 0) {
    return sourceFrame;
  }

  mProfiler->Start();
  bool result = mAnalytics->AnalizeFrame(*decodedFrame);
  mProfiler->Pause();
  mProfiler->AutoDump(kProfileDumpPeriodMs);

  if (result) {
    while (mAnalytics->HaveNextObject()) {
      sourceFrame->ReserveData(sourceFrame->Size() + sizeof(Object));
      Object* object = reinterpret_cast<Object*>(sourceFrame->ObjectData() + sourceFrame->ObjectDataSize());
      if (mAnalytics->RetrieveNextObject(*object)) {
        sourceFrame->GetHeader()->ObjectDataSize += sizeof(Object);
        sourceFrame->GetHeader()->Size += sizeof(Object);
      } else {
        break;
      }
    }
  }
  return sourceFrame;
}

bool Analizer::Optimize()
{
  foreach (const DetectorS& detector, mDetectors) {
    auto itr = detector->Settings.find("RemoveTime");
    if (itr != detector->Settings.end()) {
      qint64 periodMs = itr.value().toLongLong();
      if (!OptimizeRemoveTime(detector, periodMs)) {
        return false;
      }
    }
    itr = detector->Settings.find("RemoveCount");
    if (itr != detector->Settings.end()) {
      int count = itr.value().toLongLong();
      if (!OptimizeRemoveCount(detector, count)) {
        return false;
      }
    }
    itr = detector->Settings.find("RemoveSize");
    if (itr != detector->Settings.end()) {
      qint64 size = itr.value().toLongLong();
      if (!OptimizeRemoveSize(detector, size)) {
        return false;
      }
    }
  }
  return true;
}

bool Analizer::OptimizeRemoveTime(const DetectorS& detector, qint64 periodMs)
{
  int secs = (int)(periodMs/1000);
  if (secs > 0) {
    if (!mEventTable->RemoveEventLogByPeriod(detector->Id, QDateTime::currentDateTime().addSecs(-secs))) {
      return false;
    }
  }
  return true;
}

bool Analizer::OptimizeRemoveCount(const DetectorS& detector, int count)
{
  return mEventTable->RemoveEventLogByCount(detector->Id, count);
}

bool Analizer::OptimizeRemoveSize(const DetectorS& detector, qint64 size)
{
  return mEventTable->RemoveEventLogBySize(detector->Id, size);
}


Analizer::Analizer(DbS& _Db, int _VaId, const QString& _VaType, bool _HasCodec, bool _Debug)
  : mDb(_Db), mVaId(_VaId), mVaType(_VaType), mHasCodec(_HasCodec), mDebug(_Debug)
  , mEventTypeTable(new EventTypeTable(*mDb)), mEventTable(new EventTable(*mDb)), mFilesTable(new FilesTable(*mDb))
  , mProfiler(new Profiler("Analytics")), mNextOptimizeTime(0)
{
}

Analizer::~Analizer()
{
}
