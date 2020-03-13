#include <Lib/Log/Log.h>

#include "BlockStat.h"


const int kDenumInSection = 4;
const int kDenumInQueue = 2;
const int kLogQueuePeriodMs = 30 * 1000;

void BlockStat::LoadSettings(const SettingsAS& settings)
{
  mSectionStat.clear();
  for (int zoneIndex = 0; zoneIndex < StatLineZones().size(); zoneIndex++) {
    const AnalyticsB::StatZone* zone = &StatLineZones().at(zoneIndex);
    if (zone->Points.size() < 3) {
      continue;
    }

    const QPointF* left = &zone->Points[0];
    const QPointF* right = &zone->Points[0];
    for (int i = 2; i < zone->Points.size(); i += 2) {
      const QPointF* leftEnd = &zone->Points[i - 1];
      const QPointF* rightEnd = &zone->Points[i];

      QString hash = GetZoneHash(QList<const QPointF*>() << left << right << leftEnd << rightEnd);
      mSectionStat.resize(mSectionStat.size() + 1);
      mSectionStat.last().CountHyst.Deserialize(settings->GetValue(hash, QString("")).toString(), 128);
      QRect* dim = &mSectionStat.last().Dimentions;
      dim->setLeft( Width() * qMin(qMin(left->x(), right->x()), qMin(leftEnd->x(), rightEnd->x())) );
      dim->setRight( Width() * qMax(qMax(left->x(), right->x()), qMax(leftEnd->x(), rightEnd->x())) );
      dim->setTop( Height() * qMin(qMin(left->y(), right->y()), qMin(leftEnd->y(), rightEnd->y())) );
      dim->setBottom( Height() * qMax(qMax(left->y(), right->y()), qMax(leftEnd->y(), rightEnd->y())) );

      left = leftEnd;
      right = rightEnd;
    }
  }
}

void BlockStat::SaveSettings(const SettingsAS& settings)
{
  if (mSectionInfo.isEmpty()) {
    return;
  }
  int sectionIndex = 0;
  for (int zoneIndex = 0; zoneIndex < StatLineZones().size(); zoneIndex++) {
    const AnalyticsB::StatZone* zone = &StatLineZones().at(zoneIndex);
    if (zone->Points.size() < 3) {
      continue;
    }

    const QPointF* left = &zone->Points[0];
    const QPointF* right = &zone->Points[0];
    for (int i = 2; i < zone->Points.size(); i += 2) {
      const QPointF* leftEnd = &zone->Points[i - 1];
      const QPointF* rightEnd = &zone->Points[i];

      QString hash = GetZoneHash(QList<const QPointF*>() << left << right << leftEnd << rightEnd);
      mSectionStat[sectionIndex].CountHyst.Normalize(14*24*60*60*30);
      settings->SetValue(hash, mSectionStat[sectionIndex].CountHyst.Serialize(128));
      sectionIndex++;

      left = leftEnd;
      right = rightEnd;
    }
  }
}

void BlockStat::Init(int sectionCount)
{
  Q_ASSERT(mSectionStat.size() == sectionCount);
  mSectionInfo.resize(sectionCount);
  mSectionStat.resize(sectionCount);
}

void BlockStat::Calc()
{
  memset(mSectionInfo.data(), 0, sizeof(SectionInfo) * mSectionInfo.size());

  for (int jj = 0; jj < BlockHeight(); jj++) {
    const BlockInfo* info = GetBlockInfo().Line(jj);
    const int* mark = mDiffMark.Line(jj);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      if ((info->Flag & BlockInfo::eStat) != 0) {
        int section = (info->Flag & BlockInfo::eValueMask);
        mSectionInfo[section].Count += *mark;
      }

      info++;
      mark++;
    }
  }
}

void BlockStat::UpdateStat()
{
  mZonesInfo.resize(StatLineZones().size());
  for (int zoneIndex = 0; zoneIndex < mZonesInfo.size(); zoneIndex++) {
    UpdateStatOneZone(zoneIndex);
  }

  mReturnSectionItr = 0;
}

void BlockStat::UpdateStatOneZone(int zoneIndex)
{
  const AnalyticsB::StatZone* zone = &StatLineZones()[zoneIndex];
  int countEmpty = 0;
  int countIn = 0;
  for (int i = zone->SectionFirst; i < zone->SectionLast; i++) {
    SectionInfo* info = &mSectionInfo[i];
    SectionStat* stat = &mSectionStat[i];

    stat->CountHyst.Inc(qMin(255, info->Count));
    int threshold = (stat->CountHyst.TotalCount() > 200)? stat->CountHyst.GetValue(900) / 2: 4;
    threshold = qMax(2, threshold);
    info->Value = (info->Count >= threshold)? ((info->Count >= 2*threshold)? 2: 1): 0;

    countIn += info->Value;
    if (info->Value == 2) {
      stat->InTime += FrameMs();
      if (stat->InTime > 2 * mSectionTime) {
        stat->InTime = 2 * mSectionTime;
      }
    } else if (info->Value == 0) {
      stat->InTime -= kDenumInSection * FrameMs();
      if (stat->InTime < 0) {
        stat->InTime = 0;
      }
    }
    info->InSection = (stat->InTime > mSectionTime)? 1: 0;
    if (!info->InSection) {
      countEmpty++;
    }
  }

  ZoneInfo* zoneInfo = &mZonesInfo[zoneIndex];
  if (zoneInfo->QueueThreshold < 0) {
    int zoneCount = zone->SectionLast - zone->SectionFirst;
    if (mQueueThreshold >= zoneCount) {
      int newThreshold = zoneCount - 1;
      Log.Warning(QString("Queue threshold is too large, and will be corrected (tr: %1(->%2), size: %3)")
                  .arg(mQueueThreshold).arg(newThreshold).arg(zoneCount));
      zoneInfo->QueueThreshold = newThreshold;
    } else {
      zoneInfo->QueueThreshold = mQueueThreshold;
    }
  }

  zoneInfo->QueueCount2Total += countIn * FrameMs();
  zoneInfo->QueueTimeTotal += FrameMs();
  if (zoneInfo->QueueTimeTotal > kLogQueuePeriodMs) {
    LogQueue(zoneIndex);
  }
  bool nowInQueue = countEmpty <= zoneInfo->QueueThreshold;
  if (zoneInfo->InQueue) {
    if (nowInQueue) {
      zoneInfo->CancelQueueTime = 0;
    } else {
      zoneInfo->CancelQueueTime += FrameMs();
      if (zoneInfo->CancelQueueTime >= mQueueTime) {
        SwitchQueue(zoneIndex, false);
        zoneInfo->CancelQueueTime = 0;
      }
    }
  } else {
    if (nowInQueue) {
      zoneInfo->InQueueTime += FrameMs();
    } else {
      zoneInfo->InQueueTime -= kDenumInQueue*FrameMs();
      if (zoneInfo->InQueueTime < 0) {
        zoneInfo->InQueueTime = 0;
      }
    }
    if (zoneInfo->InQueueTime >= mQueueTime) {
      SwitchQueue(zoneIndex, true);
      zoneInfo->InQueueTime = 0;
    }
  }
}

void BlockStat::SetSmoothBlocks(bool _SmoothBlocks)
{
  mSmoothBlocks = _SmoothBlocks;
}

void BlockStat::SetTiming(int _SectionTime, int _QueueTime)
{
  mSectionTime = _SectionTime;
  mQueueTime = _QueueTime;
}

void BlockStat::SetQueueThreshold(int _QueueThreshold)
{
  mQueueThreshold = _QueueThreshold;
}

void BlockStat::SwitchQueue(int zoneIndex, bool nowInQueueTime)
{
  Log.Info(QString("Zone [%1]: %2").arg(zoneIndex).arg(nowInQueueTime? "queue start": "queue ends"));
  const AnalyticsB::StatZone* zone = &StatLineZones()[zoneIndex];
  zone->Detector->Hit(CurTimestamp() - mQueueTime, nowInQueueTime? "qstart": "qend");
  ZoneInfo* zoneInfo = &mZonesInfo[zoneIndex];
  zoneInfo->InQueue = nowInQueueTime;
}

void BlockStat::LogQueue(int zoneIndex)
{
  ZoneInfo* zoneInfo = &mZonesInfo[zoneIndex];
  qreal midQueue = 0.5 * zoneInfo->QueueCount2Total / zoneInfo->QueueTimeTotal;
  Log.Trace(QString("Zone [%1]: queue %2").arg(zoneIndex).arg(midQueue, 0, 'f', 2));
  const AnalyticsB::StatZone* zone = &StatLineZones()[zoneIndex];
  zone->Detector->Stat(CurTimestamp() - kLogQueuePeriodMs / 2, "queue", 0.5 * zoneInfo->QueueCount2Total, zoneInfo->QueueTimeTotal);
  zoneInfo->QueueCount2Total = 0;
  zoneInfo->QueueTimeTotal = 0;
}

bool BlockStat::HaveSection()
{
  for (; mReturnSectionItr < mSectionInfo.size(); mReturnSectionItr++) {
    if (CurrentSectionIsValid()) {
      return true;
    }
  }
  return false;
}

bool BlockStat::RetrieveSection(Object& object)
{
  for (; mReturnSectionItr < mSectionInfo.size(); mReturnSectionItr++) {
    if (CurrentSectionIsValid()) {
      object.Id = mReturnSectionItr;
      object.Color = CurrentSectionColor();
      object.Dimention.Left = mSectionStat[mReturnSectionItr].Dimentions.left();
      object.Dimention.Right = mSectionStat[mReturnSectionItr].Dimentions.right();
      object.Dimention.Top = mSectionStat[mReturnSectionItr].Dimentions.top();
      object.Dimention.Bottom = mSectionStat[mReturnSectionItr].Dimentions.bottom();

      mReturnSectionItr++;
      return true;
    }
  }
  return false;
}

void BlockStat::GetDbgSectionCount(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    byte* dbg = debug.Line(j);
    const BlockInfo* info = GetBlockInfo().BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      byte val = 0;
      if ((info->Flag & BlockInfo::eStat) != 0) {
        int section = (info->Flag & BlockInfo::eValueMask);
        val = qMin(255, mSectionInfo[section].Count);
      }
      for (int i = 0; i < BlockSize(); i++) {
        *dbg = val;
        dbg++;
      }
      info++;
    }
  }
}

void BlockStat::GetDbgSectionValue(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    byte* dbg = debug.Line(j);
    const BlockInfo* info = GetBlockInfo().BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      byte val = 0;
      if ((info->Flag & BlockInfo::eStat) != 0) {
        int section = (info->Flag & BlockInfo::eValueMask);
        val = mSectionInfo[section].Value + 1;
      }
      for (int i = 0; i < BlockSize(); i++) {
        *dbg = val;
        dbg++;
      }
      info++;
    }
  }
}

void BlockStat::GetDbgSectionIn(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    byte* dbg = debug.Line(j);
    const BlockInfo* info = GetBlockInfo().BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      byte val = 0;
      if ((info->Flag & BlockInfo::eStat) != 0) {
        int section = (info->Flag & BlockInfo::eValueMask);
        val = mSectionInfo[section].InSection + 1;
      }
      for (int i = 0; i < BlockSize(); i++) {
        *dbg = val;
        dbg++;
      }
      info++;
    }
  }
}

void BlockStat::GetDbgSectionHyst(byte* data)
{
  int* datai = reinterpret_cast<int*>(data);
  Hyst hyst;
  for (int j = 0; j <= 255; j++) {
    for (int i = 0; i < mSectionStat.size(); i++) {
      hyst.Data()[j] += mSectionStat[i].CountHyst.Data()[j];
    }
  }

  int totalCount = hyst.TotalCount();
  datai[0] = totalCount;
  memcpy(datai + 1, hyst.Data(), sizeof(int) * hyst.Size());

//  int lowLevel = mDiffSumHyst.GetValue(500) + 2;
//  int highLevel = mDiffSumHyst.GetValue(990);

//  datai[1 + lowLevel] = totalCount / 2;
//  datai[1 + lowLevel+1] = totalCount * 3/2;
//  datai[1 + highLevel] = totalCount / 2;
  //  datai[1 + highLevel+1] = totalCount * 3/2;
}

QString BlockStat::GetZoneHash(const QList<const QPointF*>& pointList)
{
  int hashValue = 0;
  for (auto itr = pointList.constBegin(); itr != pointList.constEnd(); itr++) {
    const QPointF* p = *itr;
    hashValue += 128*1024*1024 * p->x() + 8*1024 * p->y() + p->x() * p->y();
  }
  return QString("zone_") + QString::number(hashValue);
}

bool BlockStat::CurrentSectionIsValid()
{
  return mSectionInfo[mReturnSectionItr].Value > 0 || mSectionInfo[mReturnSectionItr].InSection > 0;
}

int BlockStat::CurrentSectionColor()
{
  /* int ind = Color / 1000; int value = qMin(100, Color % (1000*ind)); ind: 0 - White, 1 - Green, 2 - Red, 3 - Blue (gObjectColors)*/
  bool current = mSectionInfo[mReturnSectionItr].Value > 0;
  bool medium = mSectionInfo[mReturnSectionItr].InSection > 0;
  if (current && medium) {
    return 1000 + 100;
  } else if (medium) {
    return 3000 + mSectionStat[mReturnSectionItr].InTime * 50 / mSectionTime;
  } else if (current) {
    return 0 + mSectionStat[mReturnSectionItr].InTime * 50 / mSectionTime;
  } else {
    return 0;
  }
}


BlockStat::BlockStat(const AnalyticsB& _Analytics, const BlockSrc<int>& _DiffMark)
  : BlockSceneAnalizer(_Analytics)
  , mDiffMark(_DiffMark), mSmoothBlocks(false), mSectionTime(5000), mQueueTime(2000), mQueueThreshold(1)
{
}

