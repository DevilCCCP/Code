#pragma once

#include <LibV/Include/Region.h>

#include "AnalyticsB.h"
#include "BlockScene.h"


class BlockSceneAnalizer
{
  const AnalyticsB& mAnalytics;

protected:
  const BlockScene& GetBlockScene() const { return mAnalytics.getBlockScene(); }

  int BlockSize()   const { return mAnalytics.getBlockScene().BlockSize(); }
  int Width()       const { return mAnalytics.getBlockScene().Width(); }
  int Height()      const { return mAnalytics.getBlockScene().Height(); }
  int BlockWidth()  const { return mAnalytics.getBlockScene().BlockWidth(); }
  int BlockHeight() const { return mAnalytics.getBlockScene().BlockHeight(); }
  int SrcWidth()    const { return mAnalytics.getBlockScene().SrcWidth(); }
  int SrcHeight()   const { return mAnalytics.getBlockScene().SrcHeight(); }

  int    CurrentFrame() const { return mAnalytics.getCurrentFrame(); }
  qint64 CurrentMs()    const { return mAnalytics.getCurrentMs(); }
  qint64 CurTimestamp() const { return mAnalytics.getLastTimestamp(); }
  int FrameMs()    const { return mAnalytics.getFrameMs(); }
  float FrameSec() const { return mAnalytics.getFrameSec(); }

  const ImageSrc<uchar>& ImageData() { return mAnalytics.getImageData(); }
  const BlockSrc<BlockInfo>& GetBlockInfo() { return mAnalytics.getBlockInfo(); }

//  bool  UsingUins() const { return mAnalytics.getUsingUins(); }
  const QVector<AnalyticsB::Barier>&   Bariers()       const { return mAnalytics.getBariers(); }
  const QVector<AnalyticsB::StatZone>& StatLineZones() const { return mAnalytics.getStatLineZones(); }
  const QVector<AnalyticsB::InZone>&   InZones()       const { return mAnalytics.getInZones(); }

  QPoint PointPercentToScene(const QPointF& pointPercent) const
  { return QPoint(pointPercent.x() * Width(), pointPercent.y() * Height()); }
  QPoint PointPercentToBlock(const QPointF& pointPercent) const
  { return QPoint(pointPercent.x() * BlockWidth(), pointPercent.y() * BlockHeight()); }

  bool SerializeBlock(const SettingsAS& settings, const char* name, const BlockSrc<int>& block, int timeMs) const
  { return mAnalytics.SerializeBlock(settings, name, block, timeMs); }
  bool DeserializeBlock(const SettingsAS& settings, const char* name, BlockSrc<int>& block, int& timeMs) const
  { return mAnalytics.DeserializeBlock(settings, name, block, timeMs); }

  bool CreateSnapshot(const Region<uchar>& region, QByteArray& data) { return mAnalytics.CreateSnapshot(region, data); }

public:
  BlockSceneAnalizer(const AnalyticsB& _Analytics)
    : mAnalytics(_Analytics)
  { }
};

