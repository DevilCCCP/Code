#include <QStringList>
#include <QPointF>
#include <QPair>

#include <Lib/Log/Log.h>
#include <Lib/Settings/SettingsA.h>
#include <LibV/Include/Tools.h>

#include "MotionTracking.h"
#include "Formula.h"
#include "Hyst.h"


#ifdef QT_NO_DEBUG
const qint64 kSettingsSyncPeriod = 60 * 60 * 1000;
#else
const qint64 kSettingsSyncPeriod = 60 * 1000;
#endif

const int kDiffThresholdMin = 4;
const byte kThresholdSharp = 5;
const int kLayer0DiffCountThreshold = kBlockSize / 2;
const int kThresholdBlockSharp = kBlockSize * kBlockSize * 20 / 100;
const int kQualityUp = 20;
const int kQualityDown = 20;

const int kStartObjectTimestamp = 2000;
const int kUpdateStatPeriod1 = 500;

const int kBlockI = 40;
const int kBlockJ = 51;
const float kMaxSpeedChange = 100;
const float kMaxRadiusChange = 10;

const int kLayerSwitchTime = 3000;


inline int DiffFormula(byte bf, byte bb)
{
  if (bf >= bb) {
    if ((bf>>1) > bb + 5) {
      return 2;
      if ((bf>>2) > bb + 5) {
        return 3;
      }
    }
  } else {
    if ((bb>>1) > bf + 5) {
      return 1;
    }
  }
  return 0;
}

inline int DiffCountFormula(int diffCount, int threshold)
{
  return (diffCount > threshold)? ((diffCount > 2*threshold)? 2: 1): 0;
}

const char* ObjModeToString(Obj2Info::EMode mode)
{
  switch (mode) {
  case Obj2Info::eNormal: return "Normal";
  case Obj2Info::eCasper: return "Casper";
  case Obj2Info::eLost: return "Lost";
  case Obj2Info::eEnded: return "Ended";
  case Obj2Info::eDeleted: return "Deleted";
  case Obj2Info::eModeIllegal: return "ModeIllegal";
  default: return "Error";
  }
}

void ObjInfo::Init(Rectangle _Place)
{
  Quality = 0;
  Place = _Place;
  Speed = Point(0, 0);
  Mode = 0;
  Time = 0;
  PreLinkGood = nullptr;
  PreLinkBad.clear();
}

void MotionTracking::InitSettings(const SettingsAS& _Settings)
{
  mMacroObjects = _Settings->GetValue("Macro", true).toBool();
  if (mMacroObjects) {
    mDiffBackground = true;
    mMinObjectSizePer = 10;
    Log.Info("Using macro analytics");
  } else {
    mDiffBackground = false;
    mMinObjectSizePer = 2;
    Log.Info("Using normal analytics");
  }

  mStandByEnabled = _Settings->GetValue("Standby", false).toBool();
  mStandByMode = false;
}

void MotionTracking::InitDetectors(const QList<DetectorS> &_Detectors)
{
  mBariers.clear();
  mDoors.clear();

  for (auto itr = _Detectors.begin(); itr != _Detectors.end(); itr++) {
    const DetectorS& detector = *itr;
    if (detector->Name == "iod") {
      Log.Info("New in/out detector:");
      Barier barier;
      barier.Detector = detector;
      if (LoadDetectorPoints(detector, barier.Points) && barier.Points.size() >= 2) {
        mBariers.append(barier);
      }
    } else if (detector->Name == "ioo") {
      Log.Info("New door:");
      PointList door;
      if (LoadDetectorPoints(detector, door) && door.size() >= 3) {
        mDoors.append(door);
      }
    }
  }
}

bool MotionTracking::InitImageParameters(int width, int height, int stride)
{
  QString group = QString("%1x%2").arg(width).arg(height);
  if (!getLocalSettings()->BeginGroup(group)) {
    return false;
  }
  mSyncSettings.start();
  QString hystText = getLocalSettings()->GetValue("DiffHyst", QString("")).toString();
  mDiffAvgHyst.Deserialize(hystText, 8);
  getLocalSettings()->EndGroup();

  int fullStride = qMax(width, stride);
  mSafeStride = GetAlignedLeft(fullStride, kBlockSize);
  mSafeHeight = GetAlignedLeft(height, kBlockSize);

  if (mDiffBackground) {
    mDiffLayer0.clear();
    mDiffLayer0.resize(height * mSafeStride, 0);
    mDiffLayer1.clear();
    mDiffLayer1.resize(height * mSafeStride, 0);
    mDiffLayer2.clear();
    mDiffLayer2.resize(height * mSafeStride, 0);
    mDebugLayer.clear();
    mDebugLayer.resize(height * mSafeStride, 0);
    mBorderLayer.clear();
    mBorderLayer.resize(height * (mSafeStride + 2), 0);
  } else {
    mBackground.clear();
    mBackground.resize(height * mSafeStride, 0);
  }
  mStartTimestamp = 0;

  mSceneWidth = mSafeStride / kBlockSize;
  mSceneHeight = height / kBlockSize;
  if (mDiffBackground) {
    mSceneDiffInfo.resize(mSceneHeight * mSceneWidth);
    mSceneDiffStat.resize(mSceneHeight * mSceneWidth);
  } else {
    mSceneInfo.resize(mSceneHeight * mSceneWidth);
    mSceneStat.resize(mSceneHeight * mSceneWidth);
  }
  mSceneTmp.resize(mSceneHeight * mSceneWidth);
  mSceneTmp2.resize(mSceneHeight * mSceneWidth);

  mMinObjectSize = qMax(mSceneWidth, mSceneHeight) * mMinObjectSizePer / 100;
  Log.Info(QString("MinObjectSize = %1").arg(mMinObjectSize));
  return true;
}

void MotionTracking::AnalizePrepare(const byte* imageData)
{
  mImageData = imageData;
}

void MotionTracking::AnalizeInit()
{
  mFrameMs = 0;
  mStartTimestamp = mLastTimestamp = getLastTimestamp();
  if (mDiffBackground) {
    InitDiffLayer0();
    InitStatDiff();
  } else {
    InitBackground();
    InitStat();
  }
}

bool MotionTracking::AnalizeStandBy()
{
  return AnalizeAll();
}

bool MotionTracking::AnalizeNormal()
{
  return AnalizeAll();
}

int MotionTracking::GetDebugFrameCount()
{
  return 5;
}

bool MotionTracking::GetDebugFrame(const int index, QString& text, EImageType& imageType, byte* data, bool &save)
{
  Q_UNUSED(save);

  const static int kSwitchBase = __COUNTER__;
#define AUTO_CASE __COUNTER__ - kSwitchBase

  switch (index) {
  //case AUTO_CASE: text = "Background"; imageType = eValue; GetDbgBackground(data); return true;
  //case AUTO_CASE: text = "Ignore"; imageType = eIndex; GetDbgIgnore(data); return true;

    // Diff layers
  //case AUTO_CASE: text = "Diff layer 0"; imageType = eValue; GetDbgDiffLayer0(data); return true;
  //case AUTO_CASE: text = "Diff layer 1"; imageType = eValue; GetDbgDiffLayer1(data); return true;
  //case AUTO_CASE: text = "Diff layer 2"; imageType = eValue; GetDbgDiffLayer2(data); return true;
  //case AUTO_CASE: text = "Diff layer X"; imageType = eIndex; GetDbgDiffLayerX(data); return true;
  case AUTO_CASE: text = "Diff"; imageType = eValue2; GetDbgDiffLayer0Diff(data); /*save = true;*/ return true;
  case AUTO_CASE: text = "Diff calc"; imageType = eIndex; GetDbgDiffFormula(data); return true;
  //case AUTO_CASE: text = "Diff hyst"; imageType = eHyst; GetDbgHyst(data, mLayer0DiffCountHyst); return true;
  case AUTO_CASE: text = "Diff current layer"; imageType = eIndex; GetDbgDiffCurrentLayer(data); return true;
  case AUTO_CASE: text = "Diff obj detect"; imageType = eIndex; GetDbgObjDetect(data); return true;
  //case AUTO_CASE: text = "Diff layer 0 time"; imageType = eValue; GetDbgDiffLayer0Time(data); return true;
  //case AUTO_CASE: text = "Diff layer 1 time"; imageType = eValue; GetDbgDiffLayer1Time(data); return true;
  //case AUTO_CASE: text = "PreObjects"; imageType = eIndex; GetDbgDiffInfoObjects(data); return true;
  //case AUTO_CASE: text = "Custom hyst"; imageType = eHyst; GetDbgHyst(data, mDebugHyst); return true;
  //case AUTO_CASE: text = "DiffP block"; imageType = eIndex; GetDbgDiffP(data); return true;

    // Diff
  //case AUTO_CASE: text = "Diff"; imageType = eValue2; GetDbgDiff(data); return true;
//  case AUTO_CASE: text = "Diff block hyst"; imageType = eHyst; GetDbgBlockDiffHyst(data); return true;
  //case AUTO_CASE: text = "Diff block"; imageType = eIndex; GetDbgBlockDiff(data); return true;
//  case AUTO_CASE: text = "Diff block stat mid"; imageType = eIndex; GetDbgBlockDiffMidStat(data); return true;
//  case AUTO_CASE: text = "Diff micro hyst"; imageType = eHyst; GetDbgDiffMicroHyst(data); return true;
//  case AUTO_CASE: text = "Diff block stat max"; imageType = eIndex; GetDbgBlockDiffMaxStat(data); return true;
//  case AUTO_CASE: text = "Diff one block"; imageType = eHyst; GetDbgBlockDiffOneHyst(data, kBlockI, kBlockJ); return true;
//  case AUTO_CASE: text = "Diff block front"; imageType = eValue; GetDbgBlockDiff(data); return true;
//  case AUTO_CASE: text = "Diff hyst"; imageType = eHyst; GetDbgDiffHyst(data); return true;
//  case AUTO_CASE: text = "Diff background"; imageType = eValue; GetDbgDiffBackground(data); return true;

    // Sharp
//  case AUTO_CASE: text = "Sharp"; imageType = eValue; GetDbgSharp(data); return true;
//  case AUTO_CASE: text = "Sharp Diff"; imageType = eValue; GetDbgSharpDiff(data); return true;
//  case AUTO_CASE: text = "Sharp hyst"; imageType = eHyst; GetDbgSharpHyst(data); return true;
//  case AUTO_CASE: text = "Diff sharp block"; imageType = eIndex; GetDbgBlockDiffSharp(data); return true;
//  case AUTO_CASE: text = "Diff sharp block stat mid"; imageType = eIndex; GetDbgBlockDiffSharpMidStat(data); return true;
//  case AUTO_CASE: text = "Diff sharp block stat max"; imageType = eIndex; GetDbgBlockDiffSharpMaxStat(data); return true;

    // Pre objects
//  case AUTO_CASE: text = "Diff block"; imageType = eIndex; GetDbgBlockDiff(data); return true;
//  case AUTO_CASE: text = "Diff sharp block"; imageType = eIndex; GetDbgBlockDiffSharp(data); return true;
  //case AUTO_CASE: text = "PreObjects"; imageType = eIndex; GetDbgInfoObjects(data); return true;
//  case AUTO_CASE: text = "PreObjects hyst"; imageType = eHyst; GetDbgInfoObjectsHyst(data); return true;

    // Objects
//  case AUTO_CASE: text = "Diff block"; imageType = eIndex; GetDbgBlockDiff(data); return true;
//  case AUTO_CASE: text = "Diff sharp block"; imageType = eIndex; GetDbgBlockDiffSharp(data); return true;
//  case AUTO_CASE: text = "PreObjects"; imageType = eIndex; GetDbgInfoObjects(data); return true;

//  case AUTO_CASE: text = "Sharp"; imageType = eValue; GetDbgBlockSharp(data); return true;
//  case AUTO_CASE: text = "Sharp"; imageType = eIndex; GetDbgInfoSharp(data); return true;
//  case AUTO_CASE: text = "SharpStat"; imageType = eIndex; GetDbgStat(data, offsetof(Stat, SharpD), kThresholdBlockSharp * 4); return true;
//  case AUTO_CASE: text = "SharpStat"; imageType = eIndex; GetDbgStatSharp(data); return true;
//  case AUTO_CASE: text = "Front"; imageType = eIndex; GetDbgFront(data); return true;
//  case AUTO_CASE: text = "Sharp"; imageType = eHyst; GetSharpHyst(data); return true;
//  case AUTO_CASE: text = "Stable"; imageType = eValue; GetDbgStability(data); return true;
  default: return false;
  }
  return true;
#undef AUTO_CASE
}

bool MotionTracking::HaveNextObject()
{
  if (mStandByMode) {
    return false;
  }
  //return mObjectCounter < (int)mPreObjects.size();
//  return mObjectCounter < mObjectInfo.size();
  return mObjectCounter < mObject2Info.size();
}

bool MotionTracking::RetrieveNextObject(Object &object)
{
  //if (mObjectCounter < (int)mPreObjects.size()) {
  //  const ObjPre& info = mPreObjects[mObjectCounter];
  //  object.Dimention.Left = info.Place.Left;
  //  object.Dimention.Right = info.Place.Right - 1;
  //  object.Dimention.Top = info.Place.Top;
  //  object.Dimention.Bottom = info.Place.Bottom - 1;
  //  object.Color = 100;
  //  mObjectCounter++;
  //  return true;
  //}
//  if (mObjectCounter < mObjectInfo.size()) {
//    const ObjInfo& info = mObjectInfo.at(mObjectCounter);
//    object.Dimention.Left = info.Place.Left;
//    object.Dimention.Right = info.Place.Right;
//    object.Dimention.Top = info.Place.Top;
//    object.Dimention.Bottom = info.Place.Bottom;
//    object.Color = (info.Time >= 2000)
//        ? ((info.Mode)? 2000 + info.Quality: 1000 + info.Quality)
//        : info.Time / 20;
//    mObjectCounter++;
//    return true;
//  }
  if (mObjectCounter < mObject2Info.size()) {
    const Obj2Info& info = mObject2Info.at(mObjectCounter);
    object.Dimention.Left = qMax(0, (int)(info.Location.x() - info.Radius.x()) * kBlockSize);
    object.Dimention.Right = qMin(getWidth(), (int)(info.Location.x() + info.Radius.x()) * kBlockSize);
    object.Dimention.Top = qMax(0, (int)(info.Location.y() - info.Radius.y()) * kBlockSize);
    object.Dimention.Bottom = qMin(getHeight(), (int)(info.Location.y() + info.Radius.y()) * kBlockSize);
    switch (info.Mode) {
    case Obj2Info::eNormal: object.Color = 1000 + 50 + info.Quality / 20; break;
    case Obj2Info::eLost:   object.Color = 2000 + 50 + info.Quality / 20; break;
    case Obj2Info::eCasper: object.Color =    0 + 50 + info.Quality / 20; break;
    case Obj2Info::eEnded:  object.Color = 3000; break;
    case Obj2Info::eModeIllegal:
    default:                object.Color = 3000 + 50 + info.Quality / 20; break;
    }

    mObjectCounter++;
    return true;
  }
  return false;
}

bool MotionTracking::AnalizeAll()
{
  mObjectCounter = 0;
  if (!mStartTimestamp) {
    return false;
  } else {
    mFrameMs = (int)qMin(1000LL, getFrameMs());
    if (mStandByMode && mFrameMs < 1000) {
      return false;
    }
    mLastTimestamp = getLastTimestamp();
  }
  mTimeSec = mFrameMs * 0.001f;

  if (mDiffBackground) {
    SceneInitDiff();
    CalcLayer2();
    if (mStandByEnabled) {
      if (TrySwithStandByMode()) {
        return false;
      }
    }
    UpdateStatDiff();

    PreObjectsCreateDiff();

    ObjectsManage();
  } else {
    SceneInit();
    CalcFront();
    UpdateStat();

    if (mLastTimestamp - mStartTimestamp > kStartObjectTimestamp) {
      if (mMacroObjects) {
        PreObjectsCreateMacro();
        ObjectsManage();
      } else {
        PreObjectsCreateNormal();
      }
    }

    if (mLastTimestamp - mStartTimestamp > mCalc1Timestamp) {
      mCalc1Timestamp = mCalc1Timestamp + kUpdateStatPeriod1;
      UpdateStatPeriod1();
    }
  }
  return true;
}

bool MotionTracking::LoadDetectorPoints(const DetectorS& detector, PointList& points)
{
  for (int i = 0; ; i++) {
    QString key = QString::fromUtf8("Point #") + QString::number(i);
    if (detector->Settings.contains(key)) {
      QString value = detector->Settings[key];
      QStringList values = value.split(QChar(','), QString::SkipEmptyParts);
      if (values.size() == 2) {
        bool ok1, ok2;
        qreal px = values[0].toFloat(&ok1) * 100.0f;
        qreal py = values[1].toFloat(&ok2) * 100.0f;
        if (ok1 && ok2) {
          Log.Info(QString("Point %1 (%2, %3)").arg(i).arg(px).arg(py));
          points.append(QPointF(px, py));
        }
      }
    } else {
      break;
    }
  }
  return true;
}

bool MotionTracking::TrySwithStandByMode()
{
  mDiffAvgHyst.Inc(mDiffNowCount);
  if (mDiffAvgHyst.TotalCount() < 2000) {
    return false;
  }
  if (mSyncSettings.elapsed() > kSettingsSyncPeriod) {
    SyncSettings();
    mSyncSettings.restart();
  }

  int threshold1 = (2 * mDiffAvgHyst.GetValue(150) + mDiffAvgHyst.GetValue(850))/3;
  int threshold2 = mDiffAvgHyst.GetValue(500) / 4;
  int threshold3 = (mDiffAvgHyst.GetValue(150) + mDiffAvgHyst.GetValue(500))/2;
  if (mDiffNowCount < threshold1 && mDiffNowCount < threshold2) {
    if (!mStandByMode) {
      Log.Info(QString("Analize will fall asleep (diff: %1, threshold: %2, %3, %4)").arg(mDiffNowCount).arg(threshold1).arg(threshold2).arg(threshold3));
      mStandByMode = true;
    }
    return true;
  } else if (mStandByMode && mDiffNowCount > threshold3) {
    Log.Info(QString("Analize will awake (diff: %1, threshold: %2, %3, %4)").arg(mDiffNowCount).arg(threshold1).arg(threshold2).arg(threshold3));
    mStartTimestamp = 0;
    mStandByMode = false;
    return true;
  }
  return false;
}

void MotionTracking::SyncSettings()
{
  Log.Info(QString("Sync settings"));
  mDiffAvgHyst.Normalize(14*24*60*60*30);

  QString group = QString("%1x%2").arg(getWidth()).arg(getHeight());
  if (getLocalSettings()->BeginGroup(group)) {
    QString hystText = mDiffAvgHyst.Serialize(8);
    getLocalSettings()->SetValue("DiffHyst", hystText);
    getLocalSettings()->EndGroup();
  }
}

void MotionTracking::InitBackground()
{
  for (int j = 0; j < getHeight(); j++) {
    const byte* img = mImageData + j * getStride();
    byte* bgrnd = mBackground.data() + j * mSafeStride;
    for (int i = 0; i < mSafeStride; i++) {
      *bgrnd++ = *img++;
    }
  }
}

void MotionTracking::InitDiffLayer0()
{
  memset(mDebugLayer.data(), 0, mSafeStride*getHeight());
  memset(mBorderLayer.data(), 0, (mSafeStride + 2)*getHeight());
  if (mDiffBackgroundDouble) {
    memset(mDiffLayer0.data(), 0, mSafeStride);
    memset(mDiffLayer1.data(), 0, mSafeStride);
    memset(mDiffLayer2.data(), 0, mSafeStride);
    for (int j = 1; j < getHeight(); j++) {
      mDiffLayer1[j * mSafeStride] = 0;
      mDiffLayer2[j * mSafeStride] = 0;

      const byte* imgp = mImageData + j * getStride();
      const byte* img = imgp + 1;
      const byte* imgb = img - getStride();
      byte* back = mDiffLayer0.data() + j * mSafeStride;
      *back = 0; back++;
      for (int i = 1; i < mSafeStride; i++) {
        byte bh = (*imgp > *img)? *imgp - *img: *img - *imgp;
        byte bv = (*imgb > *img)? *imgb - *img: *img - *imgb;
        *back = qMax(bh, bv);

        imgb++;
        imgp++;
        img++;
        back++;
      }
    }
  } else {
    for (int j = 0; j < getHeight(); j++) {
      mDiffLayer1[j * getStride()] = 0;
      mDiffLayer2[j * getStride()] = 0;

      const byte* imgp = mImageData + j * getStride();
      const byte* img = imgp + 1;
      byte* back = mDiffLayer0.data() + j * mSafeStride;
      *back = 0; back++;
      for (int i = 1; i < mSafeStride; i++) {
        *back = (*imgp > *img)? *imgp - *img: *img - *imgp;

        imgp++;
        img++;
        back++;
      }
    }
  }
}

void MotionTracking::InitStat()
{
  mPreCountStat.Reset();

  memset(mSceneStat.data(), 0, sizeof(Stat) * mSceneWidth * mSceneHeight);

  if (mDoors.size() > 0) {
    for (int jj = 0; jj < mSceneHeight; jj++) {
      Stat* stat = &mSceneStat[jj * mSceneWidth];
      for (int ii = 0; ii < mSceneWidth; ii++) {
        stat->Flag = (IsPointInDoor(ii, jj))? ((int)(StatDif::eIgnore) | (int)(StatDif::eDoor)): 0;

        stat++;
      }
    }
  }
  //for (int j = 0; j < getHeight(); j++) {
  //  const byte* img = mImageData + j * getStride();
  //  Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
  //  byte bl = *img;
  //  for (int ii = 0; ii < mSceneWidth; ii++) {
  //    for (int i = 0; i < kBlockSize; i++) {
  //      const byte& bi = *img;

  //      if (bi > bl) {
  //        if (bi - bl > kThresholdSharp) {
  //          stat->Sharp.Update(bi - bl, 255);
  //        }
  //      } else {
  //        if (bl - bi > kThresholdSharp) {
  //          stat->Sharp.Update(bl - bi, 255);
  //        }
  //      }

  //      img++;
  //      bl = bi;
  //    }
  //    stat++;
  //  }
  //}

  //for (int j = 0; j < getHeight(); j++) {
  //  Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
  //  for (int ii = 0; ii < mSceneWidth; ii++) {
  //    stat->SharpD = stat->Sharp / 10;
  //    stat++;
  //  }
  //}
}

void MotionTracking::InitStatDiff()
{
  memset(mSceneDiffStat.data(), 0, sizeof(StatDif) * mSceneWidth * mSceneHeight);

  if (mDoors.size() > 0) {
    for (int jj = 0; jj < mSceneHeight; jj++) {
      StatDif* stat = &mSceneDiffStat[jj * mSceneWidth];
      for (int ii = 0; ii < mSceneWidth; ii++) {
        stat->Flag = (IsPointInDoor(ii, jj))? ((int)(StatDif::eIgnore) | (int)(StatDif::eDoor)): 0;

        stat++;
      }
    }
  }
}

void MotionTracking::UpdateStatPeriod1()
{
  for (int jj = 0; jj < mSceneHeight; jj++) {
    Info* info = &mSceneInfo[jj * mSceneWidth];
    Stat* stat = &mSceneStat[jj * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      stat->DiffCount.Update(info->DiffCount, kBlockSize * kBlockSize);
      stat->DiffSharpCount.Update(info->DiffSharpCount, kBlockSize * kBlockSize);

      info++;
      stat++;
    }
  }
}

void MotionTracking::SceneInit()
{
  memset(mSceneInfo.data(), 0, sizeof(Info) * mSceneWidth * mSceneHeight);
//  for (int jj = 0; jj < mSceneHeight; jj++) {
//    Info* scene = &mSceneInfo[jj * mSceneWidth];
//    for (int ii = 0; ii < mSceneWidth; ii++) {
//      scene->Info.Clear();
//      scene++;
//    }
  //  }
}

void MotionTracking::SceneInitDiff()
{
  memset(mSceneDiffInfo.data(), 0, sizeof(InfoDif) * mSceneWidth * mSceneHeight);
}

void MotionTracking::CalcFront()
{
  int diffThreshold = mDiffStatScene.Threshold();
  int diffSharpThreshold = mDiffSharpStatScene.Threshold();

  for (int j = 0; j < mSafeHeight; j++) {
    const byte* img = mImageData + j * getStride();
    byte* bgrnd = mBackground.data() + j * mSafeStride;
    Info* scene = &mSceneInfo[j / kBlockSize * mSceneWidth];
    byte bil = *img;
    byte bbl = *bgrnd;
    for (int ii = 0; ii < mSceneWidth; ii++) {
      for (int i = 0; i < kBlockSize; i++) {
        byte& bb = *bgrnd;
        const byte& bi = *img;

        if (bb < bi) {
          int d = bi - bb;
          if (d > diffThreshold) {
            scene->DiffCount++;
          }
          scene->DiffP += d;
          bb++;
        } else if (bb > bi) {
          int d = bb - bi;
          if (d > diffThreshold) {
            scene->DiffCount++;
          }
          scene->DiffM += d;
          bb--;
        }

        byte bis = (bi >= bil)? bi - bil: bil - bi;
        byte bbs = (bb >= bbl)? bb - bbl: bbl - bb;
        if (bis > bbs) {
          int dd = bis - bbs;
          if (dd > diffSharpThreshold) {
            scene->DiffSharpCount++;
          }
        } else {
          int dd = bbs - bis;
          if (dd > diffSharpThreshold) {
            scene->DiffSharpCount++;
          }
        }

        bil = bi;
        bbl = bb;
        bgrnd++;
        img++;
      }
      scene++;
    }
  }
}

void MotionTracking::CalcLayer2()
{
  mDebugHyst.Clear();
  if (mDiffBackgroundDouble) {
    for (int j = 1; j < mSafeHeight; j++) {
      const byte* img = mImageData + j * getStride();
      byte bip = *img;
      const byte* imgb = img - getStride();
      const byte* lay0 = mDiffLayer0.data() + j * mSafeStride;
      byte* lay2 = mDiffLayer2.data() + j * mSafeStride;
      InfoDif* scene = &mSceneDiffInfo[j / kBlockSize * mSceneWidth];
      byte* layB = mBorderLayer.data() + j * (mSafeStride + 2) + 1;

      for (int ii = 0; ii < mSceneWidth; ii++) {
        for (int i = 0; i < kBlockSize; i++) {
          byte bi = *img;
          byte bib = *imgb;
          byte bb = *lay0;

          byte bfh = (bip > bi)? bip - bi: bi - bip;
          byte bfv = (bib > bi)? bib - bi: bi - bib;
          byte bf = qMax(bfh, bfv);
          *lay2 = bf;

          int value = DiffFormula(bf, bb);
          if (value) {
            scene->DiffCount += value;
            scene->DiffPCount += value - 1;
            if (value > 1) {
              *layB = 1;
            } else {
              *layB = 0;
            }
          } else {
            *layB = 0;
          }
          mDebugHyst.Inc((int)bf);

          bip = bi;
          img++;
          imgb++;
          lay0++;
          lay2++;

          layB++;
        }
        scene++;
      }
    }
    mDiffNowCount = mDebugHyst.GetValue(990);
  } else {
    for (int j = 0; j < mSafeHeight; j++) {
      const byte* img = mImageData + j * getStride();
      byte bip = *img;
      const byte* lay0 = mDiffLayer0.data() + j * mSafeStride;
      byte* lay2 = mDiffLayer2.data() + j * mSafeStride;
      InfoDif* scene = &mSceneDiffInfo[j / kBlockSize * mSceneWidth];
      byte* layB = mBorderLayer.data() + j * (mSafeStride + 2) + 1;

      for (int ii = 0; ii < mSceneWidth; ii++) {
        for (int i = 0; i < kBlockSize; i++) {
          byte bi = *img;
          byte bb = *lay0;

          byte bf = (bip > bi)? bip - bi: bi - bip;
          *lay2 = bf;
          int value = DiffFormula(bf, bb);
          if (value) {
            scene->DiffCount += value;
            scene->DiffPCount += value - 1;
            if (value > 1) {
              *layB = 1;
            } else {
              *layB = 0;
            }
          } else {
            *layB = 0;
          }
          mDebugHyst.Inc((int)bf);

          bip = bi;
          img++;
          lay0++;
          lay2++;

          layB++;
        }
        scene++;
      }
    }
    mDiffNowCount = mDebugHyst.GetValue(990);
  }

  //Log.Debug(QString("cut: %1, %2, %3").arg(mDebugHyst.GetValue(950)).arg(mDebugHyst.GetValue(990)).arg(mDebugHyst.GetValue(995)));
}

void MotionTracking::UpdateStat()
{
  mDiffStatScene.InitTotalCount();
  mDiffSharpStatScene.InitTotalCount();
  for (int jj = 0; jj < mSceneHeight; jj++) {
    Info* scene = &mSceneInfo[jj * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      mDiffStatScene.AddTotalCount(scene->DiffCount);
      mDiffSharpStatScene.AddTotalCount(scene->DiffSharpCount);
      scene++;
    }
  }
  mDiffStatScene.UpdateThreshold(mSafeStride * getHeight());
  mDiffSharpStatScene.UpdateThreshold(mSafeStride * getHeight());
}

inline void CopyLayerPart(int ii, int jj, int _SafeStride, std::vector<byte>& _DiffLayerDst, std::vector<byte>& _DiffLayerSrc)
{
  byte* layd = _DiffLayerDst.data() + (jj * kBlockSize) * _SafeStride + (ii * kBlockSize);
  const byte* lays = _DiffLayerSrc.data() + (jj * kBlockSize) * _SafeStride + (ii * kBlockSize);
  for (int j = 0; j < kBlockSize; j++) {
    memcpy(layd, lays, kBlockSize);

    layd += _SafeStride;
    lays += _SafeStride;
  }
}

void MotionTracking::UpdateStatDiff()
{
  for (int jj = 0; jj < mSceneHeight; jj++) {
    InfoDif* scene = &mSceneDiffInfo[jj * mSceneWidth];
    StatDif* stat = &mSceneDiffStat[jj * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      //mLayer0DiffCountHyst.Inc(scene->DiffCount);
      if (DiffCountFormula(scene->DiffCount, kLayer0DiffCountThreshold)) {
        // lay0 miss
        stat->Layer0TimeFalse += mFrameMs;
        int diffCount = 0;
        for (int j = 0; j < kBlockSize; j++) {
          const byte* lay1 = mDiffLayer1.data() + (jj * kBlockSize + j) * mSafeStride + (ii * kBlockSize);
          const byte* lay2 = mDiffLayer2.data() + (jj * kBlockSize + j) * mSafeStride + (ii * kBlockSize);
          for (int i = 0; i < kBlockSize; i++) {
            diffCount += DiffFormula(*lay2, *lay1);

            lay1++;
            lay2++;
          }
        }

        if (!DiffCountFormula(diffCount, kLayer0DiffCountThreshold)) {
          stat->Layer = 1;
          stat->Layer1TimeTrue += mFrameMs;
          if (stat->Layer1TimeTrue > stat->Layer0TimeTrue || stat->Layer1TimeTrue > kLayerSwitchTime) {
            // lay0 <- lay1
            stat->Layer = 0;
            stat->Layer0TimeTrue = stat->Layer1TimeTrue;
            stat->Layer0TimeFalse = 0;
            stat->Layer1TimeTrue = 0;

            CopyLayerPart(ii, jj, mSafeStride, mDiffLayer0, mDiffLayer1);
          }
        } else {
          stat->Layer = 2;
          // lay1 new
          stat->Layer1TimeTrue = 0;

          CopyLayerPart(ii, jj, mSafeStride, mDiffLayer1, mDiffLayer2);
        }
      } else if (scene->DiffCount > 0) {
        stat->Layer = 0;
        stat->Layer0TimeFalse += mFrameMs;
        // lay0 small change apply
        if (stat->Layer0TimeFalse > kLayerSwitchTime) {
          CopyLayerPart(ii, jj, mSafeStride, mDiffLayer0, mDiffLayer2);
        }
      } else {
        // lay0 approve, lay1 erase
        stat->Layer = 0;
        stat->Layer0TimeTrue += mFrameMs;
        stat->Layer0TimeFalse = 0;
        stat->Layer1TimeTrue = 0;
      }

      scene++;
      stat++;
    }
  }
}

void MotionTracking::PreObjectsCreateNormal()
{
  PreObjectsMark();
  PreObjectsMarkSmooth();
  if (PreMarkValid()) {
    PreObjectsConnect(2);

    PreObjectsConstruct();
  }
  //ObjectVerify();
}

void MotionTracking::PreObjectsMark()
{
  mTotalObjects = 0;
  for (int jj = 0; jj < mSceneHeight; jj++) {
    Info* scene = &mSceneInfo[jj * mSceneWidth];
    Stat* stat = &mSceneStat[jj * mSceneWidth];
    int* tmp = &mSceneTmp[jj * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value;
      if ((stat->Flag & StatDif::eIgnore) == 0) {
        int diff = stat->DiffCount.Normalized(scene->DiffCount, kBlockSize * kBlockSize);
        int sharp = stat->DiffSharpCount.Normalized(scene->DiffSharpCount, kBlockSize * kBlockSize);
        value = 2 * diff * sharp;
      } else {
        value = 0;
      }

      scene->Object = *tmp = value;
      mTotalObjects += value;

      scene++;
      stat++;
      tmp++;
    }
  }
}

void MotionTracking::PreObjectsMarkSmooth()
{
  mSceneTmp2.assign(mSceneTmp.begin(), mSceneTmp.end());

  for (int jj = 1; jj < mSceneHeight - 1; jj++) {
    int* tmp = &mSceneTmp[jj * mSceneWidth + 1];
    int* tmp2 = &mSceneTmp2[jj * mSceneWidth + 1];
    for (int ii = 1; ii < mSceneWidth - 1; ii++) {
      int inc = *tmp2 / 2;
      if (inc) {
        *(tmp - mSceneWidth - 1) += inc;
        *(tmp - mSceneWidth - 0) += inc;
        *(tmp - mSceneWidth + 1) += inc;
        *(tmp - 1)               += inc;
        *(tmp + 1)               += inc;
        *(tmp + mSceneWidth - 1) += inc;
        *(tmp + mSceneWidth - 0) += inc;
        *(tmp + mSceneWidth + 1) += inc;
      }

      tmp++;
      tmp2++;
    }
  }
}

void MotionTracking::PreObjectsMarkSmooth2()
{
  mSceneTmp2.assign(mSceneTmp.begin(), mSceneTmp.end());

  for (int jj = 1; jj < mSceneHeight - 1; jj++) {
    int* tmp2 = &mSceneTmp2[jj * mSceneWidth + 1];
    for (int ii = 1; ii < mSceneWidth - 1; ii++) {
      int inc = *tmp2 / 2;
      for (int rad = 1; inc > 0; rad++, inc -= 1) {
        if (jj - rad >= 0) {
          int l = qMax(0, ii - rad);
          int r = qMin(mSceneWidth-1, ii + rad);
          int* tmp = &mSceneTmp[(jj - rad) * mSceneWidth + l];
          for (int i = l; i <= r; i++) {
            *tmp += inc;
            tmp++;
          }
        }
        if (jj + rad < mSceneHeight) {
          int l = qMax(0, ii - rad);
          int r = qMin(mSceneWidth-1, ii + rad);
          int* tmp = &mSceneTmp[(jj + rad) * mSceneWidth + l];
          for (int i = l; i <= r; i++) {
            *tmp += inc;
            tmp++;
          }
        }
        if (ii - rad >= 0) {
          int t = qMax(0, jj - (rad-1));
          int b = qMin(mSceneHeight-1, jj + (rad-1));
          int* tmp = &mSceneTmp[t * mSceneWidth + ii - rad];
          for (int j = t; j <= b; j++) {
            *tmp += inc;
            tmp += mSceneWidth;
          }
        }
        if (ii + rad < mSceneWidth) {
          int t = qMax(0, jj - (rad-1));
          int b = qMin(mSceneHeight-1, jj + (rad-1));
          int* tmp = &mSceneTmp[t * mSceneWidth + ii + rad];
          for (int j = t; j <= b; j++) {
            *tmp += inc;
            tmp += mSceneWidth;
          }
        }
      }

      tmp2++;
    }
  }
}

bool MotionTracking::PreMarkValid()
{
  int objectDiff = qAbs(mTotalObjectsLast - mTotalObjects);
  mTotalObjectsLast = mTotalObjects;
  mPreCountStat.Update(objectDiff);
  return mPreCountStat.Normalized(objectDiff) < 2;
}

void MotionTracking::PreObjectsConnect(int threshold)
{
  int currentObj = 0;

  for (int jj = 0; jj < mSceneHeight; jj++) {
    int* scene = &mSceneTmp[jj * mSceneWidth];
    int* object = &mSceneTmp2[jj * mSceneWidth];
    bool inObject = false;
    for (int ii = 0; ii < mSceneWidth; ii++) {
      bool isObject = *scene > threshold;
      if (isObject) {
        if (!inObject) {
          ++currentObj;
          inObject = true;
        }
        *object = currentObj;
      } else {
        if (inObject) {
          inObject = false;
        }
        *object = 0;
      }
      scene++;
      object++;
    }
  }

  mObjIds.resize(0);
  mObjIds.resize(currentObj + 1, 0);

  for (int jj = 0; jj < mSceneHeight - 1; jj++) {
    int* object1 = &mSceneTmp2[jj * mSceneWidth];
    int* object2 = &mSceneTmp2[(jj + 1) * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      if (*object1 > 0 && *object2 > 0) {
        int obj1 = *object1;
        while (mObjIds[obj1]) {
          obj1 = mObjIds[obj1];
        }
        int obj2 = *object2;
        while (mObjIds[obj2]) {
          obj2 = mObjIds[obj2];
        }
        if (obj1 != obj2) {
          int objNew = qMin(obj1, obj2);
          for (int obj = *object1; obj && obj != objNew; ) {
            int objn = mObjIds[obj];
            mObjIds[obj] = objNew;
            obj = objn;
          }
          for (int obj = *object2; obj && obj != objNew; ) {
            int objn = mObjIds[obj];
            mObjIds[obj] = objNew;
            obj = objn;
          }
        }
      }
      object1++;
      object2++;
    }
  }

  mPreObjectsCount = 0;
  for (int obj = 1; obj <= currentObj; obj++) {
    if (int objRef = mObjIds[obj]) {
      mObjIds[obj] = mObjIds[objRef];
    } else {
      mObjIds[obj] = mPreObjectsCount;
      mPreObjectsCount++;
    }
  }
}

void MotionTracking::PreObjectsConstruct()
{
  std::vector<ObjPre> preObjects(mPreObjectsCount);
  memset(preObjects.data(), 0, sizeof(ObjPre) * mPreObjectsCount);

  for (int jj = 0; jj < mSceneHeight; jj++) {
    int* object = &mSceneTmp2[jj * mSceneWidth];
    Info* scene = &mSceneInfo[jj * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      if (*object) {
        int obj = mObjIds[*object];
        ObjPre* objInfo = &preObjects[obj];
        objInfo->CMass.rx() += ii * scene->Object;
        objInfo->CMass.ry() += jj * scene->Object;
        objInfo->Mass += scene->Object;
        if (objInfo->Constructed) {
          objInfo->Place.Left = qMin(objInfo->Place.Left, ii);
          objInfo->Place.Right = qMax(objInfo->Place.Right, ii);
          objInfo->Place.Top = qMin(objInfo->Place.Top, jj);
          objInfo->Place.Bottom = qMax(objInfo->Place.Bottom, jj);
        } else {
          objInfo->Place.Left = objInfo->Place.Right = ii;
          objInfo->Place.Top = objInfo->Place.Bottom = jj;
          objInfo->Constructed = true;
        }
      }
      object++;
      scene++;
    }
  }

  mPreObjects.clear();
  for (int i = 0; i < (int)preObjects.size(); i++) {
    ObjPre pre = preObjects[i];
    int size = qMin(pre.Place.Width(), pre.Place.Height());
    if (size < mMinObjectSize) {
      continue;
    }

    if (pre.Mass > 0) {
      pre.CMass.rx() /= pre.Mass;
      pre.CMass.ry() /= pre.Mass;
    }
    int radius = qMin(pre.Place.Width()/2, pre.Place.Height()/2);
    pre.Radius = QPointF(radius, radius);

    pre.Place.Left = pre.Place.Left * kBlockSize;
    pre.Place.Right = pre.Place.Right * kBlockSize + kBlockSize;
    pre.Place.Top = pre.Place.Top * kBlockSize;
    pre.Place.Bottom = pre.Place.Bottom * kBlockSize + kBlockSize;
    mPreObjects.push_back(pre);
  }
}

void MotionTracking::PreObjectsConstructDiff()
{
  std::vector<ObjPre> preObjects(mPreObjectsCount);
  memset(preObjects.data(), 0, sizeof(ObjPre) * mPreObjectsCount);

  for (int jj = 0; jj < mSceneHeight; jj++) {
    int* object = &mSceneTmp2[jj * mSceneWidth];
    InfoDif* scene = &mSceneDiffInfo[jj * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      if (*object) {
        int obj = mObjIds[*object];
        ObjPre* objInfo = &preObjects[obj];
        objInfo->CMass.rx() += ii * scene->Object;
        objInfo->CMass.ry() += jj * scene->Object;
        objInfo->Mass += scene->Object;
        if (objInfo->Constructed) {
          objInfo->Place.Left = qMin(objInfo->Place.Left, ii);
          objInfo->Place.Right = qMax(objInfo->Place.Right, ii);
          objInfo->Place.Top = qMin(objInfo->Place.Top, jj);
          objInfo->Place.Bottom = qMax(objInfo->Place.Bottom, jj);
        } else {
          objInfo->Place.Left = objInfo->Place.Right = ii;
          objInfo->Place.Top = objInfo->Place.Bottom = jj;
          objInfo->Constructed = true;
        }
      }
      object++;
      scene++;
    }
  }

  mPreObjects.clear();
  for (int i = 0; i < (int)preObjects.size(); i++) {
    ObjPre pre = preObjects[i];
    pre.Id = i;
    int size = qMin(pre.Place.Width(), pre.Place.Height());
    if (size < mMinObjectSize) {
      continue;
    }

    if (pre.Mass > 0) {
      pre.CMass.rx() /= pre.Mass;
      pre.CMass.ry() /= pre.Mass;
    }
    pre.Radius = QPointF(pre.Place.Width()/2, pre.Place.Height()/2);

    pre.Place.Left = pre.Place.Left * kBlockSize;
    pre.Place.Right = pre.Place.Right * kBlockSize + kBlockSize;
    pre.Place.Top = pre.Place.Top * kBlockSize;
    pre.Place.Bottom = pre.Place.Bottom * kBlockSize + kBlockSize;
    mPreObjects.push_back(pre);
  }
}

void MotionTracking::PreObjectsCreateMacro()
{
  PreObjectsMarkMacro();
  PreObjectsMarkSmooth();

  PreObjectsConnect(2);

  PreObjectsConstructMacro();
}

void MotionTracking::PreObjectsMarkMacro()
{
  mTotalObjects = 0;
  for (int jj = 0; jj < mSceneHeight; jj++) {
    Info* scene = &mSceneInfo[jj * mSceneWidth];
    Stat* stat = &mSceneStat[jj * mSceneWidth];
    int* tmp = &mSceneTmp[jj * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int diff = stat->DiffCount.Normalized(scene->DiffCount, kBlockSize * kBlockSize);
//      int sharp = stat->DiffSharpCount.Normalized(scene->DiffSharpCount, kBlockSize * kBlockSize);

      *tmp = diff;
      mTotalObjects += diff;

      scene++;
      stat++;
      tmp++;
    }
  }
}

void MotionTracking::PreObjectsConstructMacro()
{
  QVector<ObjPre> preObjects(mPreObjectsCount);
  memset(preObjects.data(), 0, sizeof(ObjPre) * mPreObjectsCount);
  for (int jj = 0; jj < mSceneHeight; jj++) {
    Info* scene = &mSceneInfo[jj * mSceneWidth];
    Stat* stat = &mSceneStat[jj * mSceneWidth];
    int* tmp = &mSceneTmp[jj * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      if (*tmp) {
        int obj = mObjIds[*tmp];
        ObjPre& objInfo = preObjects[obj];

        int sharp = stat->DiffSharpCount.Normalized(scene->DiffSharpCount, kBlockSize * kBlockSize);

        objInfo.CMass.rx() += sharp * ii;
        objInfo.CMass.ry() += sharp * jj;
        objInfo.Mass += sharp;
        objInfo.CSize.rx() += ii;
        objInfo.CSize.ry() += jj;
        objInfo.Size++;
      }

      scene++;
      stat++;
      tmp++;
    }
  }

  for (int i = 0; i < (int)preObjects.size(); i++) {
    ObjPre* pre = &preObjects[i];
    if (pre->Mass) {
      pre->CMass.rx() /= pre->Mass;
      pre->CMass.ry() /= pre->Mass;
    }
    if (pre->Size) {
      pre->CSize.rx() /= pre->Size;
      pre->CSize.ry() /= pre->Size;
    }
  }

  for (int jj = 0; jj < mSceneHeight; jj++) {
    Info* scene = &mSceneInfo[jj * mSceneWidth];
    Stat* stat = &mSceneStat[jj * mSceneWidth];
    int* tmp = &mSceneTmp[jj * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      if (*tmp) {
        int obj = mObjIds[*tmp];
        ObjPre& objInfo = preObjects[obj];

        int sharp = stat->DiffSharpCount.Normalized(scene->DiffSharpCount, kBlockSize * kBlockSize);

        if (sharp) {
          objInfo.Radius.rx() += sharp * (objInfo.CMass.x() - ii) * (objInfo.CMass.x() - ii);
          objInfo.Radius.ry() += sharp * (objInfo.CMass.y() - jj) * (objInfo.CMass.y() - jj);
        }
      }

      scene++;
      stat++;
      tmp++;
    }
  }

  for (int i = 0; i < (int)preObjects.size(); i++) {
    ObjPre* pre = &preObjects[i];
    if (pre->Mass) {
      pre->Radius.rx() = sqrt(pre->Radius.x() / pre->Mass);
      pre->Radius.ry() = sqrt(pre->Radius.y() / pre->Mass);
    }
  }

  int minSquareSize = mMinObjectSize * mMinObjectSize / 4;
  int minMass = 8 * mMinObjectSize;
  mPreObjects.clear();
  for (int i = 0; i < (int)preObjects.size(); i++) {
    ObjPre pre = preObjects[i];
    if (pre.Size < minSquareSize || pre.Mass < minMass) {
      continue;
    }

    pre.Place.Left = qMax(0, (int)(pre.CMass.x() - pre.Radius.x()) * kBlockSize);
    pre.Place.Right = qMin(getWidth(), (int)(pre.CMass.x() + pre.Radius.x()) * kBlockSize + kBlockSize);
    pre.Place.Top = qMax(0, (int)(pre.CMass.y() - pre.Radius.y()) * kBlockSize);
    pre.Place.Bottom = qMin(getHeight(), (int)(pre.CMass.y() + pre.Radius.y()) * kBlockSize + kBlockSize);
    mPreObjects.push_back(pre);
  }
}

void MotionTracking::PreObjectsCreateDiff()
{
  PreObjectsMarkDiff();
  PreObjectsMarkSmooth();
  PreObjectsConnect(2);
  PreObjectsConstructDiff();
}

void MotionTracking::PreObjectsMarkDiff()
{
  for (int jj = 0; jj < mSceneHeight; jj++) {
    InfoDif* scene = &mSceneDiffInfo[jj * mSceneWidth];
    StatDif* stat = &mSceneDiffStat[jj * mSceneWidth];
    int* tmp = &mSceneTmp[jj * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value;
      if ((stat->Flag & StatDif::eIgnore) == 0) {
        value = DiffCountFormula(scene->DiffCount, kLayer0DiffCountThreshold);
      } else {
        value = 0;
      }
      scene->Object = *tmp = value;

      scene++;
      stat++;
      tmp++;
    }
  }
}

void MotionTracking::ObjectsManage()
{
  ObjectsLinkPre();
  ObjectsMoveAll();
  ObjectsUpdateSpeedStat();
}

void MotionTracking::ObjectsLinkPre()
{
  for (auto itr = mObject2Info.begin(); itr != mObject2Info.end(); itr++) {
    Obj2Info* object = &*itr;
    object->PreLink = 0;
    if (object->Mode != Obj2Info::eEnded) {
      Object2LinkOne(object);
    } else {
      object->Quality -= 2 * mFrameMs;
      if (object->Quality < 0) {
        object->Mode = Obj2Info::eModeIllegal;
      }
    }
  }

  ObjectsDevideMultiPre();
}

void MotionTracking::ObjectsDevideMultiPre()
{
  int newObjects = 0;
  Rectangle useRect(mSceneWidth, mSceneHeight, 0, 0);

  for (int i = 0; i < (int)mPreObjects.size(); i++) {
    const ObjPre& pre = mPreObjects[i];
    if (pre.Used > 1) {
      // remove bad links
      for (auto itr = mObject2Info.begin(); itr != mObject2Info.end(); itr++) {
        Obj2Info* object = &*itr;
        if (object->PreLink == i + 1 && object->Mode != Obj2Info::eNormal) {
          mPreObjects[i].Used--;
          object->PreLink = 0;
        }
      }

      if (pre.Used > 1) {
        newObjects += pre.Used;
        useRect.Left = qMin(pre.Place.Left/kBlockSize, useRect.Left);
        useRect.Top = qMin(pre.Place.Top/kBlockSize, useRect.Top);
        useRect.Right = qMax(pre.Place.Right/kBlockSize, useRect.Right);
        useRect.Bottom = qMax(pre.Place.Bottom/kBlockSize, useRect.Bottom);
      }
    }
  }
  if (newObjects == 0) {
    return;
  }

  int oldObjects = (int)mPreObjects.size();
  mPreObjects.resize(oldObjects + newObjects);
  memset(mPreObjects.data() + oldObjects, 0, newObjects * sizeof(ObjPre));

  QMap<int, QPair<int, int> > objMap;
  int addObjects = 0;
  for (int i = 0; i < oldObjects; i++) {
    const ObjPre& pre = mPreObjects[i];
    if (pre.Used > 1) {
      objMap[pre.Id] = qMakePair(i, oldObjects + addObjects);

      for (auto itr = mObject2Info.begin(); itr != mObject2Info.end(); itr++) {
        Obj2Info* object = &*itr;
        if (object->PreLink == i + 1) {
          ObjPre* newPre = &mPreObjects[oldObjects + addObjects];
          newPre->Id = oldObjects + addObjects;
          newPre->Used = 1;

          //object->PreLink = oldObjects + addObjects + 1;
          addObjects++;
        }
      }
    }
  }
  Q_ASSERT(newObjects == addObjects);

  // reorder points between objects
  for (int jj = useRect.Top; jj < useRect.Bottom; jj++) {
    int* objLink = &mSceneTmp2[jj * mSceneWidth + useRect.Left];
    InfoDif* scene = &mSceneDiffInfo[jj * mSceneWidth + useRect.Left];
    for (int ii = useRect.Left; ii < useRect.Right; ii++) {
      if (*objLink) {
        int obj = mObjIds[*objLink];
        auto itr = objMap.find(obj);
        if (itr != objMap.end()) {
          // if pre object of multi-use
          QPair<int, int> objInfo = itr.value();
          int preMulti = objInfo.first;
          int preNew = objInfo.second;
          float fitValue = 0;
          int preNewFit = 0;
          for (auto itr = mObject2Info.begin(); itr != mObject2Info.end(); itr++) {
            Obj2Info* object = &*itr;
            if (object->PreLink == preMulti + 1) {
              float newFit = Object2FitPoint(object, ii, jj);
              if (newFit > fitValue) {
                preNewFit = preNew;
                fitValue = newFit;
              }
              preNew++;
            }
          }

          ObjPre* pre = &mPreObjects[preNewFit];
          pre->CMass.rx() += ii * scene->Object;
          pre->CMass.ry() += jj * scene->Object;
          pre->Mass += scene->Object;
          if (pre->Constructed) {
            pre->Place.Left = qMin(pre->Place.Left, ii);
            pre->Place.Right = qMax(pre->Place.Right, ii);
            pre->Place.Top = qMin(pre->Place.Top, jj);
            pre->Place.Bottom = qMax(pre->Place.Bottom, jj);
          } else {
            pre->Place.Left = pre->Place.Right = ii;
            pre->Place.Top = pre->Place.Bottom = jj;
            pre->Constructed = true;
          }
        }
      }

      objLink++;
      scene++;
    }
  }

  // extra calc for new pre
  for (int i = 0; i < newObjects; i++) {
    ObjPre* pre = &mPreObjects[i + oldObjects];

    if (pre->Constructed) {
      if (pre->Mass > 0) {
        pre->CMass.rx() /= pre->Mass;
        pre->CMass.ry() /= pre->Mass;
      }
      int radius = qMin(pre->Place.Width()/2, pre->Place.Height()/2);
      pre->Radius = QPointF(radius, radius);

      pre->Place.Left = pre->Place.Left * kBlockSize;
      pre->Place.Right = pre->Place.Right * kBlockSize + kBlockSize;
      pre->Place.Top = pre->Place.Top * kBlockSize;
      pre->Place.Bottom = pre->Place.Bottom * kBlockSize + kBlockSize;
    } else {
      if (i < (int)mPreObjects.size() - 1) {
        mPreObjects[i] = mPreObjects.back();
        i--;
      }
      mPreObjects.pop_back();
      newObjects--;
    }
  }

  // reassign to new pre
  //QMap<int, QPair<int, int> > objMap;
  for (auto itr = objMap.begin(); itr != objMap.end(); itr++) {
    QPair<int, int> objInfo = itr.value();
    int preMulti = objInfo.first;
    int preNew = objInfo.second;

    for (auto itr = mObject2Info.begin(); itr != mObject2Info.end(); itr++) {
      Obj2Info* object = &*itr;
      if (object->PreLink == preMulti + 1) {
        object->PreLink = preNew + 1;
        preNew++;
      }
    }
  }

  //QString infoPre = "---------- PRE ----------\n";
  //for (int i = 0; i < mPreObjects.size(); i++) {
  //  const ObjPre& pre = mPreObjects[i];
  //  infoPre += QString("[%1]: w: (%2, %3, %4, %5), used: %6\n").arg(i).arg(pre.Place.Left).arg(pre.Place.Top).arg(pre.Place.Right).arg(pre.Place.Bottom).arg(pre.Used);
  //}

  //infoPre += "---------- OBJ ----------\n";
  //for (int i = 0; i < mObject2Info.size(); i++) {
  //  const Obj2Info& obj = mObject2Info[i];
  //  infoPre += QString("[%1]: L: (%2, %3), r: (%4, %5), link: %6\n").arg(i).arg(obj.Location.x()).arg(obj.Location.y()).arg(obj.Radius.x()).arg(obj.Radius.y()).arg(obj.PreLink);
  //}
  //infoPre += "=========================";
  //Log.Debug(infoPre);
}

void MotionTracking::ObjectsMoveAll()
{
  for (auto itr = mObject2Info.begin(); itr != mObject2Info.end(); itr++) {
    Obj2Info* object = &*itr;
    Object2ManageOne(object);
  }

  for (auto itr = mPreObjects.begin(); itr != mPreObjects.end(); itr++) {
    ObjPre* pre = &*itr;
    if (pre->Used == 0) {
      Object2CreateOne(pre);
    }
  }

  for (auto itr = mObject2Info.begin(); itr != mObject2Info.end(); ) {
    Obj2Info* object = &*itr;
    if (object->Mode & Obj2Info::eDeleted) {
      for (auto itr_ = object->BariersHit.begin(); itr_ != object->BariersHit.end(); itr_++) {
        int id = itr_.key();
        const HitInf& inf = itr_.value();
        mBariers[id].Detector->Hit(inf.Time, inf.In? "in": "out");
      }
      Log.Trace(QString("Remove obj (id: %1, type: %2, count: %3)")
                  .arg(object->Id).arg(ObjModeToString((Obj2Info::EMode)(object->Mode ^ Obj2Info::eDeleted))).arg(mObject2Info.size()));
      itr = mObject2Info.erase(itr);
    } else {
      itr++;
    }
  }
}

void MotionTracking::ObjectsUpdateSpeedStat()
{
  if (mObject2Info.size() <= 3) {
    for (auto itr = mObject2Info.begin(); itr != mObject2Info.end(); itr++) {
      Obj2Info* object = &*itr;
      if (object->Mode != Obj2Info::eNormal) {
        continue;
      }

      bool ok = true;
      for (auto itr = mObject2Info.begin(); itr != mObject2Info.end(); itr++) {
        Obj2Info* object2 = &*itr;
        if (object != object2
          && (qAbs(object->Location.x() - object2->Location.x()) < 1.5f * (object->Radius.x() + object2->Radius.x())
          || qAbs(object->Location.y() - object2->Location.y()) < 1.5f * (object->Radius.y() + object2->Radius.y())) ) {
            ok = false;
            break;
        }
      }
      if (ok) {
        QPointF v = PointScreenToPercent(object->Speed);
        float avg2 = (qAbs(v.x()) + qAbs(v.y()));
        float avgh = avg2 * 1.28f; // (x + y)/2 * 256 / 100
        mObjectSpeedHyst.Inc(qMin((int)avgh, 255));
        int index = mObjectSpeedHyst.GetValue(900);
        if (index > mObjectMaxSpeed * 2.56f) {
          mObjectMaxSpeed = (index + 0.5f) / 2.56f;
          mObjectMaxAccel = 4.0f * mObjectMaxSpeed;
          //Log.Debug(QString("Speed max increase: %1").arg(mObjectMaxSpeed, 0, 'f', 2));
        }
      }
    }
  }
}

float MotionTracking::Object2FitPoint(const Obj2Info* object, int ii, int jj)
{
  float posx = object->NewLocation.x();
  float posy = object->NewLocation.y();
  float rx = qMax((float)object->Radius.x(), 1.0f);
  float ry = qMax((float)object->Radius.y(), 1.0f);

  float dx = qAbs((ii - posx) / rx);
  float dy = qAbs((jj - posy) / ry);

  float d = qMax(dx*dx + dy*dy, 0.01f);
  return 1.0f / d;
}

void MotionTracking::Object2LinkOne(Obj2Info *object)
{
  float t = mFrameMs * 0.001f;
  float dx = object->Speed.x() * t;
  float dy = object->Speed.y() * t;
  float posx = object->NewLocation.rx() = object->Location.x() + dx;
  float posy = object->NewLocation.ry() = object->Location.y() + dy;

  float rx = object->Radius.x() + qAbs(dx);
  float ry = object->Radius.y() + qAbs(dy);

  float fitMax = mObjectFitMin;
  object->PreLink = 0;
  for (int i = 0; i < (int)mPreObjects.size(); i++) {
    ObjPre* preObject = &mPreObjects[i];

    float dcx = qAbs(preObject->CMass.x() - posx);
    float dcy = qAbs(preObject->CMass.y() - posy);
    float dmx = preObject->Radius.x() + rx;
    float dmy = preObject->Radius.y() + ry;
    float dlx = dcx / dmx;
    float dly = dcy / dmy;
    float fit = dlx*dlx + dly*dly;

    if (fit < fitMax) {
      object->PreLink = i + 1;
      fitMax = fit;
    }
  }

  if (object->PreLink) {
    if (mPreObjects.size() == 1 && mObject2Info.size() == 1) {
      int val = qMin(255, (int)(fitMax * 5000.0f));
      mObjectFitHyst.Inc(val);
      if (mObjectFitHyst.TotalCount() > 10000) {
        float fitAvg = 0.0002f * mObjectFitHyst.GetValue(990);
        if (mObjectFitMin > 4.0f * fitAvg) {
          mObjectFitMin = 4.0f * fitAvg;
          Log.Info(QString("Fit threshold down to %1").arg(mObjectFitMin, 0, 'f', 8));
        } else if (mObjectFitMin < 0.2f * fitAvg) {
          mObjectFitMin = 0.2f * fitAvg;
          Log.Info(QString("Fit threshold up to %1").arg(mObjectFitMin, 0, 'f', 8));
        }
      }
    }
    mPreObjects[object->PreLink - 1].Used++;
  }
}

void MotionTracking::Object2CreateOne(ObjPre *preObject)
{
  Obj2Info obj;
  obj.Id = mObjectNextId++;
  obj.EmergeLocation = obj.Location = preObject->CMass;
  obj.Speed = QPointF(0, 0);
  obj.Radius = preObject->Radius;
  obj.PreLink = 0;
  obj.Quality = 0;
  obj.Time = 0;
  obj.Mode = (IsNearBorder(obj))? Obj2Info::eNormal: Obj2Info::eCasper;

  mObject2Info.append(obj);
  Log.Trace(QString("Create new obj (id: %1, type: %2, count: %3)")
            .arg(obj.Id).arg(ObjModeToString(obj.Mode)).arg(mObject2Info.size()));
}

void MotionTracking::Object2ManageOne(Obj2Info* object)
{
  if (object->PreLink) {
    if (object->Mode == Obj2Info::eLost) {
      object->Mode = Obj2Info::eNormal;
    }
    ObjPre* preObject = &mPreObjects[object->PreLink-1];
    float v2x = (preObject->CMass.x() - object->Location.x()) / mTimeSec;
    float v2y = (preObject->CMass.y() - object->Location.y()) / mTimeSec;
    float dvx = v2x - object->Speed.x();
    float dvy = v2y - object->Speed.y();
    NormalizeValue(dvx, mObjectMaxAccel * mTimeSec);
    NormalizeValue(dvy, mObjectMaxAccel * mTimeSec);

    object->Speed.rx() += dvx;
    object->Speed.ry() += dvy;
    NormalizeValue(object->Speed.rx(), mObjectMaxSpeed);
    NormalizeValue(object->Speed.ry(), mObjectMaxSpeed);

    MoveObject(object);

    float drx = preObject->Radius.x() - object->Radius.x();
    float dry = preObject->Radius.y() - object->Radius.y();
    if (drx < -kMaxRadiusChange * mTimeSec) {
      drx = -kMaxRadiusChange * mTimeSec;
    } else if (drx > kMaxRadiusChange * mTimeSec) {
      drx = kMaxRadiusChange * mTimeSec;
    }
    if (dry < -kMaxRadiusChange * mTimeSec) {
      dry = -kMaxRadiusChange * mTimeSec;
    } else if (dry > kMaxRadiusChange * mTimeSec) {
      dry = kMaxRadiusChange * mTimeSec;
    }
    object->Radius.rx() += drx * mTimeSec;
    object->Radius.ry() += dry * mTimeSec;

    if (qAbs(object->Location.x() - preObject->CMass.x()) < 0.5f * object->Radius.x()
      && qAbs(object->Location.y() - preObject->CMass.y()) < 0.5f * object->Radius.y()) {
      object->Quality += mFrameMs;
    } else {
      object->Quality -= mFrameMs;
    }

    if (object->Mode == Obj2Info::eCasper) {
      if (qAbs(object->Location.x() - object->EmergeLocation.x()) + qAbs(object->Location.y() - object->EmergeLocation.y())
          > (mSceneWidth + mSceneHeight) / 4) {
        if (object->Quality > 500) {
          object->Mode = Obj2Info::eNormal;
        } else if (object->Quality < -500) {
          object->Mode = Obj2Info::eModeIllegal;
        }
      }
    }
    preObject->Used = true;
  } else {
    if (object->Mode == Obj2Info::eNormal) {
      object->EmergeLocation = object->Location;
      object->Location = object->NewLocation;
      if (object->Quality >= 500 && IsNearBorder(*object)) {
        object->Mode = Obj2Info::eEnded;
      } else {
        object->Mode = Obj2Info::eLost;
        object->Location = object->EmergeLocation;
      }
    }
    object->Quality -= mFrameMs;
    object->Time += mFrameMs;
  }

  if (object->Quality > 1000) {
    object->Quality = 1000;
  } else if (object->Quality < -1000) {
    if (object->Mode == Obj2Info::eNormal || object->Mode == Obj2Info::eLost) {
      object->Mode = Obj2Info::eCasper;
      object->Quality = 0;
    } else {
      object->Mode = Obj2Info::eModeIllegal;
    }
  }
}

void MotionTracking::MoveObject(Obj2Info *object)
{
  float p1x = object->Location.x();
  float p1y = object->Location.y();
  float p2x = p1x + object->Speed.x() * mTimeSec;
  float p2y = p1y + object->Speed.y() * mTimeSec;

  if (object->Mode == Obj2Info::eNormal && p1x > 0 && p1x < mSceneWidth && p1y > 0 && p1y < mSceneHeight) {
    QPointF& pp1 = object->Location;
    QPointF pp2 = QPointF(p2x, p2y);

    for (int i = 0; i < mBariers.size(); i++) {
      const PointList& barier = mBariers[i].Points;

      int inOut = IntersectBarier(pp1, pp2, barier);
      if (inOut) {
        auto itr = object->BariersHit.find(i);
        if (itr != object->BariersHit.end()) {
          if (itr->In == (inOut > 0)) {
            Log.Warning(QString("Object in/out twice"));
            itr->Time = mLastTimestamp;
          } else {
            object->BariersHit.erase(itr);
            Log.Trace(QString("[%1] Object %2: %3 cancel").arg(i).arg(object->Id).arg((inOut > 0)? "in": "out"));
          }
        } else {
          HitInf inf;
          inf.In = (inOut > 0);
          inf.Time = mLastTimestamp;
          object->BariersHit[i] = inf;
          Log.Trace(QString("[%1] Object %2: %3").arg(i).arg(object->Id).arg((inOut > 0)? "in": "out"));
        }
      }
    }
  }
  object->Location.rx() = p2x;
  object->Location.ry() = p2y;
}

int MotionTracking::IntersectBarier(const QPointF& pp1, const QPointF& pp2, const PointList& barier)
{
  bool in1 = true;
  bool in2 = true;

  auto itr = barier.begin();
  QPointF p1 = PointPercentToScreen(*itr);
  for (itr++; itr != barier.end(); itr++) {
    QPointF p2 = PointPercentToScreen(*itr);

    QPointF v1 = pp1 - p1;
    QPointF v2 = p2 - p1;
    float vp1 = v1.x()*v2.y() - v2.x()*v1.y();
    if (vp1 > 0) {
      in1 = false;
    }

    v1 = pp2 - p1;
    float vp2 = v1.x()*v2.y() - v2.x()*v1.y();
    if (vp2 > 0) {
      in2 = false;
    }

    p1 = p2;
  }

  if (in1 && !in2) {
    return 1;
  } else if (!in1 && in2) {
    return -1;
  } else {
    return 0;
  }
}

void MotionTracking::ObjectVerify()
{
  for (auto itr = mObjectInfo.begin(); itr != mObjectInfo.end(); itr++) {
    ObjInfo& obj = *itr;
    ObjectLinkOne(obj);
  }

  //int counter = 0;
  for (auto itr = mObjectInfo.begin(); itr != mObjectInfo.end(); ) {
    ObjInfo& obj = *itr;
    //Log.Debug(QString("Object verify %1").arg(++counter));
    if (ObjectVerifyOne(obj)) {
      //Log.Debug(QString("--> (%1, %2, %3, %4)")
      //          .arg(obj.Place.Left).arg(obj.Place.Top).arg(obj.Place.Right).arg(obj.Place.Bottom));
      obj.Time += mFrameMs;
      itr++;
    } else {
      itr = mObjectInfo.erase(itr);
    }
  }

  bool newValid = PreMarkValid();
  if (newValid) {
    for (int i = 0; i < (int)mPreObjects.size(); i++) {
      if (!mPreObjects[i].Used) {
        ObjInfo obj;
        obj.Init(mPreObjects[i].Place);
        mObjectInfo.append(obj);
      }
    }
  }
}

void MotionTracking::ObjectLinkOne(ObjInfo &object)
{
  int dx = object.Speed.X * mFrameMs / 100000;
  int dy = object.Speed.Y * mFrameMs / 100000;
  object.NewPlace.Left = object.Place.Left + dx;
  object.NewPlace.Right = object.Place.Right + dx;
  object.NewPlace.Top = object.Place.Top + dy;
  object.NewPlace.Bottom = object.Place.Bottom + dy;
  Point moveThreshold;
  moveThreshold.X = (object.NewPlace.Width()) * 3/4 + qAbs(dx) * 1/2;
  moveThreshold.Y = (object.NewPlace.Height()) * 3/4 + qAbs(dy) * 1/2;
  int sizeThreshold = (object.NewPlace.Width() + object.NewPlace.Height()) * 1/4;

  bool goodFit = false;
  object.PreLinkGood = nullptr;
  object.PreLinkBad.clear();
  for (int i = 0; i < (int)mPreObjects.size(); i++)
  {
    ObjPre& preObject = mPreObjects[i];
    int fitQuality = ObjectFitPreCalc(object.NewPlace, preObject.Place, moveThreshold, sizeThreshold);
    if (fitQuality >= 100) {
      preObject.Used += 100;
      if (!goodFit) {
        object.PreLinkGood = &preObject;
      } else if (object.PreLinkGood) {
        ObjLink link;
        link.PreObject = &preObject;
        link.Quality = fitQuality;
        object.PreLinkBad.append(link);

        link.PreObject = object.PreLinkGood;
        link.Quality = 100;
        object.PreLinkBad.append(link);
        object.PreLinkGood = nullptr;
      }
    } else {
      preObject.Used += 1;
      ObjLink link;
      link.PreObject = &preObject;
      link.Quality = fitQuality;
      object.PreLinkBad.append(link);
    }
  }
}

bool MotionTracking::ObjectVerifyOne(ObjInfo &object)
{
  if (object.PreLinkGood) {
    if (object.PreLinkGood->Used == 100) {
      //Log.Debug("good");
      object.Quality = qMin(100, object.Quality + kQualityUp * mFrameMs / 1000);
      ObjectModifyGoodFit(object);
      return true;
    } else {
      ObjLink link;
      link.PreObject = object.PreLinkGood;
      link.Quality = 100;
      object.PreLinkBad.append(link);
      object.PreLinkGood = nullptr;
    }
  }

  int qualityDelta = 0;
  if (!object.PreLinkBad.isEmpty()) {
    //Log.Debug(QString("bad %1").arg(object.PreLinkBad.size()));
    Rectangle sumRect;
    auto itr = object.PreLinkBad.begin();
    sumRect = itr->PreObject->Place;
    for (; itr != object.PreLinkBad.end(); itr++) {
      Rectangle& addRect = itr->PreObject->Place;
      sumRect.Left = qMin(sumRect.Left, addRect.Left);
      sumRect.Top = qMin(sumRect.Top, addRect.Top);
      sumRect.Right = qMax(sumRect.Right, addRect.Right);
      sumRect.Bottom = qMax(sumRect.Bottom, addRect.Bottom);
    }
    //Log.Debug(QString("sum rect (%1, %2, %3, %4)")
    //          .arg(sumRect.Left).arg(sumRect.Top).arg(sumRect.Right).arg(sumRect.Bottom));

    Rectangle& newPlace = object.NewPlace;
    if (sumRect.Left >= newPlace.Left && sumRect.Right <= newPlace.Right) {
      //Log.Debug("x a");
      int d = newPlace.Width() - sumRect.Width();
      qualityDelta = -d * 10 / newPlace.Width();
    } else if (sumRect.Left <= newPlace.Left && sumRect.Right >= newPlace.Right) {
      //Log.Debug("x b");
      int d = sumRect.Width() - newPlace.Width();
      qualityDelta = -d * 10 / newPlace.Width();
    } else if (sumRect.Left < newPlace.Left) {
      //Log.Debug("x c");
      int l = (newPlace.Right - sumRect.Right) * mFrameMs / 1000;
      newPlace.Left -= l;
      newPlace.Right -= l;
      int d = qAbs(newPlace.Width() - sumRect.Width());
      qualityDelta = -d * 10 / newPlace.Width();
    } else if (sumRect.Right > newPlace.Right) {
      //Log.Debug("x d");
      int l = (sumRect.Left - newPlace.Left) * mFrameMs / 1000;
      newPlace.Left += l;
      newPlace.Right += l;
      int d = qAbs(newPlace.Width() - sumRect.Width());
      qualityDelta = -d * 10 / newPlace.Width();
    }

    if (sumRect.Top >= newPlace.Top && sumRect.Bottom <= newPlace.Bottom) {
      //Log.Debug("y a");
      int d = newPlace.Height() - sumRect.Height();
      qualityDelta = -d * 10 / newPlace.Height();
    } else if (sumRect.Top <= newPlace.Top && sumRect.Bottom >= newPlace.Bottom) {
      //Log.Debug("y b");
      int d = sumRect.Height() - newPlace.Height();
      qualityDelta = -d * 10 / newPlace.Height();
    } else if (sumRect.Top < newPlace.Top) {
      //Log.Debug("y c");
      int l = (newPlace.Bottom - sumRect.Bottom) * mFrameMs / 1000;
      newPlace.Top -= l;
      newPlace.Bottom -= l;
      int d = qAbs(newPlace.Height() - sumRect.Height());
      qualityDelta = -d * 10 / newPlace.Height();
    } else if (sumRect.Bottom > newPlace.Bottom) {
      //Log.Debug("y d");
      int l = (sumRect.Top - newPlace.Top) * mFrameMs / 1000;
      newPlace.Top += l;
      newPlace.Bottom += l;
      int d = qAbs(newPlace.Height() - sumRect.Height());
      qualityDelta = -d * 10 / newPlace.Height();
    }
  } else {
    //Log.Debug("bad none");
    qualityDelta = 20;
  }

  object.Quality += qualityDelta * mFrameMs / 1000;
  if (object.Quality > 100) {
    object.Quality = 100;
  }
  //Log.Debug(QString("quality %1 (%2)").arg(object.Quality).arg(qualityDelta));
  if (object.Quality < -50 || (object.Quality < 0 && object.Time > 2000)) {
    return false;
  }
  ObjectModifyBadFit(object);
  return true;
}

void MotionTracking::ObjectModifyGoodFit(ObjInfo &object)
{
  Rectangle& bestPlace = object.PreLinkGood->Place;
  Rectangle& newPlace = object.NewPlace;
  int dl = bestPlace.Left - newPlace.Left;
  int dr = bestPlace.Right - newPlace.Right;
  int dt = bestPlace.Top - newPlace.Top;
  int db = bestPlace.Bottom - newPlace.Bottom;

  int dx;
  if (qAbs(dl) <= qAbs(dr)) {
    dx = dl;
  } else {
    dx = dr;
  }
  int dw = bestPlace.Width() - newPlace.Width();
  if (qAbs(dw) > qAbs(dx)) {
    dw = (dw > 0)? qAbs(dx): -qAbs(dx);
  }
  newPlace.Left += dx;
  newPlace.Right += dx;

  int dy;
  if (qAbs(dt) <= qAbs(db)) {
    dy = dt;
  } else {
    dy = db;
  }
  int dh = bestPlace.Height() - newPlace.Height();
  if (qAbs(dh) > qAbs(dy)) {
    dh = (dh > 0)? qAbs(dy): -qAbs(dy);
  }
  newPlace.Top += dy;
  newPlace.Bottom += dy;
}

void MotionTracking::ObjectModifyBadFit(ObjInfo &object)
{
  ObjectModifyFinal(object);
}

void MotionTracking::ObjectModifyFinal(ObjInfo &object, int deltaWidth, int deltaHeight)
{
  int speedX = object.NewPlace.Left - object.Place.Left;
  object.Place.Left = object.NewPlace.Left - deltaWidth/2;
  object.Place.Right = object.NewPlace.Right + deltaWidth/2;
  if (mFrameMs < 500) {
    object.Speed.X = (object.Speed.X * 500 + speedX * mFrameMs) / (500 + mFrameMs);
  } else {
    object.Speed.X = (object.Speed.X + speedX) / 2;
  }

  int speedY = object.NewPlace.Top - object.Place.Top;
  object.Place.Top = object.NewPlace.Top - deltaHeight/2;
  object.Place.Bottom = object.NewPlace.Bottom + deltaHeight/2;
  if (mFrameMs < 500) {
    object.Speed.Y = (object.Speed.Y * 500 + speedY * mFrameMs) / (500 + mFrameMs);
  } else {
    object.Speed.Y = (object.Speed.Y + speedY) / 2;
  }
}

//bool MotionTracking::ObjectFitPreGood(const Rectangle& newPlace, const Rectangle& testPlace, const Point& moveThreshold, int sizeThreshold)
//{
//  if (qMax(qAbs(testPlace.Left - newPlace.Left), qAbs(testPlace.Right - newPlace.Right)) > moveThreshold.X) {
//    return false;
//  }
//  if (qMax(qAbs(testPlace.Top - newPlace.Top), qAbs(testPlace.Bottom - newPlace.Bottom)) > moveThreshold.Y) {
//    return false;
//  }
//  if (qAbs(testPlace.Width() - newPlace.Width()) + qAbs(testPlace.Height() - newPlace.Height()) > sizeThreshold) {
//    return false;
//  }
//  return true;
//}

int MotionTracking::ObjectFitPreCalc(const Rectangle &newPlace, const Rectangle &testPlace, const Point &moveThreshold, int sizeThreshold)
{
  int quality = 100;
  int dx = qMax(qAbs(testPlace.Left - newPlace.Left), qAbs(testPlace.Right - newPlace.Right));
  if (dx >= 2 * moveThreshold.X) {
    return 0;
  } else if (dx > moveThreshold.X) {
    quality -= (dx - moveThreshold.X) * 35 / moveThreshold.X;
  }
  int dy = qMax(qAbs(testPlace.Top - newPlace.Top), qAbs(testPlace.Bottom - newPlace.Bottom));
  if (dy >= 2 * moveThreshold.Y) {
    return 0;
  } else if (dy > moveThreshold.Y) {
    quality -= (dy - moveThreshold.Y) * 35 / moveThreshold.Y;
  }
  int dsz = qAbs(testPlace.Width() - newPlace.Width()) + qAbs(testPlace.Height() - newPlace.Height());
  if (dsz > sizeThreshold) {
    quality -= dsz * 30 / sizeThreshold;
  }
  return quality;
}

int MotionTracking::ObjectCalcFit(ObjPre &objectPre, Rectangle& place, int distance)
{
  if ((objectPre.Place.Right <= place.Left - distance)
      || (objectPre.Place.Left >= place.Right + distance)
      || (objectPre.Place.Bottom <= place.Top - distance)
      || (objectPre.Place.Top >= place.Bottom + distance)) {
    return -100;
  }
  int horz = 0;
  if ((objectPre.Place.Right >= place.Left)
      && (objectPre.Place.Left <= place.Right)) {
    horz = 50;
  } else if (objectPre.Place.Right < place.Left) {
    horz = (distance - (place.Left - objectPre.Place.Right)) * 100 / distance - 50;
  }
  int vert = 0;
  if ((objectPre.Place.Bottom >= place.Top)
      && (objectPre.Place.Top <= place.Bottom)) {
    vert = 50;
  } else if (objectPre.Place.Bottom < place.Top) {
    vert = (distance - (place.Top - objectPre.Place.Bottom)) * 100 / distance - 50;
  }

  return horz + vert;
}

bool MotionTracking::ObjectModifyOne(ObjInfo& object, Rectangle &newPlace, const Rectangle &fitPlace, const Rectangle &fitCount)
{
  Q_UNUSED(fitPlace);
  Q_UNUSED(fitCount);
  //int deltaQuality = 0;
  //if (fitCount.Left <= -100) {
  //  deltaQuality -= 25;
  //} else if (fitCount.Left > object.Quality - 50) {
  //  deltaQuality += (object.Quality - fitCount.Left) / 4;
  //}
  //if (fitCount.Right <= -100) {
  //  deltaQuality -= 25;
  //} else if (fitCount.Right > object.Quality - 50) {
  //  deltaQuality += (object.Quality - fitCount.Right) / 4;
  //}
  //if (fitCount.Top <= -100) {
  //  deltaQuality -= 25;
  //} else if (fitCount.Top > object.Quality - 50) {
  //  deltaQuality += (object.Quality - fitCount.Top) / 4;
  //}
  //if (fitCount.Bottom <= -100) {
  //  deltaQuality -= 25;
  //} else if (fitCount.Bottom > object.Quality - 50) {
  //  deltaQuality += (object.Quality - fitCount.Bottom) / 4;
  //}

  //if (deltaQuality < object.Quality) {
  //  int d = qMin(object.Quality - deltaQuality, kQualityDown * mFrameMs / 1000);
  //  object.Quality -= d;
  //  if (object.Quality <= -100) {
  //    return false;
  //  }
  //} else {
  //  int d = qMin(deltaQuality - object.Quality, kQualityUp * mFrameMs / 1000);
  //  object.Quality += d;
  //}

  newPlace.Left = (newPlace.Left + fitPlace.Left) / 2;
  newPlace.Right = (newPlace.Right + fitPlace.Right) / 2;
  newPlace.Top = (newPlace.Top + fitPlace.Top) / 2;
  newPlace.Bottom = (newPlace.Bottom + fitPlace.Bottom) / 2;

  Point speed;
  speed.X = ((newPlace.Left + newPlace.Right) / 2 - (object.Place.Left + object.Place.Right) / 2) * 100 / mFrameMs;
  speed.Y = ((newPlace.Top + newPlace.Bottom) / 2 - (object.Place.Top + object.Place.Bottom) / 2) * 100 / mFrameMs;

  object.Speed.X = (2000 * object.Speed.X + speed.X * mFrameMs) / (2000 + mFrameMs);
  object.Speed.Y = (2000 * object.Speed.Y + speed.Y * mFrameMs) / (2000 + mFrameMs);

  object.Place = newPlace;

  return true;
}

void MotionTracking::GetDbgBackground(byte *data)
{
  if (mMacroObjects) {
    return;
  }
  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    const byte* bgrnd = mBackground.data() + j * mSafeStride;
    for (int i = 0; i < mSafeStride; i++) {
      *dbg = *bgrnd;
      dbg++;
      bgrnd++;
    }
  }
}

void MotionTracking::GetDbgDiffLayer0(byte *data)
{
  if (!mMacroObjects) {
    return;
  }
  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    const byte* lay0 = mDiffLayer0.data() + j * mSafeStride;
    for (int i = 0; i < mSafeStride; i++) {
      *dbg = *lay0;
      dbg++;
      lay0++;
    }
  }
}

void MotionTracking::GetDbgDiffLayerX(byte *data)
{
  if (!mMacroObjects) {
    return;
  }
  for (int j = 0; j < mSafeHeight; j++) {
    byte* dbg = data + j * getStride();
    StatDif* stat = &mSceneDiffStat[(j / kBlockSize) * mSceneWidth];

    for (int ii = 0; ii < mSceneWidth; ii++) {
      for (int i = 0; i < kBlockSize; i++) {
        if (stat->Layer == 0) {
          *dbg = 1;
        } else if (stat->Layer == 1) {
          *dbg = 2;
        } else {
          *dbg = 4;
        }

        dbg++;
      }
      stat++;
    }
  }
}

void MotionTracking::GetDbgDiff(byte *data)
{
  if (mMacroObjects) {
    return;
  }
  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    const byte* img = mImageData + j * getStride();
    const byte* bgrnd = mBackground.data() + j * mSafeStride;
    for (int i = 0; i < mSafeStride; i++) {
      const byte& bi = *img;
      const byte& bb = *bgrnd;
      byte d;
      if (bi > bb) {
        d = bi - bb;
        d = (d > 50)? 127: d * 5 / 2;
      } else {
        d = bb - bi;
        d = (d > 50)? 127: d * 5 / 2;
        d |= 0x80;
      }

      *dbg = d;
      dbg++;
      img++;
      bgrnd++;
    }
  }
}

void MotionTracking::GetDbgDiffLayer0Diff(byte *data)
{
  if (!mMacroObjects) {
    return;
  }

  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    const byte* lay2 = mDiffLayer2.data() + j * mSafeStride;
    const byte* lay0 = mDiffLayer0.data() + j * mSafeStride;
    for (int i = 0; i < mSafeStride; i++) {
      const byte& bi = *lay2;
      const byte& bb = *lay0;
      byte d;
      if (bi > bb) {
        d = bi - bb;
        d = (d > 50)? 127: d * 5 / 2;
      } else {
        d = bb - bi;
        d = (d > 50)? 127: d * 5 / 2;
        d |= 0x80;
      }

      *dbg = d;
      dbg++;
      lay2++;
      lay0++;
    }
  }
}

void MotionTracking::GetDbgDiffFormula(byte *data)
{
  if (!mMacroObjects) {
    return;
  }

  for (int j = 1; j < mSafeHeight; j++) {
    byte* dbg = data + j * getStride();
    const byte* img = mImageData + j * getStride();
    byte bip = *img;
    const byte* imgb = img - getStride();
    const byte* lay0 = mDiffLayer0.data() + j * mSafeStride;
    byte* lay2 = mDiffLayer2.data() + j * mSafeStride;

    for (int ii = 0; ii < mSceneWidth; ii++) {
      for (int i = 0; i < kBlockSize; i++) {
        byte bi = *img;
        byte bib = *imgb;
        byte bb = *lay0;

        byte bfh = (bip > bi)? bip - bi: bi - bip;
        byte bfv = (bib > bi)? bib - bi: bi - bib;
        byte bf = qMax(bfh, bfv);
        *lay2 = bf;

        int value = DiffFormula(bf, bb);
        *dbg = value;

        bip = bi;
        img++;
        imgb++;
        lay0++;
        lay2++;

        dbg++;
      }
    }
  }
}

struct MassCell {
  int I;
  int J;
  int Value;

  static bool MassCellGreate(const MassCell* a, const MassCell* b)
  {
    return a->Value > b->Value;
  }
};

void MotionTracking::GetDbgDiffP(byte *data)
{
  if (!mMacroObjects) {
    return;
  }
  memset(data, 0, getHeight() * getStride());

  QVector<MassCell> allCell(mSceneHeight * mSceneWidth);
  QList<MassCell*> sortedCell;
  sortedCell.reserve(mSceneHeight * mSceneWidth);
  for (int jj = 0; jj < mSceneHeight; jj++) {
    InfoDif* scene = &mSceneDiffInfo[jj * mSceneWidth];
    MassCell* curCell = &allCell[jj * mSceneWidth];

    for (int ii = 0; ii < mSceneWidth; ii++) {
      if (scene->DiffPCount) {
        curCell->I = ii;
        curCell->J = jj;
        curCell->Value = scene->DiffPCount;
        sortedCell.append(curCell);
      }

      scene++;
      curCell++;
    }
  }

  const int kRound2 = 3;
  const int kRoundMap[] = { -500, -500, -500, -10, -7, -4, -2, 0, 0, 0 };
  while (sortedCell.size() > 0) {
    qSort(sortedCell.begin(), sortedCell.end(), MassCell::MassCellGreate);
    const MassCell& cell = *sortedCell.first();
    if (cell.Value <= 8) {
      break;
    }
    for (int j = 0; j < kBlockSize; j++) {
      for (int i = 0; i < kBlockSize; i++) {
        data[(cell.J*kBlockSize + j) * getStride() + cell.I*kBlockSize + i] = 4;
      }
    }
    allCell[cell.J * mSceneWidth + cell.I].Value = 0;
    for (int j = qMax(0, cell.J - kRound2); j <= qMin(mSceneHeight - 1, cell.J + kRound2); j++) {
      for (int i = qMax(0, cell.I - kRound2); i <= qMin(mSceneWidth - 1, cell.I + kRound2); i++) {
        int d = qAbs(cell.J - j) + qAbs(cell.I - i);
        d = kRoundMap[d];
        allCell[j * mSceneWidth + i].Value = qMax(0, allCell[j * mSceneWidth + i].Value + d);
      }
    }
  }

//  for (int j = 0; j < mSafeHeight; j++) {
//    byte* dbg = data + j * getStride();
//    InfoDif* scene = &mSceneDiffInfo[j / kBlockSize * mSceneWidth];

//    for (int ii = 0; ii < mSceneWidth; ii++) {
//      int value;
//      if (scene->DiffPCount > 8) {
//        value = 4;
//      } else {
//        value = scene->DiffPCount/2;
//      }

//      for (int i = 0; i < kBlockSize; i++) {
//        *dbg++ = value;
//      }
//      scene++;
//    }
//  }
}

void MotionTracking::GetDbgDiffMicroHyst(byte *data)
{
  int* datai = reinterpret_cast<int*>(data);
  Hyst hyst;
  for (int j = 0; j < getHeight(); j++) {
    const byte* img = mImageData + j * getStride();
    const byte* bgrnd = mBackground.data() + j * mSafeStride;
    for (int i = 0; i < mSafeStride; i++) {
      const byte& bi = *img;
      const byte& bb = *bgrnd;
      int diff = qAbs(bi - bb);
      hyst.Inc(diff);

      img++;
      bgrnd++;
    }
  }

  int totalCount = hyst.TotalCount();
  datai[0] = totalCount;
  memcpy(datai + 1, hyst.GetVector().constData(), sizeof(int) * hyst.GetVector().size());
  datai[mDiffStatScene.Threshold()] = totalCount / 2;
}

//void MotionTracking::GetDbgDiffHyst(byte *data)
//{
//  static std::vector<int> gHyst;
//  gHyst.resize(0);
//  gHyst.resize(256, 0);

//  int* datai = reinterpret_cast<int*>(data);
//  memset(datai, 0, 256 * sizeof(int));
//  for (int j = 0; j < getHeight(); j++) {
//    const byte* img = mImageData + j * getStride();
//    const byte* bgrnd = mBackground.data() + j * mSafeStride;
//    for (int i = 0; i < mSafeStride; i++) {
//      const byte& bi = *img;
//      const byte& bb = *bgrnd;
//      int diff = qAbs(bi - bb) + 1;

//      datai[diff]++;
//      img++;
//      bgrnd++;
//    }
//  }
////  for (int j = 0; j < kBlockSize; j++) {
////    const byte* img = mImageData + (kBlockJ * kBlockSize + j) * getStride() + kBlockI * kBlockSize;
////    const byte* bgrnd = mBackground.data() + (kBlockJ * kBlockSize + j) * mSafeStride + kBlockI * kBlockSize;
////    for (int i = 0; i < kBlockSize; i++) {
////      const byte& bi = *img;
////      const byte& bb = *bgrnd;
////      int diff = qAbs(bi - bb);

////      gHyst[diff]++;
////      img++;
////      bgrnd++;
////    }
////  }

//  int totalCount = getHeight() * mSafeStride;
////  int totalCount = 0;
////  for (int i = 0; i < 256; i++) {
////    datai[1 + i] = gHyst[i];
////    totalCount += gHyst[i];
////  }
//  datai[0] = totalCount;

//  int threshCount = 0;
//  int thresh0 = 25*totalCount/100;
//  int thresh1 = 50*totalCount/100;
//  int thresh2 = 75*totalCount/100;
//  int thresh0Ind = 0;
//  int thresh1Ind = 0;
//  int thresh2Ind = 0;
//  for (int i = 1; i < 256; i++) {
//    threshCount += datai[i];
//    if (!thresh0Ind) {
//      if (threshCount > thresh0) {
//        thresh0Ind = i - 1;
//      }
//    }
//    if (!thresh1Ind) {
//      if (threshCount > thresh1) {
//        thresh1Ind = i - 1;
//      }
//    }
//    if (!thresh2Ind) {
//      if (threshCount > thresh2) {
//        thresh2Ind = i - 1;
//      }
//    }
//  }

//  datai[mDiffStatScene.Threshold()] = totalCount;
////  datai[thresh0Ind] = totalCount;
////  datai[thresh1Ind] = totalCount;
////  datai[thresh2Ind] = totalCount;
//}

void MotionTracking::GetDbgDiffBackground(byte *data)
{
  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    const byte* img = mImageData + j * getStride();
    const byte* bgrnd = mBackground.data() + j * mSafeStride;
    for (int i = 0; i < mSafeStride; i++) {
      const byte& bi = *img;
      const byte& bb = *bgrnd;
      int d = qAbs(bi - bb);

      if (d < mDiffStatScene.Threshold()) {
        *dbg = bb;
      } else {
        *dbg = 0;
      }

      dbg++;
      img++;
      bgrnd++;
    }
  }
}

void MotionTracking::GetDbgBlockDiffHyst(byte *data)
{
  int* datai = reinterpret_cast<int*>(data);
  memset(datai, 0, 256 * sizeof(int));
  for (int jj = 0; jj < mSceneHeight; jj++) {
    Info* scene = &mSceneInfo[jj * mSceneWidth];
    Stat* stat = &mSceneStat[jj * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value = scene->DiffCount * 100 / (kBlockSize * kBlockSize);
      int statValue = (mFrameMs > 0)? stat->DiffCount.MidValue(100): 0;
      value = 100 + (value - statValue);

      datai[value + 1]++;
      scene++;
    }
  }

//  for (int i = 0; i < kBlockSize * kBlockSize / 4; i++) {
//    datai[1 + i*4] += datai[1 + i*4 + 1];
//    datai[1 + i*4] += datai[1 + i*4 + 2];
//    datai[1 + i*4] += datai[1 + i*4 + 3];
//    datai[1 + i*4 + 1] = datai[1 + i*4 + 2] = datai[1 + i*4 + 3] = datai[1 + i*4];
//  }

  int totalCount = mSceneHeight * mSceneWidth;
  int threshCount = 0;
  int thresh0 = 25*totalCount/100;
  int thresh0Ind = 0;
  int thresh1 = 50*totalCount/100;
  int thresh1Ind = 0;
  int thresh2 = 75*totalCount/100;
  int thresh2Ind = 0;
  for (int i = 1; i < 256; i++) {
    threshCount += datai[i];
    if (!thresh0Ind) {
      if (threshCount > thresh0) {
        thresh0Ind = i - 1;
      }
    }
    if (!thresh1Ind) {
      if (threshCount > thresh1) {
        thresh1Ind = i - 1;
      }
    }
    if (!thresh2Ind) {
      if (threshCount > thresh2) {
        thresh2Ind = i - 1;
      }
    }
  }

//  datai[thresh0Ind] = totalCount;
//  datai[thresh1Ind] = totalCount;
//  datai[thresh2Ind] = totalCount;
  datai[0] = totalCount;
  datai[101] = totalCount;
  datai[201] = totalCount;
}

void MotionTracking::GetDbgBlockDiffOneHyst(byte *data, int blockI, int blockJ)
{
  const Stat& stat = mSceneStat[blockJ * mSceneWidth + blockI];
  int total = stat.DiffCount.TotalCount();;

  int* datai = reinterpret_cast<int*>(data);
  memset(datai, 0, 256 * sizeof(int));
  for (int i = 0; i < 100; i++) {
    datai[1 + i] = stat.DiffCount.Count(i, 99);
  }

  datai[0] = total;
  datai[1 + 100] = total;
}

void MotionTracking::GetDbgBlockDiff(byte *data)
{
  if (mMacroObjects) {
    return;
  }
  memset(data + mSafeHeight * getStride(), 0, (getHeight() - mSafeHeight) * getStride());
  for (int j = 0; j < mSafeHeight; j++) {
    byte* dbg = data + j * getStride();
    Info* scene = &mSceneInfo[j / kBlockSize * mSceneWidth];
    Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value = stat->DiffCount.Normalized(scene->DiffCount, kBlockSize * kBlockSize);
      byte val = (byte)value;

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      scene++;
      stat++;
    }
  }
}

void MotionTracking::GetDbgBlockDiffMidStat(byte *data)
{
  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value = stat->DiffCount.MidValue(100);
      byte val;

      if (value > 90) {
        val = 4;
      } else if (value > 60) {
        val = 3;
      } else if (value > 30) {
        val = 2;
      } else if (value > 15) {
        val = 1;
      } else {
        val = 0;
      }

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      stat++;
    }
  }
}

void MotionTracking::GetDbgBlockDiffMaxStat(byte *data)
{
  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value = stat->DiffCount.MaxValue(100);
      byte val;

      if (value > 90) {
        val = 4;
      } else if (value > 60) {
        val = 3;
      } else if (value > 30) {
        val = 2;
      } else if (value > 15) {
        val = 1;
      } else {
        val = 0;
      }

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      stat++;
    }
  }
}

void MotionTracking::GetDbgSharpHyst(byte *data)
{
  int* datai = reinterpret_cast<int*>(data);
  memset(datai, 0, 256 * sizeof(int));
  for (int j = 0; j < getHeight(); j++) {
    const byte* img = mImageData + j * getStride();
    const byte* imgl = img;
    for (int i = 0; i < getWidth(); i++) {
      const byte& bi = *img;
      const byte& bl = *imgl;
      int diff = qAbs(bl - bi);

      datai[diff + 1]++;
      imgl = img;
      img++;
    }
  }

  int totalCount = getHeight() * getWidth();

  int threshCount = 0;
  int thresh0 = 25*totalCount/100;
  int thresh1 = 50*totalCount/100;
  int thresh2 = 75*totalCount/100;
  int thresh0Ind = 0;
  int thresh1Ind = 0;
  int thresh2Ind = 0;
  for (int i = 1; i < 256; i++) {
    threshCount += datai[i];
    if (!thresh0Ind) {
      if (threshCount > thresh0) {
        thresh0Ind = i - 1;
      }
    }
    if (!thresh1Ind) {
      if (threshCount > thresh1) {
        thresh1Ind = i - 1;
      }
    }
    if (!thresh2Ind) {
      if (threshCount > thresh2) {
        thresh2Ind = i - 1;
      }
    }
  }

  datai[0] = totalCount/2;
  datai[thresh0Ind] = totalCount;
  datai[thresh1Ind] = totalCount;
  datai[thresh2Ind] = totalCount;

  //Log.Debug(QString("1: %1, 2: %2, 3: %3").arg(thresh0Ind).arg(thresh1Ind).arg(thresh2Ind));
}

void MotionTracking::GetDbgBlockSharp(byte *data)
{
  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    Info* scene = &mSceneInfo[j / kBlockSize * mSceneWidth];
    Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value = stat->DiffCount.Normalized(scene->DiffCount, kBlockSize * kBlockSize);
      byte val = (byte)value;

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      scene++;
      stat++;
    }
  }
}

void MotionTracking::GetDbgBlockDiffSharp(byte *data)
{
  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    Info* scene = &mSceneInfo[j / kBlockSize * mSceneWidth];
    Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value = stat->DiffSharpCount.Normalized(scene->DiffSharpCount, kBlockSize * kBlockSize);
      byte val = (byte)value;

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      scene++;
      stat++;
    }
  }
}

void MotionTracking::GetDbgBlockDiffSharpMidStat(byte *data)
{
  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value = stat->DiffSharpCount.MidValue(100);
      byte val;

      if (value > 90) {
        val = 4;
      } else if (value > 60) {
        val = 3;
      } else if (value > 30) {
        val = 2;
      } else if (value > 15) {
        val = 1;
      } else {
        val = 0;
      }

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      stat++;
    }
  }
}

void MotionTracking::GetDbgBlockDiffSharpMaxStat(byte *data)
{
  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value = stat->DiffSharpCount.MaxValue(100);
      byte val;

      if (value > 90) {
        val = 4;
      } else if (value > 60) {
        val = 3;
      } else if (value > 30) {
        val = 2;
      } else if (value > 15) {
        val = 1;
      } else {
        val = 0;
      }

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      stat++;
    }
  }
}

void MotionTracking::GetDbgInfoObjects(byte *data)
{
  memset(data + mSafeHeight * getStride(), 0, (getHeight() - mSafeHeight) * getStride());
  for (int j = 0; j < mSafeHeight; j++) {
    byte* dbg = data + j * getStride();
    int* tmp2 = &mSceneTmp2[j / kBlockSize * mSceneWidth];
    //Info* scene = &mSceneInfo[j / kBlockSize * mSceneWidth];
    //Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
//      int diff = stat->DiffCount.Normalized(scene->DiffCount, kBlockSize * kBlockSize);
//      int sharp = stat->DiffSharpCount.Normalized(scene->DiffSharpCount, kBlockSize * kBlockSize);

//      byte val;
//      if (diff && sharp) {
//        val = 4;
//      } else if (diff > 1) {
//        val = 3;
//      } else if (sharp > 1) {
//        val = 2;
//      } else if (diff || sharp) {
//        val = 1;
//      } else {
//        val = 0;
//      }

      int value = *tmp2;
      byte val = qMin((byte)4, (byte)value);
//      byte val;
//      if (value > 10) {
//        val = 4;
//      } else if (value == 10) {
//        val = 3;
//      } else if (value > 2) {
//        val = 2;
//      } else if (value) {
//        val = 1;
//      } else {
//        val = 0;
//      }

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      tmp2++;
    }
  }
}

void MotionTracking::GetDbgInfoObjectsHyst(byte *data)
{
  //static QList<int> countHistory;
  //
  //int* datai = reinterpret_cast<int*>(data);
  //memset(datai, 0, 256 * sizeof(int));

  //int count = 0;
  //for (int jj = 0; jj < mSceneHeight; jj++) {
  //  Info* scene = &mSceneInfo[jj * mSceneWidth];
  //  for (int ii = 0; ii < mSceneWidth; ii++) {
  //    count += scene->Object;

  //    scene++;
  //  }
  //}

  //int ind = 1 + count / 5;
  //countHistory.append(ind);
  //if (countHistory.size() > 255) {
  //  countHistory.removeFirst();
  //}

  //int maxCount = 10;
  //for (auto itr = countHistory.begin(); itr != countHistory.end(); itr++) {
  //  maxCount = qMax(maxCount, *itr);
  //}

  //ind = 1;
  //for (auto itr = countHistory.begin(); itr != countHistory.end(); itr++) {
  //  datai[ind] = *itr;
  //  ind++;
  //}
  //datai[0] = maxCount;
  int* datai = reinterpret_cast<int*>(data);
  memset(datai, 0, 256 * sizeof(int));
  for (int i = 0; i < 100; i++) {
    datai[1 + i] = mPreCountStat.Count(i, 99);
  }

  datai[0] = mPreCountStat.TotalCount();
  datai[1 + 100] = mPreCountStat.TotalCount();
}

void MotionTracking::GetDbgDiffInfoObjects(byte *data)
{
  if (!mMacroObjects) {
    return;
  }
  memset(data + mSafeHeight * getStride(), 0, (getHeight() - mSafeHeight) * getStride());
  for (int j = 0; j < mSafeHeight; j++) {
    byte* dbg = data + j * getStride();
    int* object = &mSceneTmp[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      byte val;
      if (*object > 4) {
        val = 4;
      } else {
        val = (byte)*object;
      }

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      object++;
    }
  }
}

void MotionTracking::GetDbgInfoSharp(byte* data)
{
  memset(data + mSafeHeight * getStride(), 0, (getHeight() - mSafeHeight) * getStride());
  for (int j = 0; j < mSafeHeight; j++) {
    byte* dbg = data + j * getStride();
    Info* scene = &mSceneInfo[j / kBlockSize * mSceneWidth];
    Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value = 0;
      int threshold_ = stat->SharpD + kThresholdBlockSharp;
      byte val;

      if (value > threshold_ * 2) {
        val = 4;
      } else if (value > threshold_) {
        val = 3;
      } else if (value > threshold_ / 2) {
        val = 2;
      } else if (value > threshold_ / 4) {
        val = 1;
      } else {
        val = 0;
      }

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      scene++;
      stat++;
    }
  }
}

void MotionTracking::GetDbgStatSharp(byte *data)
{
  memset(data + mSafeHeight * getStride(), 0, (getHeight() - mSafeHeight) * getStride());
  for (int j = 0; j < mSafeHeight; j++) {
    byte* dbg = data + j * getStride();
    Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value = stat->SharpD;
      int threshold_ = kThresholdBlockSharp * 2;
      byte val;

      if (value > threshold_ * 2) {
        val = 4;
      } else if (value > threshold_) {
        val = 3;
      } else if (value > threshold_ / 2) {
        val = 2;
      } else if (value > threshold_ / 4) {
        val = 1;
      } else {
        val = 0;
      }

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      stat++;
    }
  }
}

void MotionTracking::GetDbgInfo(byte* data, size_t disp, size_t dispStat, int threshold)
{
  Q_UNUSED(dispStat);
  memset(data + mSafeHeight * getStride(), 0, (getHeight() - mSafeHeight) * getStride());
  for (int j = 0; j < mSafeHeight; j++) {
    byte* dbg = data + j * getStride();
    Info* scene = &mSceneInfo[j / kBlockSize * mSceneWidth];
    Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value = *(int*)(scene + disp);
      //int valueStat = *(int*)(stat + dispStat);
      int threshold_ = threshold;// + *(int*)(stat + dispStat);
      byte val;

      if (value > threshold_ * 2) {
        val = 4;
      } else if (value > threshold_) {
        val = 3;
      } else if (value > threshold_ / 2) {
        val = 2;
      } else if (value > threshold_ / 4) {
        val = 1;
      } else {
        val = 0;
      }

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      scene++;
      stat++;
    }
  }
}

void MotionTracking::GetDbgStat(byte *data, size_t disp, int threshold)
{
  memset(data + mSafeHeight * getStride(), 0, (getHeight() - mSafeHeight) * getStride());
  for (int j = 0; j < mSafeHeight; j++) {
    byte* dbg = data + j * getStride();
    Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value = *(int*)(stat + disp);
      byte val;

      if (value > threshold * 2) {
        val = 4;
      } else if (value > threshold) {
        val = 3;
      } else if (value > threshold / 2) {
        val = 2;
      } else if (value > threshold / 4) {
        val = 1;
      } else {
        val = 0;
      }

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      stat++;
    }
  }
}

//void MotionTracking::GetDbgDiffM(byte *data)
//{
//  for (int j = 0; j < getHeight(); j++) {
//    byte* dbg = data + j * getStride();
//    const byte* img = mImageData + j * getStride();
//    const byte* bgrnd = mBackground.data() + j * mSafeStride;
//    for (int i = 0; i < mSafeStride; i++) {
//      byte d = (*img > *bgrnd)? 0: *bgrnd - *img;
//      d = (d >= 50)? 255: d * 5;
//      *dbg = d;
//      dbg++;
//      img++;
//      bgrnd++;
//    }
//  }
//}

void MotionTracking::GetDbgSharp(byte *data)
{
  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    const byte* img = mImageData + j * getStride();
    for (int i = 0; i < mSafeStride - 1; i++) {
      const byte& bi = *img;
      const byte& bi2 = *(img + 1);
      byte s = (bi >= bi2)? bi - bi2: bi2 - bi;
      *dbg = (s > 50)? 255: s * 5;
      img++;
      dbg++;
    }
  }
}

void MotionTracking::GetDbgSharpDiff(byte *data)
{
  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    const byte* img = mImageData + j * getStride();
    const byte* bgrnd = mBackground.data() + j * mSafeStride;
    for (int i = 0; i < mSafeStride - 1; i++) {
      const byte& bi = *img;
      const byte& bi2 = *(img + 1);
      const byte& bb = *bgrnd;
      const byte& bb2 = *(bgrnd + 1);
      byte si = (bi >= bi2)? bi - bi2: bi2 - bi;
      byte sb = (bb >= bb2)? bb - bb2: bb2 - bb;
      byte s = (si >= sb)? si - sb: sb - si;
      *dbg = (s > 50)? 255: s * 5;
      img++;
      bgrnd++;
      dbg++;
    }
  }
}

void MotionTracking::GetDbgFront(byte *data)
{
  memset(data + mSafeHeight * getStride(), 0, (getHeight() - mSafeHeight) * getStride());
  for (int j = 0; j < mSafeHeight; j++) {
    byte* dbg = data + j * getStride();
    Info* scene = &mSceneInfo[j / kBlockSize * mSceneWidth];
    Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      int value = stat->DiffCount.Normalized(scene->DiffCount, kBlockSize * kBlockSize);
      byte val;

      if (value >= 2) {
        val = 4;
      } else if (value == 1) {
        val = 2;
      } else {
        val = 0;
      }

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      scene++;
      stat++;
    }
  }
}

void MotionTracking::GetDbgDiffLayer1(byte *data)
{
  if (!mMacroObjects) {
    return;
  }

  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    const byte* lay1 = mDiffLayer1.data() + j * mSafeStride;

    for (int i = 0; i < mSafeStride; i++) {
      *dbg = *lay1;

      lay1++;
      dbg++;
    }
  }
}

void MotionTracking::GetDbgDiffLayer2(byte *data)
{
  if (!mMacroObjects) {
    return;
  }

  for (int j = 0; j < getHeight(); j++) {
    byte* dbg = data + j * getStride();
    const byte* lay2 = mDiffLayer2.data() + j * mSafeStride;

    for (int i = 0; i < mSafeStride; i++) {
      *dbg = *lay2;

      lay2++;
      dbg++;
    }
  }
}

void MotionTracking::GetDbgDiffCurrentLayer(byte *data)
{
  if (!mMacroObjects) {
    return;
  }

  memset(data + mSafeHeight * getStride(), 0, (getHeight() - mSafeHeight) * getStride());
  for (int j = 0; j < mSafeHeight; j++) {
    byte* dbg = data + j * getStride();
    StatDif* stat = &mSceneDiffStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      byte val = (byte)stat->Layer;

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      stat++;
    }
  }
}

void MotionTracking::GetDbgDiffLayer0Time(byte *data)
{
  if (!mMacroObjects) {
    return;
  }

  memset(data + mSafeHeight * getStride(), 0, (getHeight() - mSafeHeight) * getStride());
  for (int j = 0; j < mSafeHeight; j++) {
    byte* dbg = data + j * getStride();
    StatDif* stat = &mSceneDiffStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      byte val = (stat->Layer0TimeTrue > kLayerSwitchTime)? (byte)255: (byte)(200 * stat->Layer0TimeTrue / kLayerSwitchTime);

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      stat++;
    }
  }
}

void MotionTracking::GetDbgDiffLayer1Time(byte *data)
{
  if (!mMacroObjects) {
    return;
  }

  memset(data + mSafeHeight * getStride(), 0, (getHeight() - mSafeHeight) * getStride());
  for (int j = 0; j < mSafeHeight; j++) {
    byte* dbg = data + j * getStride();
    StatDif* stat = &mSceneDiffStat[j / kBlockSize * mSceneWidth];
    for (int ii = 0; ii < mSceneWidth; ii++) {
      byte val = (stat->Layer1TimeTrue > kLayerSwitchTime)? (byte)255: (byte)(200 * stat->Layer1TimeTrue / kLayerSwitchTime);

      for (int i = 0; i < kBlockSize; i++) {
        *dbg++ = val;
      }
      stat++;
    }
  }
}

void MotionTracking::GetDbgObjDetect(byte *data)
{
  if (!mMacroObjects) {
    return;
  }

  memset(data, 0, getHeight() * getStride());
  for (int j = 0; j < mSafeHeight; j++) {
    byte* dbg = data + j * getStride();
    const byte* layB = mBorderLayer.data() + j * (mSafeStride + 2) + 1;
    InfoDif* scene = &mSceneDiffInfo[j / kBlockSize * mSceneWidth];

    for (int ii = 0; ii < mSceneWidth; ii++) {
      if (scene->Object > 1) {
        for (int i = 0; i < kBlockSize; i++) {
          *dbg = *layB > 0? 2: 0;

          layB++;
          dbg++;
        }
      } else {
        layB += kBlockSize;
        dbg += kBlockSize;
      }
      scene++;
    }
  }
}

void MotionTracking::GetDbgIgnore(byte* data)
{
  memset(data + mSafeHeight * getStride(), 0, (getHeight() - mSafeHeight) * getStride());
  if (!mMacroObjects) {
    for (int j = 0; j < mSafeHeight; j++) {
      byte* dbg = data + j * getStride();
      Stat* stat = &mSceneStat[j / kBlockSize * mSceneWidth];
      for (int ii = 0; ii < mSceneWidth; ii++) {
        byte val;
        if ((stat->Flag & StatDif::eIgnore) == 0) {
          val = 1;
        } else {
          val = 0;
        }

        for (int i = 0; i < kBlockSize; i++) {
          *dbg++ = val;
        }
        stat++;
      }
    }
  } else {
    for (int j = 0; j < mSafeHeight; j++) {
      byte* dbg = data + j * getStride();
      StatDif* stat = &mSceneDiffStat[j / kBlockSize * mSceneWidth];
      for (int ii = 0; ii < mSceneWidth; ii++) {
        byte val;
        if ((stat->Flag & StatDif::eIgnore) == 0) {
          val = 1;
        } else {
          val = 0;
        }

        for (int i = 0; i < kBlockSize; i++) {
          *dbg++ = val;
        }
        stat++;
      }
    }
  }
}

void MotionTracking::GetDbgHyst(byte* data, const Hyst& hyst, int percentDraw)
{
  int* datai = reinterpret_cast<int*>(data);

  int totalCount = hyst.TotalCount();
  datai[0] = totalCount;
  memcpy(datai + 1, hyst.GetVector().constData(), sizeof(int) * hyst.GetVector().size());
  if (percentDraw >= 0) {
    datai[percentDraw + 1] = totalCount / 2;
  }
}

bool MotionTracking::IsPointInDoor(int ii, int jj)
{
  for (auto itr = mDoors.begin(); itr != mDoors.end(); itr++) {
    const PointList& points = *itr;
    int pos = 0;
    QPointF pointl = PointPercentToScreen(points.last());
    for (auto itr = points.begin(); itr != points.end(); itr++) {
      QPointF point = PointPercentToScreen(*itr);
      float x1 = point.x() - pointl.x();
      float y1 = point.y() - pointl.y();
      float x2 = (0.5 + ii) - pointl.x();
      float y2 = (0.5 + jj) - pointl.y();
      float res = x1*y2 - x2*y1;
      if (qAbs(res) < 0.001f) {
        pos = 0;
        break;
      }
      if (pos == 0) {
        pos = (res > 0)? 1: -1;
      } else if ((pos > 0) != (res > 0)) {
        pos = 0;
        break;
      }
      pointl = point;
    }
    if (pos != 0) {
      return true;
    }
  }
  return false;
}

QPointF MotionTracking::PointPercentToScreen(const QPointF &pointPercent)
{
  return QPointF(pointPercent.x() * mSceneWidth * 0.01f, pointPercent.y() * mSceneHeight * 0.01f);
}

QPointF MotionTracking::PointScreenToPercent(const QPointF &pointScreen)
{
  return QPointF(pointScreen.x() * 100.0f / mSceneWidth, pointScreen.y() * 100.0f / mSceneHeight);
}

bool MotionTracking::IsNearBorder(const Obj2Info& object)
{
  int x = (int)object.Location.x();
  int y = (int)object.Location.y();
  int x1 = (int)(object.Location.x() - 1.5f*object.Radius.x());
  int x2 = (int)(object.Location.x() + 1.5f*object.Radius.x());
  int y1 = (int)(object.Location.y() - 1.5f*object.Radius.y());
  int y2 = (int)(object.Location.y() + 1.5f*object.Radius.y());
  if (x1 < 0 || x2 >= mSceneWidth || y1 < 0 || y2 >= mSceneHeight) {
    return true;
  } else if ((mSceneDiffStat[y * mSceneWidth + x1].Flag & StatDif::eDoor) != 0) {
    return true;
  } else if ((mSceneDiffStat[y * mSceneWidth + x2].Flag & StatDif::eDoor) != 0) {
    return true;
  } else if ((mSceneDiffStat[y1 * mSceneWidth + x].Flag & StatDif::eDoor) != 0) {
    return true;
  } else if ((mSceneDiffStat[y2 * mSceneWidth + x].Flag & StatDif::eDoor) != 0) {
    return true;
  } else if ((mSceneDiffStat[y1 * mSceneWidth + x1].Flag & StatDif::eDoor) != 0) {
    return true;
  } else if ((mSceneDiffStat[y1 * mSceneWidth + x2].Flag & StatDif::eDoor) != 0) {
    return true;
  } else if ((mSceneDiffStat[y2 * mSceneWidth + x1].Flag & StatDif::eDoor) != 0) {
    return true;
  } else if ((mSceneDiffStat[y2 * mSceneWidth + x2].Flag & StatDif::eDoor) != 0) {
    return true;
  }

  return false;
}


MotionTracking::MotionTracking()
  : mDiffBackgroundDouble(true)
  , mStandByMode(false)
  , mStartTimestamp(0), mCalc1Timestamp(0), mSafeStride(0), mSafeHeight(0)
  //, mLayer0DiffCountThreshold(kLayer0DiffCountThreshold)
  , mDiffStatScene("Diff", kDiffThresholdMin), mDiffSharpStatScene("Diff sharp", kDiffThresholdMin)
  , mObjectMaxSpeed(25.0f), mObjectMaxAccel(100.0f), mObjectFitMin(0.33f)
  , mObjectNextId(1)
  , mTotalObjectsLast(0)
{
}

MotionTracking::~MotionTracking()
{
}

