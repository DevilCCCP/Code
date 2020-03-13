#include <algorithm>
#include <QSet>

#include <Lib/Log/Log.h>
#include <LibV/Include/Region.h>
#include <LibV/Include/Tools.h>

#include "BlockObj.h"
//#include "UinPre.h"


//#define LOG_OBJ

#ifdef LOG_OBJ
#define LogLive(X) Log.Trace(X)
#define CONNECT_DUMP
#else
#define LogLive(X)
#endif

const int kObjTrackPeriod = 500;
const int kObjQualityMax = 2000;
const int kObjQualityStart = 500;
const int kObjQualityGood = 1000;
const int kNormalStatMinimum = 200;
const int kShareObjThreshold = 2;
const int kShotOutdateMs = 30 * 1000;
//const int kDefaultNormalSize = 20;
//const int kDefaultNormalSpeed = 100;
//const int kDefaultNormalAccel = 1000;

const int kMinUinWidth  = 200;


void BlockObj::SetSmoothBlocks(bool _SmoothBlocks)
{
  mSmoothBlocks = _SmoothBlocks;
}

void BlockObj::SetThresholds(int _MarkThreshold)
{
  mMarkThreshold = _MarkThreshold;
}

void BlockObj::SetUseScreenshots(int _UseScreenshots)
{
  mUseScreenshots = _UseScreenshots;
}

void BlockObj::SetInZoneTime(int _InZoneTime)
{
  mInZoneTime = _InZoneTime;
}

void BlockObj::LoadSettings(const SettingsAS& settings)
{
  mHystMoment.Deserialize(settings->GetValue("ObjMomentHyst", QString("")).toString(), 128);

  mObjEnergy.InitSource();
//  DeserializeBlock(settings, "ObjectEnergy", mObjEnergy, mEnergyMs);
}

void BlockObj::SaveSettings(const SettingsAS& settings)
{
  mHystMoment.Normalize(14*24*60*60*30);
  settings->SetValue("ObjMomentHyst", mHystMoment.Serialize(128));

//  SerializeBlock(settings, "ObjectEnergy", mObjEnergy, mEnergyMs);
}

void BlockObj::Init()
{
  mPreObjMark.InitSource();
  if (mSmoothBlocks) {
    mDiffMarkSmooth.InitSource();
  }
//  if (UsingUins()) {
//  }

  mBarierInfos.clear();
  for (int i = 0; i < Bariers().size(); i++) {
    const AnalyticsB::PointList& points = Bariers().at(i).Points;
    mBarierInfos.reserve(Bariers().size());

    BarierInfo barierInfo;
    foreach (const QPointF& point, points) {
      QPoint p = PointPercentToScene(point);
      barierInfo.Points.append(p);
    }
    mBarierInfos.append(barierInfo);
  }
}

void BlockObj::Analize()
{
  Prepare();
  PreObjCreate();
  ObjManage();
}

void BlockObj::Prepare()
{
  mReturnObjItr = 0;
  qreal norm = ((BlockWidth() + BlockHeight())/4) * 0.01;
  if (mHystMoment.TotalCount() > kNormalStatMinimum) {
    int tooHigh = 950;
    int tooHighValue = mHystMoment.GetValue(tooHigh);
    int tooLowValue = tooHighValue;
    for (int tooLow = 0; tooLow < 950; tooLow += 100) {
      tooLowValue = mHystMoment.GetValue(tooLow);
      if (3 * tooLowValue > tooHighValue) {
        break;
      }
    }
    int normalMoment = (int)(norm * (tooLowValue + tooHighValue) / 3 + 0.5);
    if (normalMoment != mNormalMoment) {
      mNormalMoment = normalMoment;
      Log.Info(QString("Normal moment set to %1 (l: %2, h: %3)").arg(mNormalMoment).arg(tooLowValue).arg(tooHighValue));
    }
  } else {
    mNormalMoment = 0;
  }
}

void BlockObj::PreObjCreate()
{
  PreConnect();
  PreConstruct();
  PreCalc();
  PreFilter();
  PreAdjust();
}

void BlockObj::ObjManage()
{
//  int curMs = CurrentMs();

  ObjPrepare();
  ObjConnectPre();
  ObjConnectAdjust();
  ObjConnectFilterObj();
  ObjConnectFilterPre();
  ObjSharePre();
  ObjDone();
  ObjConfirm();
  ObjMove();
//  ObjCleanup();
  ObjNew();
  ObjDump();
}

//void BlockObj::UinManage()
//{
//  UinDetect();
//}

qreal BlockObj::CalcStable()
{
  return 100.0 * mObjs.size();
}

void BlockObj::ResetEnergy()
{
  mEnergyMs = 0;
  mObjEnergy.InitSource();
}

bool BlockObj::HavePreObj()
{
  return mReturnObjItr < mPreObjs.size();
}

bool BlockObj::RetrievePreObj(Object& object)
{
  if (!HavePreObj()) {
    return false;
  }

  const PreObj& pre = mPreObjs[mReturnObjItr++];
  object.Id = mReturnObjItr;
  object.Color = 100;
  object.Dimention.Left = pre.Place.left();
  object.Dimention.Top = pre.Place.top();
  object.Dimention.Right = pre.Place.right();
  object.Dimention.Bottom = pre.Place.bottom();
  return true;
}

bool BlockObj::HaveObj()
{
  if (!mNormalMoment) {
    return false;
  }

  for (; mReturnObjItr < mObjs.size(); mReturnObjItr++) {
    mCurrentObj = &mObjs[mReturnObjItr];
    if (qMin(mCurrentObj->Place.width(), mCurrentObj->Place.height()) >= mNormalMoment) {
      return true;
    }
  }
  mReturnObjItr = 0;
  return false;
}

bool BlockObj::RetrieveObj(Object& object)
{
  if (!HaveObj()) {
    return false;
  }

  mCurrentObj = &mObjs[mReturnObjItr++];
  object.Id = mCurrentObj->Id;
  object.Color = ObjGetColor();
  object.Dimention.Left   = mCurrentObj->Place.left();
  object.Dimention.Top    = mCurrentObj->Place.top();
  object.Dimention.Right  = mCurrentObj->Place.right();
  object.Dimention.Bottom = mCurrentObj->Place.bottom();
  return true;
}

void BlockObj::CreateMark(const BlockSrc<int>& diffMark)
{
  for (int jj = 0; jj < BlockHeight(); jj++) {
    const int* mark = diffMark.Line(jj);
    const BlockInfo* info = GetBlockInfo().Line(jj);
    int* object = mPreObjMark.Line(jj);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      bool isObject = *mark >= mMarkThreshold && (info->Flag & BlockInfo::eIgnore) == 0;
      *object = (isObject)? 1: 0;

      mark++;
      info++;
      object++;
    }
  }
}

void BlockObj::CreateMarkSmooth()
{
  for (int jj = 0; jj < BlockHeight(); jj++) {
    const int* mark = mDiffMark.Line(jj);
    int* marks = mDiffMarkSmooth.Line(jj);
    memcpy(marks, mark, BlockWidth() * sizeof(int));
  }

  for (int jj = 1; jj < BlockHeight() - 1; jj++) {
    const int* mark = mDiffMark.Line(jj);
    int* marks = mDiffMarkSmooth.Line(jj);
    int* marksp = mDiffMarkSmooth.Line(jj - 1);
    int* marksn = mDiffMarkSmooth.Line(jj + 1);
    for (int ii = 1; ii < BlockWidth() - 1; ii++) {
      int inc = *mark / 2;
      if (inc) {
        *(marksp - 1) += inc;
        *(marksp - 0) += inc;
        *(marksp + 1) += inc;
        *(marks  - 1) += inc;
        *(marks  + 1) += inc;
        *(marksn - 1) += inc;
        *(marksn - 0) += inc;
        *(marksn + 1) += inc;
      }

      mark++;
      marks++;
      marksp++;
      marksn++;
    }
  }

  CreateMark(mDiffMarkSmooth);
}

void BlockObj::PreConnect()
{
  if (mSmoothBlocks) {
    CreateMarkSmooth();
  } else {
    CreateMark(mDiffMark);
  }

  int currentObj = 0;
  for (int jj = 0; jj < BlockHeight(); jj++) {
    int* object = mPreObjMark.Line(jj);

    bool inObject = false;
    for (int ii = 0; ii < BlockWidth(); ii++) {
      bool isObject = (*object != 0);
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
      }

      object++;
    }
  }

  mObjIds.fill(0, currentObj + 1);

  for (int jj = 0; jj < BlockHeight() - 1; jj++) {
    int* object1 = mPreObjMark.Line(jj);
    int* object2 = mPreObjMark.Line(jj + 1);

    for (int ii = 0; ii < BlockWidth(); ii++) {
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

  mPreObjCount = 0;
  for (int obj = 1; obj <= currentObj; obj++) {
    if (int objRef = mObjIds[obj]) {
      mObjIds[obj] = mObjIds[objRef];
    } else {
      mObjIds[obj] = mPreObjCount;
      mPreObjCount++;
    }
  }
}

void BlockObj::PreConstruct()
{
  QVector<PreObj> preObjects(mPreObjCount);
  InitVector(preObjects, mPreObjCount);

  for (int jj = 0; jj < BlockHeight(); jj++) {
//    const int* diff = DiffMark().Line(jj);
    const int* object = mPreObjMark.Line(jj);
    const BlockInfo* info = GetBlockInfo().Line(jj);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      if (*object) {
        int iObj = mObjIds[*object];
        PreObj* preObj = &preObjects[iObj];
        if (preObj->Moment) {
          preObj->Block.setLeft(qMin(preObj->Block.left(), ii));
          preObj->Block.setRight(qMax(preObj->Block.right(), ii));
          preObj->Block.setTop(qMin(preObj->Block.top(), jj));
          preObj->Block.setBottom(qMax(preObj->Block.bottom(), jj));
        } else {
          preObj->Block.setCoords(ii, jj, ii, jj);
          preObj->Moment = 1;
        }
        preObj->Mass++;
        if (info->Flag & BlockInfo::eDoor) {
          preObj->NearDoor = true;
        }
      }

//      diff++;
      object++;
      info++;
    }
  }

  mObjIds2.resize(preObjects.size());
  mPreObjs.clear();
  for (int i = 0; i < (int)preObjects.size(); i++) {
    PreObj* preObj = &preObjects[i];
    preObj->Moment = (preObj->Block.width() + preObj->Block.height())/2;
    BlockToPlace(preObj->Block, preObj->Place);
    mObjIds2[i] = mPreObjs.size();
    mPreObjs.push_back(*preObj);
  }
  mPreObjCount = 0;
}

void BlockObj::PreCalc()
{
}

void BlockObj::PreFilter()
{
  int maxMoment = 0;
  for (int i = 0; i < mPreObjs.size(); i++) {
    const PreObj& pre = mPreObjs[i];
    maxMoment = qMax(maxMoment, (int)pre.Moment);
  }
  if (maxMoment <= 1) {
    mPreObjs.clear();
    return;
  }
  mFilterMoment = maxMoment/2;
  QList<const PreObj*> okPre;
  for (int i = 0; i < mPreObjs.size(); i++) {
    const PreObj& pre = mPreObjs[i];
    if (pre.Moment >= mFilterMoment) {
      okPre.append(&pre);
      if (okPre.size() > 3) {
        break;
      }
    }
  }
  if (okPre.size() <= 3) {
    qreal norm = 100.0 / ((BlockWidth() + BlockHeight())/4);
    for (int i = 0; i < okPre.size(); i++) {
      const PreObj* pre = okPre.at(i);
      int perc = qMin(255, (int)(pre->Moment * norm));
      mHystMoment.Inc(perc);
    }
  }

  int filter = mNormalMoment? mNormalMoment: mFilterMoment;
  int preId = 0;
  for (int i = 0; i < mPreObjs.size(); preId++) {
    PreObj* pre = &mPreObjs[i];
    if (pre->Moment >= filter) {
      pre->Id = preId;
      i++;
    } else {
      mPreObjs.removeAt(i);
    }
  }
}

void BlockObj::PreAdjust()
{
}

void BlockObj::ObjPrepare()
{
  mConnectMapObj.clear();
  mConnectMapPre.clear();

  qint64 latestFrame = CurrentMs() - kObjTrackPeriod;

  for (int iObj = 0; iObj < mObjs.size(); iObj++) {
    mCurrentObj = &mObjs[iObj];

    while (!mCurrentObj->Track.isEmpty()) {
      if (mCurrentObj->Track.first().Timestamp > latestFrame) {
        break;
      }
      mCurrentObj->Track.removeFirst();
    }

    mCurrentObj->Move = mCurrentObj->Block;
    if (mCurrentObj->Track.size() >= 3) {
      QPoint v = mCurrentObj->Track.last().Block - mCurrentObj->Track.first().Block;
      qint64 t = mCurrentObj->Track.last().Timestamp - mCurrentObj->Track.first().Timestamp;
      v = v * FrameMs() / t;

      if (v.x()) {
        mCurrentObj->Move.setLeft(mCurrentObj->Move.left() + v.x());
        mCurrentObj->Move.setRight(mCurrentObj->Move.right() + v.x());
      }
      if (v.y()) {
        mCurrentObj->Move.setTop(mCurrentObj->Move.top() + v.y());
        mCurrentObj->Move.setBottom(mCurrentObj->Move.bottom() + v.y());
      }
    }

    if (mCurrentObj->Type == eGrow) {
      mCurrentObj->MaxMass = mCurrentObj->NormalMass = qMax(mCurrentObj->NormalMass, mCurrentObj->Mass);
    } else if (mCurrentObj->Type == eGood && mCurrentObj->Track.size() >= 2) {
      mCurrentObj->NormalMass = 0;
      foreach (const ObjStep& step, mCurrentObj->Track) {
        mCurrentObj->NormalMass += step.Mass;
      }
      mCurrentObj->NormalMass /= mCurrentObj->Track.size();
      mCurrentObj->MaxMass = qMax(mCurrentObj->MaxMass, mCurrentObj->NormalMass);
    }

    mCurrentObj->PreIndex = -1;
    mCurrentObj->PreSize = 0;
  }
}

void BlockObj::ObjConnectPre()
{
#ifdef CONNECT_DUMP
  int iStep = CurrentFrame();
  LogLive(QString("--- step %1 ---").arg(CurrentFrame()));
#endif
  for (int iObj = 0; iObj < mObjs.size(); iObj++) {
    mCurrentObj = &mObjs[iObj];
#ifdef CONNECT_DUMP
    LogLive(QString("Obj %5: (%1, %2, %3, %4)")
            .arg(mCurrentObj->Block.left()).arg(mCurrentObj->Block.top())
            .arg(mCurrentObj->Block.width()).arg(mCurrentObj->Block.height()).arg(iObj));
#endif

    for (int iPre = 0; iPre < mPreObjs.size(); iPre++) {
      mCurrentPre = &mPreObjs[iPre];
      int l = qMax(mCurrentObj->Block.left(), mCurrentPre->Block.left());
      int r = qMin(mCurrentObj->Block.right(), mCurrentPre->Block.right());
      int w = r - l + 1;
      if (w <= 0) {
        continue;
      }
      int t = qMax(mCurrentObj->Block.top(), mCurrentPre->Block.top());
      int b = qMin(mCurrentObj->Block.bottom(), mCurrentPre->Block.bottom());
      int h = b - t + 1;
      if (h <= 0) {
        continue;
      }
      int mass = w * h;
      if (mass > 2) {
        ConnectS connect(new Connect());
        connect->Mass = mass;
        connect->Block = QRect(l, t, w, h);
        mConnectMapObj[iObj][iPre] = connect;
        mConnectMapPre[iPre][iObj] = connect;

#ifdef CONNECT_DUMP
        LogLive(QString("connect %5 -> %6: (%1, %2, %3, %4)")
                .arg(connect->Block.left()).arg(connect->Block.top())
                .arg(connect->Block.width()).arg(connect->Block.height()).arg(iObj).arg(iPre));
#endif

      }
    }
  }
}

void BlockObj::ObjConnectAdjust()
{

}

void BlockObj::ObjConnectFilterObj()
{
  for (auto itr = mConnectMapObj.begin(); itr != mConnectMapObj.end(); itr++) {
    int iObj = itr.key();
    QMap<int, ConnectS>* connects = &itr.value();
    if (connects->size() <= 1) {
      continue;
    }
//    mCurrentObj = &mObjs[iObj];

    int bestMass = 0;
    for (auto itr = connects->begin(); itr != connects->end(); itr++) {
      const ConnectS& connect = itr.value();
      if (connect->Mass > bestMass) {
        bestMass = connect->Mass;
      }
    }

    int thresholdMass = bestMass / 3;
    for (auto itr = connects->begin(); itr != connects->end(); ) {
      int iPre = itr.key();
      mCurrentPre = &mPreObjs[iPre];
      const ConnectS& connect = itr.value();
      if (connect->Mass < thresholdMass) {
        itr = connects->erase(itr);
        mConnectMapPre[iPre].remove(iObj);
      } else {
        mCurrentPre->Used++;
        itr++;
      }
    }
  }
}

void BlockObj::ObjConnectFilterPre()
{
  for (auto itr = mConnectMapPre.begin(); itr != mConnectMapPre.end(); itr++) {
    int iPre = itr.key();
    QMap<int, ConnectS>* connects = &itr.value();
    if (connects->size() <= 1) {
      continue;
    }
    bool hasGood = false;
    bool hasExit = false;
    for (auto itr = connects->begin(); itr != connects->end(); itr++) {
      int iObj = itr.key();
      mCurrentObj = &mObjs[iObj];
      if (ObjIsGood()) {
        hasGood = true;
      } else if (mCurrentObj->Type == eExit) {
        hasExit = true;
      }
    }
    if (!hasGood) {
      for (auto itr = connects->begin(); itr != connects->end(); itr++) {
        int iObj = itr.key();
        mCurrentObj = &mObjs[iObj];
        if (mCurrentObj->Type != eExit) {
          mCurrentObj->Type = eObjIllegal;
        }
        mConnectMapObj[iObj].remove(iPre);
      }
      connects->clear();
    } else if (hasExit) {
      for (auto itr = connects->begin(); itr != connects->end(); ) {
        int iObj = itr.key();
        mCurrentObj = &mObjs[iObj];
        if (mCurrentObj->Type == eExit) {
          itr = connects->erase(itr);
          mConnectMapObj[iObj].remove(iPre);
        } else {
          itr++;
        }
      }
    }
  }
}

void BlockObj::ObjSharePre()
{
  for (auto itr = mConnectMapPre.begin(); itr != mConnectMapPre.end(); itr++) {
    int iPre = itr.key();
    QMap<int, ConnectS>* connects = &itr.value();
    if (connects->isEmpty()) {
#ifdef CONNECT_DUMP
      LogLive(QString("Pre %1: n/a").arg(iPre));
#endif
      continue;
    }

    mCurrentPre = &mPreObjs[iPre];
    mCurrentPre->Used = connects->size();
    if (connects->size() == 1) {
      const ConnectS& connect = connects->first();
      connect->Mass = mCurrentPre->Mass;
      connect->Block = mCurrentPre->Block;
      connect->NearDoor = mCurrentPre->NearDoor;
#ifdef CONNECT_DUMP
      LogLive(QString("Pre %5: (%1, %2, %3, %4) con: %6")
              .arg(mCurrentPre->Block.left()).arg(mCurrentPre->Block.top())
              .arg(mCurrentPre->Block.width()).arg(mCurrentPre->Block.height()).arg(iPre).arg(connects->size()));
#endif
      continue;
    }

    Region<int> preUsed;
    preUsed.SetSize(mCurrentPre->Block.width(), mCurrentPre->Block.height());
    preUsed.SetData(0);

    int leftPre = mCurrentPre->Block.left();
    int topPre = mCurrentPre->Block.top();
    for (auto itr = connects->begin(); itr != connects->end(); itr++) {
      const ConnectS& connect = itr.value();
      int leftRel = connect->Block.left() - leftPre;
      int topRel = connect->Block.top() - topPre;
      for (int jj = 0; jj < connect->Block.height(); jj++) {
        int* count = preUsed.Data(leftRel, topRel + jj);
        for (int ii = 0; ii < connect->Block.width(); ii++) {
          (*count)++;

          count++;
        }
      }
    }

    for (int jj = 0; jj < mCurrentPre->Block.height(); jj++) {
      const int* object = mPreObjMark.Data(leftPre, topPre + jj);
      int* count = preUsed.Line(jj);
      for (int ii = 0; ii < mCurrentPre->Block.width(); ii++) {
        if (*object && mObjIds[*object] == mCurrentPre->Id) {
          if (!*count) {
            *count = 2;
          }
        } else {
          *count = 0;
        }

        object++;
        count++;
      }
    }

    QVector<QRect> baseRects;
    for (auto itr = connects->begin(); itr != connects->end(); ) {
      const ConnectS& connect = itr.value();
      int leftCon = connect->Block.left();
      int topCon = connect->Block.top();
      int leftRel = connect->Block.left() - leftPre;
      int topRel = connect->Block.top() - topPre;
      Connect newConnect;
      for (int jj = 0; jj < connect->Block.height(); jj++) {
        const int* count = preUsed.Data(leftRel, topRel + jj);
//        const int* diff = DiffMark().Data(leftCon, topCon + jj);
//        const BlockInfo* info = GetBlockInfo().Data(leftCon, topCon + jj);
        for (int ii = 0; ii < connect->Block.width(); ii++) {
          if (*count == 1) {
            if (!newConnect.Block.isNull()) {
              newConnect.Block.setLeft(qMin(newConnect.Block.left(), ii));
              newConnect.Block.setRight(qMax(newConnect.Block.right(), ii));
              newConnect.Block.setTop(qMin(newConnect.Block.top(), jj));
              newConnect.Block.setBottom(qMax(newConnect.Block.bottom(), jj));
            } else {
              newConnect.Block.setCoords(ii, jj, ii, jj);
            }
            newConnect.Mass++;
//            if (info->Flag & BlockInfo::eDoor) {
//              newConnect.NearDoor = true;
//            }
          }

          count++;
//          diff++;
//          info++;
        }
      }
      if (newConnect.Mass > 0) {
        newConnect.NearDoor = mCurrentPre->NearDoor;
        *connect = newConnect;
        connect->Block.adjust(leftCon, topCon, leftCon, topCon);

        QRect baseRect = connect->Block;
        baseRect.adjust(-leftPre, -topPre, -leftPre, -topPre);
        baseRects.append(baseRect);
        itr++;
      } else {
        int iObj = itr.key();
        itr = connects->erase(itr);
        mConnectMapObj[iObj].remove(iPre);
      }
    }

    for (int jj = 0; jj < mCurrentPre->Block.height(); jj++) {
//      const BlockInfo* info = GetBlockInfo().Data(leftPre, topPre + jj);
      const int* count = preUsed.Line(jj);
//      const int* diff = DiffMark().Data(leftPre, topPre + jj);
      for (int ii = 0; ii < mCurrentPre->Block.width(); ii++) {
        if (*count > 1) {
#ifdef CONNECT_DUMP
          int iStep = CurrentFrame();
#endif
          int takenIndex = -1;
          int minLength = 0x77777777;
          for (int i = 0; i < baseRects.size(); i++) {
            const QRect& baseRect = baseRects.at(i);
            int length = 0;
            if (jj > baseRect.bottom()) {
              length = jj - baseRect.bottom();
              if (ii > baseRect.right()) {
                length += ii - baseRect.right();
              } else if (ii < baseRect.left()) {
                length += baseRect.left() - ii;
              }
            } else if (jj < baseRect.top()) {
              length = baseRect.top() - jj;
              if (ii > baseRect.right()) {
                length += ii - baseRect.right();
              } else if (ii < baseRect.left()) {
                length += baseRect.left() - ii;
              }
            } else {
              if (ii < baseRect.left()) {
                length = baseRect.left() - ii;
              } else if (ii > baseRect.right()) {
                length = ii - baseRect.right();
              }
            }

            //Log.Debug(QString("Shared: (%1, %2) %3: %4").arg(ii).arg(jj).arg(i).arg(length));
            if (length < minLength) {
              if (minLength - length > kShareObjThreshold) {
                takenIndex = i;
              } else {
                takenIndex = -1;
              }
              minLength = length;
            } else if (length - minLength <= kShareObjThreshold) {
              takenIndex = -1;
            }
          }

          //Log.Debug(QString("Shared: (%1, %2) -> %3").arg(ii).arg(jj).arg(takenIndex));
          if (takenIndex >= 0) {
            int index = 0;
            for (auto itr = connects->begin(); itr != connects->end(); itr++) {
              const ConnectS& connect = itr.value();
              if (takenIndex == index) {
                connect->Block.setLeft(qMin(connect->Block.left(), leftPre + ii));
                connect->Block.setRight(qMax(connect->Block.right(), leftPre + ii));
                connect->Block.setTop(qMin(connect->Block.top(), topPre + jj));
                connect->Block.setBottom(qMax(connect->Block.bottom(), topPre + jj));
//                if (info->Flag & BlockInfo::eDoor) {
//                  connect->NearDoor = true;
//                }
                connect->Mass++;
                break;
              }
              index++;
            }
          }
        }

        count++;
//        info++;
//        diff++;
      }
    }

#ifdef CONNECT_DUMP
    QString dbgText = QString("share %5: (%1, %2, %3, %4) -> ")
        .arg(mCurrentPre->Block.left()).arg(mCurrentPre->Block.top())
        .arg(mCurrentPre->Block.width()).arg(mCurrentPre->Block.height()).arg(iPre);
    for (auto itr = connects->begin(); itr != connects->end(); itr++) {
      int iObj = itr.key();
      mCurrentObj = &mObjs[iObj];
      const ConnectS& connect = itr.value();
      dbgText.append(QString("%5 (%1, %2, %3, %4), ").arg(connect->Block.left()).arg(connect->Block.top())
                     .arg(connect->Block.width()).arg(connect->Block.height()).arg(mCurrentObj->Id));
    }
    LogLive(dbgText);
#endif
  }
}

void BlockObj::ObjDone()
{
//  mEnergyMs += FrameMs();

//  int iStep = CurrentFrame();
  int objCount = mObjs.size();
  for (int iObj = 0; iObj < mObjs.size(); iObj++) {
    mCurrentObj = &mObjs[iObj];
    QMap<int, ConnectS>* connects = &mConnectMapObj[iObj];

    if (mCurrentObj->Type == eObjIllegal) {
      continue;
    }
    ObjFilterConnections(iObj, connects);
    if (connects->size() > 1) {
      ObjUseAllConnects(connects);
    } else if (connects->size() == 1) {
      const ConnectS& connect = connects->first();
      ObjUseConnect(connect, connects->firstKey());
      ObjPromoteQuality(100, 1);
    } else if (iObj < objCount) {
      if (ObjLostQuality(100, 1)/* || !ObjIsGood()*/) {
        continue;
      } else {
        ObjDisonnect();
      }
    }
  }
}

void BlockObj::ObjMove()
{
  for (int iObj = 0; iObj < mObjs.size(); iObj++) {
    mCurrentObj = &mObjs[iObj];
    mCurrentObj->Block = mCurrentObj->Move;
    QPoint p1 = (mCurrentObj->Place.topLeft() + mCurrentObj->Place.bottomRight())/2;
    BlockToPlace(mCurrentObj->Block, mCurrentObj->Place);
    QPoint p2 = (mCurrentObj->Place.topLeft() + mCurrentObj->Place.bottomRight())/2;
    if (p1 != p2) {
      ObjMoveBarriers(p1, p2);
    }
    ObjAddStep();
  }
}

void BlockObj::ObjCleanup()
{
  for (auto itr = mObjs.begin(); itr != mObjs.end(); ) {
    const Obj& obj = *itr;
    if (obj.Type == eObjIllegal) {
      itr = mObjs.erase(itr);
    } else {
      itr++;
    }
  }
}

void BlockObj::ObjNew()
{
  foreach (const PreObj& pre, mPreObjs) {
    if (!pre.Used) {
      ObjCreateNewOne(pre);
    }
  }
}

void BlockObj::ObjDump()
{
  QString objInfo = QString(" --- step %1 ---         ").arg(CurrentFrame());
  for (int iObj = 0; iObj < mObjs.size(); iObj++) {
    mCurrentObj = &mObjs[iObj];
    objInfo.append(QString("\nId: %1, type: %2 (%3, %4, %5, %6) mass: %7, norm: %8, max: %9")
               .arg(mCurrentObj->Id).arg(ObjTypeToString())
               .arg(mCurrentObj->Block.left()).arg(mCurrentObj->Block.top())
               .arg(mCurrentObj->Block.width()).arg(mCurrentObj->Block.height())
               .arg(mCurrentObj->Mass).arg(mCurrentObj->NormalMass).arg(mCurrentObj->MaxMass));
  }
  LogLive(objInfo);
}

void BlockObj::ObjMoveBarriers(const QPoint& p1, const QPoint& p2)
{
  for (int i = 0; i < mBarierInfos.size(); i++) {
    const QVector<QPoint>& points = mBarierInfos.at(i).Points;

    //int iStep = CurrentFrame();
    int inOut = ObjMoveBarierOne(p1, p2, points);
    if (inOut) {
      ObjHitBarrier(i, inOut > 0);
    }
  }
}

//void BlockObj::ObjMoveZone(const QPointF& p1, const QPointF& p2)
//{
//  //if (InZones().isEmpty()) {
//  //  return;
//  //}
//  //if (!ObjInZone((int)(p1.x() + 0.5), (int)(p1.y() + 0.5))) {
//  //  ObjInZone((int)(p2.x() + 0.5), (int)(p2.y() + 0.5));
//  //}
//}
//
//bool BlockObj::ObjInZone(int ii, int jj)
//{
//  //if (ii >= 0 && ii < GetBlockInfo().Width() && jj >= 0 && jj < GetBlockInfo().Height()) {
//  //  const BlockInfo* info = GetBlockInfo().Line(jj) + ii;
//  //  if ((info->Flag & BlockInfo::eZone) != 0) {
//  //    int iZone = (int)(info->Flag & BlockInfo::eValueMask);
//  //    if (iZone >= 0 && iZone < InZones().size()) {
//  //      auto itr = mCurrentObj->ZoneHit.find(iZone);
//  //      if (itr != mCurrentObj->ZoneHit.end()) {
//  //        if (itr->InZoneMs < mInZoneTime) {
//  //          itr->InZoneMs += FrameMs();
//  //          if (itr->InZoneMs >= mInZoneTime) {
//  //            Log.Trace(QString("[%1] Object %2: in zone detected").arg(iZone).arg(mCurrentObj->Id));
//  //          }
//  //        }
//  //      } else {
//  //        Obj::ZoneInf inf;
//  //        inf.Time = CurTimestamp();
//  //        inf.InZoneMs = 0;
//  //        mCurrentObj->ZoneHit[iZone] = inf;
//  //        Log.Trace(QString("[%1] Object %2: in zone").arg(iZone).arg(mCurrentObj->Id));
//  //      }
//  //      return true;
//  //    }
//  //  }
//  //}
//
//  return false;
//}
//
int BlockObj::ObjMoveBarierOne(const QPoint& p1, const QPoint& p2, const QVector<QPoint>& points)
{
  QPoint v = p2 - p1;
  //if (v.x() == 0 && v.y() == 0) {
  //  return 0;
  //}

  int in = 0;
  int out = 0;
  auto itr = points.begin();
  QPoint b1 = *itr;
  for (itr++; itr != points.end(); itr++) {
    QPoint b2 = *itr;

    // *** FORMULA ***
    // QPoint q = b2 - b1;
    //
    // k1 * v + p1 = k2 * q + b1
    // v * k1 - q * k2 + (p1 - b1)
    //
    // v * k1 + r * k2 + f = 0
    // vx * k1 + rx * k2 + fx = 0
    // vy * k1 + ry * k2 + fy = 0
    // vx * k1 * vy + rx * k2 * vy + fx * vy = 0
    // vy * k1 * vx + ry * k2 * vx + fy * vx = 0
    // (rx * vy - ry * vx) * k2 + (fx * vy - fy * vx) = 0
    // a*k2 = b; k2 = b/a
    // 0 <= k2 <= 1
    // 0 <= b <= a
    QPoint r = b1 - b2;
    QPoint f = p1 - b1;
    int a = r.x()*v.y() - r.y()*v.x();
    int b = f.y()*v.x() - f.x()*v.y();
    if (a < 0) {
      a = -a;
      b = -b;
    }
    if (b >= 0 && a >= b) {
      // k2 = b/a
      // vx * k1 = - (fx + rx * b/a)
      // c * k1 = d; k1 = d/c
      // 0 <= k1 < 1
      // 0 <= d < c
      bool firstEq = qAbs(v.x()) >= qAbs(v.y());
      int c = (firstEq)? v.x() * a: v.y() * a;
      int d = (firstEq)? -(f.x() * a + r.x() * b): -(f.y() * a + r.y() * b);
      if (c < 0) {
        c = -c;
        d = -d;
      }
      if (d >= 0 && c > d) {
        int sq = v.x() * r.y() - v.y() * r.x();
        if (sq < 0) {
          in++;
        } else if (sq > 0) {
          out++;
        }
      }
    }

    b1 = b2;
  }

  if (in > out) {
    return 1;
  } else if (out > in) {
    return -1;
  }
  return 0;
}

void BlockObj::ObjHitBarrier(int index, bool in)
{
  QMap<int, ObjHit>* objHits = &mObjHitMap[mCurrentObj->Id];
  auto itr = objHits->find(index);
  if (itr != objHits->end()) {
    ObjHit* objHit = &itr.value();
    if (objHit->In == in) {
      objHit->Time = CurTimestamp();
    } else {
      objHits->erase(itr);
      Log.Trace(QString("[%1] Object %2: %3 cancel").arg(index).arg(mCurrentObj->Id).arg(in? "out": "in"));
    }

    ObjUpdateShot();
  } else {
    ObjHit inf;
    inf.In = in;
    inf.Time = CurTimestamp();
    objHits->insert(index, inf);
    Log.Trace(QString("[%1] Object %2: %3").arg(index).arg(mCurrentObj->Id).arg(in? "in": "out"));

    ObjMakeShot();
  }
}

void BlockObj::ObjMakeShot()
{
  if (!mUseScreenshots) {
    return;
  }

  auto itr = mObjShotMap.find(mCurrentObj->Id);
  bool created = false;
  if (itr == mObjShotMap.end()) {
    itr = mObjShotMap.insert(mCurrentObj->Id, ObjShot());
    created = true;
  }

  ObjShot* shot = &itr.value();
  ObjUpdateShotTrace(shot);
  if (created || CurrentMs() > shot->BodyTimestamp + kShotOutdateMs) {
    ObjUpdateShotBody(shot);
  }
}

void BlockObj::ObjUpdateShot()
{
  if (!mUseScreenshots) {
    return;
  }

  auto itr = mObjShotMap.find(mCurrentObj->Id);
  if (itr != mObjShotMap.end()) {
    ObjShot* shot = &itr.value();
    ObjUpdateShotTrace(shot);
  }
}

void BlockObj::ObjUpdateShotTrace(BlockObj::ObjShot* shot)
{
  for (auto itr = mCurrentObj->Track.begin(); itr != mCurrentObj->Track.end(); itr++) {
    const ObjStep& step = *itr;
    if (step.Timestamp > shot->TraceTimestamp) {
      for (; itr != mCurrentObj->Track.end(); itr++) {
        const ObjStep& step = *itr;
        shot->Trace.append(step.Block);
        shot->TraceTimestamp = step.Timestamp;
      }
      break;
    }
  }
}

void BlockObj::ObjUpdateShotBody(BlockObj::ObjShot* shot)
{
  const QRect& block = mCurrentObj->Block;
  shot->Place = block;
  shot->BodyMark.SetSize(block.width(), block.height());
  int count = 0;
  for (int j = 0; j < block.height(); j++) {
    const int* src = mPreObjMark.Data(block.left(), block.top() + j);
    uchar* body = shot->BodyMark.Line(j);
    for (int i = 0; i < block.width(); i++) {
      if (*src > 0) {
        *body = 1;
        count++;
      } else {
        *body = 0;
      }

      body++;
      src++;
    }
  }
  shot->BodyData.resize(count * BlockSize()*BlockSize());
  if (!mLayerFront) {
    shot->BodyData.fill((char)(uchar)0xff);
    return;
  }

  uchar* data = (uchar*)shot->BodyData.data();
  for (int j = 0; j < block.height(); j++) {
    const uchar* body = shot->BodyMark.Line(j);
    for (int i = 0; i < block.width(); i++) {
      if (*body) {
        int iBlock = (block.left() + i) * BlockSize();
        int jBlock = (block.top() + j) * BlockSize();

        for (int j = 0; j < BlockSize(); j++) {
          const uchar* src = mLayerFront->Data(iBlock, jBlock + j);
          memcpy(data, src, BlockSize());
          data += BlockSize();
        }
      }

      body++;
    }
  }
}

void BlockObj::ObjApplyDetectors()
{
  auto itr = mObjHitMap.find(mCurrentObj->Id);
  if (itr != mObjHitMap.end()) {
    ObjHitBarier(itr.value());
    mObjHitMap.erase(itr);
  }
}

void BlockObj::ObjHitBarier(const QMap<int, BlockObj::ObjHit>& hits)
{
  if (mCurrentObj->MaxMass <= 0) {
    return;
  }

  QByteArray img;
  bool hasImg = MakeShotImage(img);

  for (auto itr = hits.begin(); itr != hits.end(); itr++) {
    int index = itr.key();
    const ObjHit& hit = itr.value();
    const AnalyticsB::Barier& barier = Bariers().at(index);
    Log.Info(QString("Barier [%1]: %2").arg(barier.Detector->Id).arg(hit.In? "in": "out"));
//    QPoint c = (mCurrentObj->Block.topLeft() + mCurrentObj->Block.bottomRight())/2;
//    int move = (mCurrentObj->First - c).manhattanLength();
//    Log.Debug(QString("Hit info (id: %1, move: %2, exit: %3, mass: %4)")
//              .arg(mCurrentObj->Id).arg(move).arg(mCurrentObj->GoodExit? "good": "bad").arg(mCurrentObj->MaxMass));
    if (hasImg) {
      barier.Detector->Hit(hit.Time, hit.In? "in": "out", img);
    } else {
      barier.Detector->Hit(hit.Time, hit.In? "in": "out");
    }
  }
}

bool BlockObj::MakeShotImage(QByteArray& img)
{
  if (!mUseScreenshots) {
    return false;
  }

  auto itr = mObjShotMap.find(mCurrentObj->Id);
  if (itr == mObjShotMap.end()) {
    return false;
  }

  ObjShot* shot = &itr.value();
  ObjUpdateShotTrace(shot);

  QRect part = shot->Place;
  foreach (const QPoint& p, shot->Trace) {
    part.setLeft(qMin(part.left(), p.x()));
    part.setRight(qMax(part.right(), p.x()));
    part.setTop(qMin(part.top(), p.y()));
    part.setBottom(qMax(part.bottom(), p.y()));
  }
  part.setLeft(qMax(0, BlockSize() * (part.left() - 4)));
  part.setRight(qMin(Width() - 1, BlockSize() * (part.right() + 4)));
  part.setTop(qMax(0, BlockSize() * (part.top() - 4)));
  part.setBottom(qMin(Height() - 1, BlockSize() * (part.bottom() + 4)));

  QByteArray imgData;
  imgData.resize(part.width() * part.height());
  Region<uchar> region;
  region.SetSource((uchar*)imgData.data(), part.width(), part.height(), part.width());
  if (mLayerBackground) {
    for (int j = 0; j < region.Height(); j++) {
      const uchar* src = mLayerBackground->Data(part.left(), part.top() + j);
      uchar* dst = region.Line(j);
      memcpy(dst, src, region.Width());
    }
  } else {
    region.ZeroData();
  }

  uchar* data = (uchar*)shot->BodyData.data();
  for (int j = 0; j < shot->BodyMark.Height(); j++) {
    const uchar* block = shot->BodyMark.Line(j);
    for (int i = 0; i < shot->BodyMark.Width(); i++) {
      if (*block) {
        int iBlock = BlockSize() * (shot->Place.left() + i) - part.left();
        int jBlock = BlockSize() * (shot->Place.top() + j) - part.top();

        for (int j = 0; j < BlockSize(); j++) {
          uchar* dst = region.Data(iBlock, jBlock + j);
          memcpy(dst, data, BlockSize());
          data += BlockSize();
        }
      }
      block++;
    }
  }

  if (shot->Trace.size() >= 2) {
    ImageSrc<uchar> painter(GetBlockScene());
    painter.SetSource((uchar*)imgData.data());
    painter.SetStride(region.Width());
    auto itr = shot->Trace.begin();
    const QPoint* p1 = &*itr;
    for (itr++; itr != shot->Trace.end(); itr++) {
      const QPoint* p2 = &*itr;
      int iPoint1 = BlockSize() * p1->x() - part.left();
      int jPoint1 = BlockSize() * p1->y() - part.top();
      int iPoint2 = BlockSize() * p2->x() - part.left();
      int jPoint2 = BlockSize() * p2->y() - part.top();
      painter.FillLine(0xff, iPoint1, jPoint1, iPoint2, jPoint2);

      p1 = &*itr;
    }
  }

  return CreateSnapshot(region, img);
}

void BlockObj::ObjConfirm()
{
  for (int iObj = 0; iObj < mObjs.size(); iObj++) {
    mCurrentObj = &mObjs[iObj];

    switch (mCurrentObj->Type) {
    case eGrow:   ObjConfirmGrow(); break;
    case eGood:   ObjConfirmGood(); break;
    case eCasper: ObjConfirmCasper(); break;
    case eAway:   ObjConfirmAway(); break;
    case eExit:   ObjConfirmExit(iObj); break;
    case eObjIllegal: break;
    default: Log.Warning(QString("Invalid object Confirm (id: %1)").arg(mCurrentObj->Id)); break;
    }

    if (mCurrentObj->Type == eObjIllegal) {
      ObjApplyDetectors();
      mObjs.removeAt(iObj);
      iObj--;
    }
  }
}

void BlockObj::ObjConfirmGrow()
{
}

void BlockObj::ObjConfirmGood()
{
}

void BlockObj::ObjConfirmCasper()
{
  QPoint c = (mCurrentObj->Block.topLeft() + mCurrentObj->Block.bottomRight())/2;
  int move = (mCurrentObj->First - c).manhattanLength();
  if (move > qMin(BlockWidth(), BlockHeight()) / 2) {
    mCurrentObj->Type = eGood;
  }
}

void BlockObj::ObjConfirmAway()
{
  if (mCurrentObj->Mass < mCurrentObj->NormalMass/2 || mCurrentObj->Mass < mCurrentObj->MaxMass/4) {
    mCurrentObj->Type = eExit;
    mCurrentObj->GoodExit = true;
    mCurrentObj->NormalMass = mCurrentObj->Mass;
  }
}

void BlockObj::ObjConfirmExit(int iObj)
{
  if (!ObjLostQuality(100, 1)) {
    if (mCurrentObj->PreIndex >= 0 && mCurrentObj->Mass > mCurrentObj->NormalMass*3/2) {
      mCurrentObj->Type = eObjIllegal;
      int iPre = mCurrentObj->PreIndex;
      mConnectMapObj[iObj].remove(iPre);
      mConnectMapPre[iPre].remove(iObj);
    } else {
      mCurrentObj->NormalMass = qMin(mCurrentObj->Mass, mCurrentObj->NormalMass);
    }
  }
}

bool BlockObj::ObjPromoteQuality(int qualityBase, int multiply)
{
  mCurrentObj->Quality += (int)(qualityBase * multiply * 10 * FrameSec());
  if (mCurrentObj->Quality > kObjQualityMax) {
    mCurrentObj->Quality = kObjQualityMax;
    return true;
  }
  return false;
}

bool BlockObj::ObjLostQuality(int qualityBase, int multiply)
{
  mCurrentObj->Quality -= (int)(qualityBase * multiply * 10 * FrameSec());
  if (mCurrentObj->Quality < 0) {
    LogLive(QString("Obj lost quality (id: %1)").arg(mCurrentObj->Id));
    mCurrentObj->Type = eObjIllegal;
    return true;
  }
  return false;
}

void BlockObj::ObjCreateNewOne(const BlockObj::PreObj& pre)
{
  Obj objTemp;
  mObjs.append(objTemp);
  mCurrentObj = &mObjs.last();
  mCurrentObj->Id = ++mObjectId;
  mCurrentObj->Type = pre.NearDoor? eGrow: eCasper;
  mCurrentObj->Quality = kObjQualityStart;
  mCurrentObj->Place = pre.Place;
  mCurrentObj->Block = pre.Block;
  mCurrentObj->First = (pre.Block.topLeft() + pre.Block.bottomRight())/2;
  mCurrentObj->NormalMass = mCurrentObj->MaxMass = mCurrentObj->Mass = 0;
  mCurrentObj->GoodExit = false;
  //BlockToBorder(mCurrentObj->Place, pre.Id, mCurrentObj->Border);
  ObjAddStep();

  LogLive(QString("Create new obj (id: %1, type: '%2', ts: %3, r: (%4, %5, %6, %7))").arg(mCurrentObj->Id).arg(ObjTypeToString()).arg(CurrentMs())
          .arg(mCurrentObj->Block.left()).arg(mCurrentObj->Block.top()).arg(mCurrentObj->Block.width()).arg(mCurrentObj->Block.height()));
}

void BlockObj::ObjCreateClone()
{
  mObjs.append(*mCurrentObj);
  mCurrentObj = &mObjs.last();
  mCurrentObj->Id = ++mObjectId;

  LogLive(QString("Create clone obj (id: %1, type: '%2', ts: %3, r: (%4, %5, %6, %7))").arg(mCurrentObj->Id).arg(ObjTypeToString()).arg(CurrentMs())
          .arg(mCurrentObj->Block.left()).arg(mCurrentObj->Block.top()).arg(mCurrentObj->Block.width()).arg(mCurrentObj->Block.height()));
}

void BlockObj::ObjAddStep()
{
  ObjStep step;
  step.Mass = mCurrentObj->Mass;
  step.Block = (mCurrentObj->Block.topLeft() + mCurrentObj->Block.bottomRight())/2;
  step.Timestamp = CurrentMs();
  mCurrentObj->Track.append(step);
}

void BlockObj::ObjAdjustSteps(const QLinkedList<ObjStep>& baseTrack, const QPoint& adjustVect)
{
  foreach (const ObjStep& baseStep, baseTrack) {
    ObjStep step = baseStep;
    step.Mass = 0;
    step.Block += adjustVect;
    mCurrentObj->Track.append(step);
  }
}

void BlockObj::ObjUseAllConnects(QMap<int, ConnectS>* connects)
{
  QPoint baseCenter = (mCurrentObj->Block.topLeft() + mCurrentObj->Block.bottomRight())/2;
  QLinkedList<ObjStep> baseTrack;
  baseTrack.swap(mCurrentObj->Track);
  int connectIndex = 0;
  for (auto itr = connects->begin(); itr != connects->end(); itr++) {
    if (connectIndex > 0) {
      ObjCreateClone();
    }
    const ConnectS& connect = itr.value();
    ObjUseConnect(connect, itr.key());
    QPoint newCenter = (mCurrentObj->Move.topLeft() + mCurrentObj->Move.bottomRight())/2;
    ObjAdjustSteps(baseTrack, newCenter - baseCenter);

    connectIndex++;
  }
}

void BlockObj::ObjUseConnect(const ConnectS& connect, int iPre)
{
  mCurrentObj->Mass = connect->Mass;
  mCurrentObj->Move = connect->Block;
  mCurrentObj->PreIndex = iPre;

  switch (mCurrentObj->Type) {
  case eGrow:
    if (!connect->NearDoor) {
      mCurrentObj->Type = eGood;
    }
    break;
  case eGood:
    if (connect->NearDoor) {
      mCurrentObj->Type = eAway;
    }
    break;
  case eCasper:
    break;
  case eAway:
    if (!connect->NearDoor) {
      mCurrentObj->Type = eGood;
    }
    break;
  case eExit:
  case eObjIllegal:
  default:
    break;
  }
}

void BlockObj::ObjDisonnect()
{
  mCurrentObj->Move = mCurrentObj->Block;

  switch (mCurrentObj->Type) {
  case eGrow:
    mCurrentObj->Type = eObjIllegal;
    break;
  case eCasper:
  case eAway:
    mCurrentObj->Type = eExit;
    break;
  case eGood:
    if (mCurrentObj->Quality < kObjQualityStart) {
      mCurrentObj->Type = eExit;
    }
    break;
  case eExit:
  case eObjIllegal:
  default:
    break;
  }
}

void BlockObj::ObjFilterConnections(int iObj, QMap<int, ConnectS>* connects)
{
  int minMass = mCurrentObj->NormalMass / 4;
  //QString text = QString("Min: %1").arg(minMass);
  if (mCurrentObj->Type != eExit) {
    for (auto itr = connects->begin(); itr != connects->end(); ) {
      const ConnectS& connect = itr.value();
      if (connect->Mass < minMass) {
        //text.append(QString(", (%1, %2, %3, %4) [-%5]").arg(connect->Block.left()).arg(connect->Block.top())
        //            .arg(connect->Block.width()).arg(connect->Block.height()).arg(connect->Mass));
        int iPre = itr.key();
        itr = connects->erase(itr);
        mConnectMapPre[iPre].remove(iObj);
      } else {
        //text.append(QString(", (%1, %2, %3, %4) [+%5]").arg(connect->Block.left()).arg(connect->Block.top())
        //            .arg(connect->Block.width()).arg(connect->Block.height()).arg(connect->Mass));
        itr++;
      }
    }
  }

  //Log.Debug(text);
//  if (connects->isEmpty()) {
//    return;
//  }
//  if (connects->size() == 1) {
//    return;
//  }

//  int maxMass = 0;
//  for (auto itr = connects->begin(); itr != connects->end(); itr++) {
//    const ConnectS& connect = itr.value();
//    maxMass = qMax(maxMass, connect->Mass);
//  }
//  int minMass = qMax(mCurrentObj->Mass / 4, maxMass / 4);
//  QString text = QString("Max: %1, Min: %2").arg(maxMass).arg(minMass);
//  for (auto itr = connects->begin(); itr != connects->end(); ) {
//    const ConnectS& connect = itr.value();
//    if (connect->Mass < minMass) {
//      text.append(QString(", (%1, %2, %3, %4) [-%5]").arg(connect->Block.left()).arg(connect->Block.top())
//                  .arg(connect->Block.width()).arg(connect->Block.height()).arg(connect->Mass));
//      itr = connects->erase(itr);
//    } else {
//      text.append(QString(", (%1, %2, %3, %4) [+%5]").arg(connect->Block.left()).arg(connect->Block.top())
//                  .arg(connect->Block.width()).arg(connect->Block.height()).arg(connect->Mass));
//      itr++;
//    }
//  }
//  Log.Debug(text);
}

bool BlockObj::ObjIsGood()
{
  switch (mCurrentObj->Type) {
  case eGrow:    return mCurrentObj->Quality > kObjQualityGood;
  case eGood:    return true;
  case eCasper:  return mCurrentObj->Quality > kObjQualityGood;
  case eAway:    return true;
  case eExit:    return false;
  case eObjIllegal: return false;
  default:       return false;
  }
}

int BlockObj::ObjGetColor()
{
  /* int ind = Color / 1000; int value = qMin(100, Color % (1000*ind)); ind: 0 - White, 1 - Green, 2 - Red, 3 - Blue (gObjectColors)*/
  int value = qMin(100, mCurrentObj->Quality / 10);
  switch (mCurrentObj->Type) {
  case eGrow:    return 3000 + value;
  case eGood:    return 1000 + value;
  case eCasper:  return 0000 + value;
  case eAway:    return 3000 + value;
  case eExit:    return 0000 + value;
  case eObjIllegal: return 2000 + value;
  default:       return 0 + value;
  }
}

const char* BlockObj::ObjTypeToString()
{
  switch (mCurrentObj->Type) {
  case eGrow:    return "Grow";
  case eGood:    return "Good";
  case eCasper:  return "Casper";
  case eAway:    return "Away";
  case eExit:    return "Exit";
  case eObjIllegal: return "Illegal";
  default:       return "xxx";
  }
}

//void BlockObj::UinDetect()
//{
//  foreach (const PreObj& pre, mPreObjs) {
//    if (pre.Place.width() > kMinUinWidth && pre.Place.height() > UinPre::MinHeight()) {
//      int regionI = pre.Place.x();
//      int regionJ = pre.Place.y();
//      mUinPre->setRegionI(regionI);
//      mUinPre->setRegionJ(regionJ);
//      mUinPre->refDetectRegion().SetSource(const_cast<uchar*>(ImageData().Data(regionI, regionJ))
//                                           , pre.Place.width(), pre.Place.height(), ImageData().Stride());
//      mUinPre->Calc();
//    }
//  }
//}
//
void BlockObj::BlockToPlace(const QRect& block, QRect& place)
{
  place.setLeft(block.left() * BlockSize());
  place.setRight(block.right() * BlockSize() + BlockSize() - 1);
  place.setTop(block.top() * BlockSize());
  place.setBottom(block.bottom() * BlockSize() + BlockSize() - 1);
}

//void BlockObj::BlockToBorder(const QRect& block, int id, BlockBorder& blockBorder)
//{
//  blockBorder.Init(block);
//
//  for (int jj = 0; jj < block.height(); jj++) {
//    const int* object = mPreObjMark.Data(block.left(), block.top() + jj);
//    for (int ii = 0; ii < block.width(); ii++) {
//      if (*object && mObjIds[*object] == id) {
//        blockBorder.TestTopLeft(ii, jj);
//      }
//      object++;
//    }
//  }
//}

void BlockObj::GetDbgPreObjMark(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    byte* dbg = debug.Line(j);
    const int* mark = DiffMark().BlockLine(j);
    const BlockInfo* info = GetBlockInfo().BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      byte val = (*mark > 2*mMarkThreshold)? 4: ((*mark > mMarkThreshold)? 3: 0);
      if ((info->Flag & BlockInfo::eIgnore) != 0) {
        val = 0;
      }
      for (int i = 0; i < BlockSize(); i++) {
        *dbg = val;
        dbg++;
      }
      mark++;
      info++;
    }
  }
}

void BlockObj::GetDbgPreObj(ImageSrc<uchar>& debug)
{
  debug.ClearSource();

  for (int ind = 0; ind < mPreObjs.size(); ind++) {
    const PreObj& pre = mPreObjs[ind];

    //BlockBorder blockBorder;
    //BlockToBorder(pre.Block, pre.Id, blockBorder);


    //for (int jj = 0; jj < pre.Block.height(); jj++) {
    //  for (int ii = 0; ii < pre.Block.width(); ii++) {
    //    if (blockBorder.getLeft()[jj] == ii || blockBorder.getRight()[jj] == ii
    //        || blockBorder.getTop()[ii] == jj || blockBorder.getBottom()[ii] == jj) {
    //      for (int j = 0; j < BlockSize(); j++) {
    //        for (int i = 0; i < BlockSize(); i++) {
    //          *debug.Data((pre.Block.left() + ii)*BlockSize() + i, (pre.Block.top() + jj)*BlockSize() + j) = 3;
    //        }
    //      }
    //    }
    //  }
    //}

    for (int jj = pre.Block.top(); jj <= pre.Block.bottom(); jj++) {
      const int* object = mPreObjMark.Line(jj) + pre.Block.left();
      for (int ii = pre.Block.left(); ii <= pre.Block.right(); ii++) {
        if (*object) {
          int iObj = mObjIds[*object];
          if (iObj == pre.Id) {

            for (int j = 0; j < BlockSize(); j++) {
              const uchar* layD = mLayerDiff.Data(ii*BlockSize(), jj*BlockSize() + j);
              for (int i = 0; i < BlockSize(); i++) {
                if (*layD > 1) {
                  *debug.Data(ii*BlockSize() + i, jj*BlockSize() + j) = 1;
                }
                layD++;
              }
            }
          }
        }
        object++;
      }
    }

    debug.FillLine4(pre.NearDoor? 3: 2, pre.Place);

//    int weight;
//    int weightLimit = pre.Weight/8;
//    weight = 0;
//    for (int j = pre.Place.top(); j <= pre.Place.bottom(); j++) {
//      const int* object = mPreObjMark.BlockLine(j) + pre.Block.left();
//      const uchar* layD = mLayerDiff.Data(pre.Block.left()*BlockSize(), j);
//      for (int ii = pre.Block.left(); ii <= pre.Block.right(); ii++) {
//        if (*object) {
//          int iObj = mObjIds[*object];
//          if (iObj == pre.Id) {
//            for (int i = 0; i < BlockSize(); i++) {
//              if (*layD) {
//                if (*layD > 1) {
//                  *debug.Data(ii*BlockSize() + i, j) = 3;
//                }
//                weight += *layD;
//              }
//              layD++;
//            }
//          } else {
//            layD += BlockSize();
//          }
//        } else {
//          layD += BlockSize();
//        }
//        object++;
//      }
//      if (weight > weightLimit) {
//        break;
//      }
//    }

//    weight = 0;
//    for (int j = pre.Place.bottom(); j >= pre.Place.top(); j--) {
//      const int* object = mPreObjMark.BlockLine(j) + pre.Block.left();
//      const uchar* layD = mLayerDiff.Data(pre.Block.left()*BlockSize(), j);
//      for (int ii = pre.Block.left(); ii <= pre.Block.right(); ii++) {
//        if (*object) {
//          int iObj = mObjIds[*object];
//          if (iObj == pre.Id) {
//            for (int i = 0; i < BlockSize(); i++) {
//              if (*layD) {
//                if (*layD > 1) {
//                  *debug.Data(ii*BlockSize() + i, j) = 3;
//                }
//                weight += *layD;
//              }
//              layD++;
//            }
//          } else {
//            layD += BlockSize();
//          }
//        } else {
//          layD += BlockSize();
//        }
//        object++;
//      }
//      if (weight > weightLimit) {
//        break;
//      }
//    }

//    weight = 0;
//    for (int i = pre.Place.left(); i <= pre.Place.right(); i++) {
//      const int* object = mPreObjMark.BlockData(i, pre.Place.top());
//      const uchar* layD = mLayerDiff.Data(i, pre.Block.top()*BlockSize());
//      for (int jj = pre.Block.top(); jj <= pre.Block.bottom(); jj++) {
//        if (*object) {
//          int iObj = mObjIds[*object];
//          if (iObj == pre.Id) {
//            for (int j = 0; j < BlockSize(); j++) {
//              if (*layD) {
//                if (*layD > 1) {
//                  *debug.Data(i, jj*BlockSize() + j) = 3;
//                }
//                weight += *layD;
//              }
//              layD += mLayerDiff.Stride();
//            }
//          } else {
//            layD += mLayerDiff.Stride() * BlockSize();
//          }
//        } else {
//          layD += mLayerDiff.Stride() * BlockSize();
//        }
//        object += mPreObjMark.Stride();
//      }
//      if (weight > weightLimit) {
//        break;
//      }
//    }

//    weight = 0;
//    for (int i = pre.Place.right(); i >= pre.Place.left(); i--) {
//      const int* object = mPreObjMark.BlockData(i, pre.Place.top());
//      const uchar* layD = mLayerDiff.Data(i, pre.Block.top()*BlockSize());
//      for (int jj = pre.Block.top(); jj <= pre.Block.bottom(); jj++) {
//        if (*object) {
//          int iObj = mObjIds[*object];
//          if (iObj == pre.Id) {
//            for (int j = 0; j < BlockSize(); j++) {
//              if (*layD) {
//                if (*layD > 1) {
//                  *debug.Data(i, jj*BlockSize() + j) = 3;
//                }
//                weight += *layD;
//              }
//              layD += mLayerDiff.Stride();
//            }
//          } else {
//            layD += mLayerDiff.Stride() * BlockSize();
//          }
//        } else {
//          layD += mLayerDiff.Stride() * BlockSize();
//        }
//        object += mPreObjMark.Stride();
//      }
//      if (weight > weightLimit) {
//        break;
//      }
//    }
  }

  debug.FillRhombus(3, Width()/2, Height()/2, mNormalMoment * BlockSize(), mNormalMoment * BlockSize());
  debug.FillRhombus(4, Width()/2, Height()/2, mFilterMoment * BlockSize(), mFilterMoment * BlockSize());
}

void BlockObj::GetDbgObj(ImageSrc<uchar>& debug)
{
  debug.ClearSource();

  auto bariers = Bariers();
  int barierIndex = 1;
  foreach (const AnalyticsB::Barier& barier, bariers) {
    if (barier.Points.size() >= 2) {
      QPoint p1 = PointPercentToScene(barier.Points.first());
      QPoint c = p1;
      for (int i = 1; i < barier.Points.size(); i++) {
        QPoint p2 = PointPercentToScene(barier.Points.at(i));
        c += p2;

        debug.FillLine(1, p1, p2);
        p1 = p2;
      }
      c /= barier.Points.size();
      for (int i = 0; i < barierIndex; i++) {
        debug.FillLine(1, c.x() - 2, c.y() + 2*i, c.x() + 2, c.y() + 2*i);
      }
      barierIndex++;
    }
  }

  for (int ind = 0; ind < mObjs.size(); ind++) {
    mCurrentObj = &mObjs[ind];

    int color = (mCurrentObj->Type == eGood)? 2: (mCurrentObj->Type == eExit || mCurrentObj->Type == eCasper)? 3: 1;
    debug.FillLine4(color, mCurrentObj->Place);
    if (!mCurrentObj->Track.isEmpty()) {
      auto itr = mCurrentObj->Track.begin();
      const ObjStep& step = *itr;
      QPoint p1 = step.Block * BlockSize();
      for (itr++; itr != mCurrentObj->Track.end(); itr++) {
        const ObjStep& step = *itr;
        QPoint p2 = step.Block * BlockSize();
        debug.FillLine(color, p1, p2);
        p1 = p2;
      }
      QPoint c = (mCurrentObj->Place.topLeft() + mCurrentObj->Place.bottomRight())/2;
      debug.FillDiamond(4, c.x(), c.y(), 1, 1);
    }

    auto itr2 = mObjHitMap.find(mCurrentObj->Id);
    if (itr2 != mObjHitMap.end()) {
      const QMap<int, ObjHit>& map = itr2.value();
      for (int i = 0; i < Bariers().size(); i++) {
        auto itr3 = map.find(i);
        if (itr3 != map.end()) {
          const ObjHit& hit = itr3.value();
          QPoint c = mCurrentObj->Place.topLeft();
          debug.FillLine4(hit.In? 2: 4, c.x(), c.y() + 2*i, c.x() + 4, c.y() + 2*i + 1);
        } else {
          QPoint c = mCurrentObj->Place.topLeft();
          debug.FillLine4(1, c.x(), c.y() + 2*i, c.x() + 4, c.y() + 2*i + 1);
        }
      }
    }
  }
}

void BlockObj::GetDbgObjEnergy(ImageSrc<uchar>& debug)
{
  GetDbgEnergyX(debug, mObjEnergy);
}

void BlockObj::GetDbgEnergyX(ImageSrc<uchar>& debug, const BlockSrc<int>& energyX)
{
  int maxValue = energyX.CalcMaxValue();

  if (maxValue <= 0) {
    debug.ClearSource();
    return;
  }

  for (int j = 0; j < Height(); j++) {
    byte* dbg = debug.Line(j);
    const int* energy = energyX.BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      byte val = *energy * 255 / maxValue;
      for (int i = 0; i < BlockSize(); i++) {
        *dbg = val;
        dbg++;
      }
      energy++;
    }
  }
}

void BlockObj::GetDbgMomentHyst(byte* data)
{
  int* datai = reinterpret_cast<int*>(data);

  int totalCount = mHystMoment.TotalCount();
  datai[0] = totalCount;
  memset(datai + 1, 0, sizeof(int) * 256);
  memcpy(datai + 1, mHystMoment.Data(), sizeof(int) * mHystMoment.Size());

//  int lowLevel = mHystMoment.GetValue(200) + 2;
//  int highLevel = mHystMoment.GetValue(990);

//  datai[1 + lowLevel] = totalCount/2;
//  datai[1 + lowLevel+1] = totalCount/2;
//  datai[1 + highLevel] = totalCount / 2;
  //  datai[1 + highLevel+1] = totalCount * 3/2;
}

//void BlockObj::GetDbgUinDetect(ImageSrc<uchar>& debug)
//{
//  debug.ZeroSource();
//
//  foreach (const PreObj& pre, mPreObjs) {
//    if (pre.Place.width() > kMinUinWidth && pre.Place.height() > UinPre::MinHeight()) {
//      int regionI = pre.Place.x();
//      int regionJ = pre.Place.y();
//      mUinPre->setRegionI(regionI);
//      mUinPre->setRegionJ(regionJ);
//      mUinPre->refDetectRegion().SetSource(const_cast<uchar*>(ImageData().Data(regionI, regionJ))
//                                           , pre.Place.width(), pre.Place.height(), ImageData().Stride());
//      mUinPre->Calc();
//
//      for (int j = 0; j < mUinPre->getDetectResult().Height(); j++) {
//        const uchar* src = mUinPre->getDetectResult().Line(j);
//        uchar* dbg = debug.Data(regionI, regionJ + j);
//
//        for (int i = 1; i < mUinPre->getDetectResult().Width(); i++) {
//          if (*src > 16) {
//            *dbg = 3;
//          } else if (*src > 8) {
//            *dbg = 2;
//          } else if (*src) {
//            *dbg = 1;
//          }
//
//          src++;
//          dbg++;
//        }
//      }
//    }
//  }
//}
//
//void BlockObj::GetDbgUinRegions(ImageSrc<uchar>& debug)
//{
//  debug.ZeroSource();
//
//  //foreach (const PreObj& pre, mPreObjs) {
//  //  if (pre.Place.width() > kMinUinWidth && pre.Place.height() > UinPre::MinHeight()) {
//  //    mDetectRegion.SetSource(const_cast<uchar*>(mLayerF.Data(pre.Place.x(), pre.Place.y())), pre.Place.width(), pre.Place.height(), ImageData().Stride());
//  //    for (int j = 0; j < mDetectRegion.Height(); j++) {
//  //      uchar* dbg = debug.Line(pre.Place.y() + j) + pre.Place.x();
//  //      const uchar* src = mDetectRegion.Line(j);
//  //      for (int i = 0; i < mDetectRegion.Width(); i++) {
//  //        *dbg = *src;
//
//  //        src++;
//  //        dbg++;
//  //      }
//  //    }
//  //  }
//  //}
//}


BlockObj::BlockObj(const AnalyticsB& _Analytics, const BlockSrc<int>& _DiffMark, const ImageSrc<uchar>& _LayerDiff)
  : BlockSceneAnalizer(_Analytics)
  , mSmoothBlocks(false), mInZoneTime(15000), mUseScreenshots(false)
  , mDiffMark(_DiffMark), mLayerDiff(_LayerDiff), mLayerBackground(nullptr), mDiffMarkSmooth(GetBlockScene()), mPreObjMark(GetBlockScene()), mMarkThreshold(1)
  , mObjEnergy(GetBlockScene())
  , mObjectId(0)
  , mNormalMoment(0)
  //, mUinPre(new UinPre())
{
}


