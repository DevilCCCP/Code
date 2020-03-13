#pragma once

#include <QList>
#include <QRect>
#include <QPointF>

#include <Lib/Include/Common.h>

#include "BlockSceneAnalizer.h"
#include "BlockScene.h"
#include "Hyst.h"


DefineClassS(BlockStat);

class BlockStat: public BlockSceneAnalizer
{
  struct SectionInfo {
    int  Count;
    int  Value;
    int  InSection;
  };
  struct SectionStat {
    int   InTime;
    Hyst  CountHyst;
    QRect Dimentions;

    SectionStat(): InTime(0) { }
  };
  struct ZoneInfo {
    int   QueueThreshold;
    int   InQueueTime;
    int   CancelQueueTime;
    bool  InQueue;

    qreal QueueCount2Total;
    int   QueueTimeTotal;

    ZoneInfo()
      : QueueThreshold(-1), InQueueTime(0), CancelQueueTime(0), InQueue(false)
      , QueueCount2Total(0), QueueTimeTotal(0)
    { }
  };

  const BlockSrc<int>&     mDiffMark;
  bool                     mSmoothBlocks;
  int                      mSectionTime;
  int                      mQueueTime;
  int                      mQueueThreshold;

  QVector<SectionInfo>     mSectionInfo;
  QVector<SectionStat>     mSectionStat;
  QVector<ZoneInfo>        mZonesInfo;

  int                      mReturnSectionItr;

public:
  void LoadSettings(const SettingsAS& settings);
  void SaveSettings(const SettingsAS& settings);
  void Init(int sectionCount);
  void Calc();
  void UpdateStat();
  void UpdateStatOneZone(int zoneIndex);
  void SetSmoothBlocks(bool _SmoothBlocks);
  void SetTiming(int _SectionTime, int _QueueTime);
  void SetQueueThreshold(int _QueueThreshold);

private:
  void SwitchQueue(int zoneIndex, bool nowInQueueTime);
  void LogQueue(int zoneIndex);

public:
  bool HaveSection();
  bool RetrieveSection(Object& object);

public:
  void GetDbgSectionCount(ImageSrc<uchar>& debug);
  void GetDbgSectionValue(ImageSrc<uchar>& debug);
  void GetDbgSectionIn(ImageSrc<uchar>& debug);
  void GetDbgSectionHyst(byte* data);

private:
  QString GetZoneHash(const QList<const QPointF*>& pointList);
  bool CurrentSectionIsValid();
  int  CurrentSectionColor();

public:
  BlockStat(const AnalyticsB& _Analytics, const BlockSrc<int>& _DiffMark);
};
