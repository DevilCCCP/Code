#pragma once

#include <QVector>
#include <QImage>

#include <LibV/Include/Hyst.h>
#include <LibV/Va/AnalyticsA.h>

#include "BlockScene.h"


struct BlockInfo {
  enum EFlag {
    eNormal    = 0,
    eIgnore    = 1 << 16,
    eDoor      = 1 << 17,
    eStat      = 1 << 18,
    eZone      = 1 << 19,
    eUin       = 1 << 20,

    eValueMask = 0xffff
  };

  int Flag;
};

DefineClassS(QImage);

// Block based analytics
class AnalyticsB: public AnalyticsA
{
public:
  typedef QList<QPointF> PointList;
  struct Barier {
    DetectorS Detector;
    PointList Points;
  };
  struct InZone {
    DetectorS Detector;
  };
  struct StatZone {
    DetectorS Detector;
    PointList Points;
    int       SectionFirst;
    int       SectionLast;
  };

private:
  PROPERTY_GET(int,        BlockSize)
  PROPERTY_GET(BlockScene, BlockScene)
  bool                     mOpenCamera;

  int                    mDiffCount;
  int                    mDiffCountReported;
  Hyst                   mDiffAvgHyst;

  // Detectors
  QVector<PointList>     mDoors;
  QVector<PointList>     mLineDoors;
  QVector<PointList>     mUin;
  QVector<PointList>     mIgnore;
  QVector<PointList>     mZone;
  PROPERTY_GET(bool,              UsingUins)
  PROPERTY_GET(QVector<Barier>,   Bariers)
  PROPERTY_GET(QVector<InZone>,   InZones)
  PROPERTY_GET(QVector<StatZone>, StatLineZones)
  PROPERTY_GET(int,               SectionCount)

  PROPERTY_GET(ImageSrc<uchar>,     ImageData)
  PROPERTY_GET(BlockSrc<BlockInfo>, BlockInfo)

  int                    mStableTimeMs;
  qreal                  mStableValue;

  QImageS                mLogoImg;
  ImageSrc<uchar>        mDebugData;

protected:
  void SetBlockSize(int _BlockSize);

protected:
  /*override */virtual void InitSettings(const SettingsAS& _Settings) Q_DECL_OVERRIDE;
  /*override */virtual void InitDetectors(const QList<DetectorS>& _Detectors) Q_DECL_OVERRIDE;
  /*override */virtual bool InitImageParameters(int width, int height, int stride) Q_DECL_OVERRIDE;

  /*override */virtual void AnalizePrepare(const byte* imageData) Q_DECL_OVERRIDE;
  /*override */virtual bool AnalizeStandBy() Q_DECL_OVERRIDE;
  /*override */virtual bool AnalizeNormal() Q_DECL_OVERRIDE;
  /*override */virtual void SaveVariables(const SettingsAS& settings) Q_DECL_OVERRIDE;

//  /*override */virtual int GetDebugFrameCount() Q_DECL_OVERRIDE;
//  /*override */virtual bool GetDebugFrame(const int index, QString& text, EImageType& imageType, byte* data, bool& save) Q_DECL_OVERRIDE;

public:
//  /*override */virtual bool HaveNextObject() Q_DECL_OVERRIDE;
//  /*override */virtual bool RetrieveNextObject(Object& object) Q_DECL_OVERRIDE;
//  /*override */virtual void Finish() Q_DECL_OVERRIDE;

protected:
  /*new */virtual void ExtraSettings(const SettingsAS& _Settings) { Q_UNUSED(_Settings); }

  /*new */virtual void AnalizeFront() = 0;
  /*new */virtual void AnalizeScene() = 0;
  /*new */virtual bool NeedStable();
  /*new */virtual qreal CalcStable();

  /*new */virtual void LoadSettings(const SettingsAS& settings) { Q_UNUSED(settings); }
  /*new */virtual void SaveSettings(const SettingsAS& settings) { Q_UNUSED(settings); }
  /*new */virtual int GetDiffCount() = 0;

public:
  bool SerializeBlock(const SettingsAS& settings, const char* name, const BlockSrc<int>& block, int timeMs) const;
  bool DeserializeBlock(const SettingsAS& settings, const char* name, BlockSrc<int>& block, int& timeMs) const;
  void CompactBlock(const BlockSrc<int>& block, QByteArray& data, int& multiply) const;
  void DecompactBlock(BlockSrc<int>& block, const QByteArray& data, int multiply) const;

protected:
  void MakeStatImage(const BlockSrc<int>& block, QByteArray& data);

  ImageSrc<uchar>& DebugData(uchar* data);
  void GetDbgIgnore(ImageSrc<uchar>& debug);
  void GetDbgDoor(ImageSrc<uchar>& debug);
  void GetDbgUin(ImageSrc<uchar>& debug);
  void GetDbgZone(ImageSrc<uchar>& debug);
  void GetDbgStatSection(ImageSrc<uchar>& debug);

private:
  bool RestoreSettings();

  void AnalizeStable();

  void InitBlockInfo();
  void InitLineZoneInfo(const PointList& points, BlockInfo::EFlag flag);

  bool InitStandByCalc();
  bool TrySwitchStandByOn();
  bool TrySwitchStandByOff();

  bool LoadDetectorPoints(const DetectorS& detector, PointList& points);
  bool IsPointInDoor(int ii, int jj);
  bool IsPointInUin(int ii, int jj);
  int GetPointZone(int ii, int jj);
  bool IsPointInIgnore(int ii, int jj);
  int IsPointInArea(const QVector<PointList>& area, int ii, int jj);
  void MarkPoint(int ii, int jj, BlockInfo::EFlag flag);

  static qreal GetVectMult(const QPointF& v1, const QPointF& v2);
  static qreal GetScalarMult(const QPointF& v1, const QPointF& v2);
  static qreal GetAngle(const QPointF& v1, const QPointF& v2);
  static qreal GetVectSize(const QPointF& v);

public:
  QPointF PointToScreen(const QPointF& pointPercent) const;
  QPointF ScreenToPoint(const QPointF& pointScreen) const;

private:
  void AddZone(QVector<PointList>& zoneArray, const PointList& points);
  void AddLineZone(QVector<PointList>& zoneArray, const PointList& points);

  bool GetIntersectPoint(const QPointF& a1, const QPointF& a2, const QPointF& b1, const QPointF& b2, qreal* t = nullptr);

public:
  AnalyticsB(int _BlockSize);
  /*override */~AnalyticsB();
};
