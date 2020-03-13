#pragma once

#include <vector>
#include <QElapsedTimer>
#include <QVector>
#include <QPointF>

#include <LibV/Include/Rect.h>

#include "AnalyticsB.h"
#include "StatNormal.h"
#include "StatScene.h"
#include "Hyst.h"

//const int kDiffHystLen = 4;

struct Info {
  int DiffCount;
  int DiffSharpCount;
  int DiffP;
  int DiffM;
  int Sharp;
  int Object;
};

struct StatBase {
  enum EFlag {
    eNormal = 0,
    eIgnore = 1 << 0,
    eDoor = 1 << 1
  };

  int Flag;
};

struct Stat: public StatBase {
  StatNormal DiffCount;
  StatNormal DiffSharpCount;
  int        SharpD;
};

struct InfoDif {
  int DiffCount;
  int DiffPCount;
  int Object;
};

struct StatDif: public StatBase {
  int Layer;
  int Layer0TimeTrue;
  int Layer0TimeFalse;
  int Layer1TimeTrue;
  int Layer1TimeFalse;
};

struct ObjPre {
  int       Id;
  Rectangle Place;
  QPointF   CMass;
  int       Mass;
  QPointF   CSize;
  int       Size;
  QPointF   Radius;

  int       Used;
  bool      Constructed;
};

struct ObjLink {
  ObjPre* PreObject;
  int     Quality;
};

struct ObjInfo {
  Rectangle      Place;
  Rectangle      NewPlace;
  Point          Speed;
  ObjPre*        PreLinkGood;
  QList<ObjLink> PreLinkBad;
  int            Quality;
  int            Mode;
  int            Time;

  void Init(Rectangle _Place);
};

struct HitInf {
  qint64 Time;
  bool   In;
};

struct Obj2Info {
  enum EMode {
    eNormal,
    eCasper,
    eLost,
    eEnded,
    eDeleted = 0x10000,
    eModeIllegal
  };

  int               Id;
  QPointF           EmergeLocation;
  QPointF           Location;
  QPointF           NewLocation;
  QPointF           Speed;
  QPointF           Radius;
  int               PreLink;
  EMode             Mode;
  int               Quality;
  int               Time;
  QMap<int, HitInf> BariersHit;
};
const char* ObjModeToString(Obj2Info::EMode mode);

typedef QList<QPointF> PointList;
struct Barier {
  DetectorS Detector;
  PointList Points;
};

class MotionTracking: public AnalyticsA
{
  bool                 mMacroObjects;
  bool                 mDiffBackground;
  bool                 mDiffBackgroundDouble;
  int                  mMinObjectSizePer;

  bool                 mStandByEnabled;
  bool                 mStandByMode;
  Hyst                 mDiffAvgHyst;
  int                  mDiffNowCount;
  QElapsedTimer        mSyncSettings;

  std::vector<byte>    mBackground;
  std::vector<byte>    mDiffLayer0;
  std::vector<byte>    mDiffLayer1;
  std::vector<byte>    mDiffLayer2;
  std::vector<byte>    mBorderLayer;
  std::vector<byte>    mDebugLayer;
  qint64               mStartTimestamp;
  qint64               mCalc1Timestamp;
  qint64               mLastTimestamp;
  int                  mFrameMs;
  float                mTimeSec;
  int                  mSceneWidth;
  int                  mSceneHeight;
  int                  mSafeStride;
  int                  mSafeHeight;

  // Detectors
  QVector<PointList>   mDoors;
  QVector<Barier>      mBariers;

  // scene
  std::vector<Info>    mSceneInfo;
  std::vector<Stat>    mSceneStat;
  std::vector<InfoDif> mSceneDiffInfo;
  std::vector<StatDif> mSceneDiffStat;
  std::vector<int>     mSceneTmp;
  std::vector<int>     mSceneTmp2;

  // scene stat
  //int                  mLayer0DiffCountThreshold;
  //Hyst                 mLayer0DiffCountHyst;
  StatScene            mDiffStatScene;
  StatScene            mDiffSharpStatScene;
  int                  mMinObjectSize;
  Hyst                 mObjectSpeedHyst;
  float                mObjectMaxSpeed;
  float                mObjectMaxAccel;
  Hyst                 mObjectFitHyst;
  float                mObjectFitMin;
  Hyst                 mDebugHyst;

  // pre obj
  StatNormalAuto       mPreCountStat;
  std::vector<int>     mObjIds;
  std::vector<ObjPre>  mPreObjects;
  int                  mPreObjectsCount;

  // obj
  QList<ObjInfo>       mObjectInfo;
  QList<Obj2Info>      mObject2Info;
  int                  mObjectNextId;
  int                  mObjectCounter;

  // total scene
  int                  mTotalObjectsLast;
  int                  mTotalObjects;

  // source image
  const byte*          mImageData;

protected:
  /*override */virtual void InitSettings(const SettingsAS& _Settings) Q_DECL_OVERRIDE;
  /*override */virtual void InitDetectors(const QList<DetectorS>& _Detectors) Q_DECL_OVERRIDE;
  /*override */virtual bool InitImageParameters(int width, int height, int stride) Q_DECL_OVERRIDE;

  /*override */virtual void AnalizePrepare(const byte* imageData) Q_DECL_OVERRIDE;
  /*override */virtual void AnalizeInit() Q_DECL_OVERRIDE;
  /*override */virtual bool AnalizeStandBy() Q_DECL_OVERRIDE;
  /*override */virtual bool AnalizeNormal() Q_DECL_OVERRIDE;

  /*override */virtual int GetDebugFrameCount() Q_DECL_OVERRIDE;
  /*override */virtual bool GetDebugFrame(const int index, QString& text, EImageType& imageType, byte* data, bool& save) Q_DECL_OVERRIDE;

public:
  /*override */virtual bool HaveNextObject() Q_DECL_OVERRIDE;
  /*override */virtual bool RetrieveNextObject(Object& object) Q_DECL_OVERRIDE;

private:
  bool AnalizeAll();
  bool LoadDetectorPoints(const DetectorS& detector, PointList& points);

  bool TrySwithStandByMode();
  void SyncSettings();

  //
  // Scene
  //
  void InitBackground();
  void InitDiffLayer0();
  void InitStat();
  void InitStatDiff();
  void UpdateStatPeriod1();
  void SceneInit();
  void SceneInitDiff();
  void CalcFront();
  void CalcLayer2();
  void UpdateStat();
  void UpdateStatDiff();

  //
  // Pre Objects
  //
  void PreObjectsCreateNormal();
  void PreObjectsMark();
  void PreObjectsMarkSmooth();
  void PreObjectsMarkSmooth2();
  bool PreMarkValid();
  void PreObjectsConnect(int threshold);
  void PreObjectsConstruct();
  void PreObjectsConstructDiff();

  void PreObjectsCreateMacro();
  void PreObjectsMarkMacro();
  void PreObjectsConstructMacro();

  void PreObjectsCreateDiff();
  void PreObjectsMarkDiff();

  //
  // Objects
  //
  void ObjectsManage();
  void ObjectsLinkPre();
  void ObjectsDevideMultiPre();
  void ObjectsMoveAll();
  void ObjectsUpdateSpeedStat();
  float Object2FitPoint(const Obj2Info* object, int ii, int jj);
  void Object2LinkOne(Obj2Info* object);
  void Object2CreateOne(ObjPre* preObject);
  void Object2ManageOne(Obj2Info* object);
  void MoveObject(Obj2Info* object);
  int IntersectBarier(const QPointF& pp1, const QPointF& pp2, const PointList& barier);

  //
  // Objects old
  //
  void ObjectVerify();
  void ObjectLinkOne(ObjInfo& object);
  bool ObjectVerifyOne(ObjInfo& object);
  void ObjectModifyGoodFit(ObjInfo& object);
  void ObjectModifyBadFit(ObjInfo& object);
  void ObjectModifyFinal(ObjInfo& object, int deltaWidth = 0, int deltaHeight = 0);
//  bool ObjectFitPreGood(const Rectangle& newPlace, const Rectangle& testPlace, const Point& moveThreshold, int sizeThreshold);
  int ObjectFitPreCalc(const Rectangle& newPlace, const Rectangle& testPlace, const Point& moveThreshold, int sizeThreshold);
  int ObjectCalcFit(ObjPre& objectPre, Rectangle &place, int distance);
  bool ObjectModifyOne(ObjInfo& object, Rectangle &newPlace, const Rectangle &fitPlace, const Rectangle &fitCount);

  void GetDbgBackground(byte* data);
  void GetDbgDiffLayer0(byte* data);
  void GetDbgDiffLayerX(byte* data);

  void GetDbgDiff(byte* data);
  void GetDbgDiffLayer0Diff(byte* data);
  void GetDbgDiffFormula(byte* data);
  void GetDbgDiffP(byte* data);
  void GetDbgDiffMicroHyst(byte* data);
  void GetDbgDiffBackground(byte* data);

  void GetDbgBlockDiffHyst(byte* data);
  void GetDbgBlockDiffOneHyst(byte* data, int blockI, int blockJ);
  void GetDbgBlockDiff(byte* data);
  void GetDbgBlockDiffMidStat(byte* data);
  void GetDbgBlockDiffMaxStat(byte* data);

  void GetDbgSharpHyst(byte* data);
  void GetDbgBlockSharp(byte* data);

  void GetDbgBlockDiffSharp(byte* data);
  void GetDbgBlockDiffSharpMidStat(byte* data);
  void GetDbgBlockDiffSharpMaxStat(byte* data);

  void GetDbgInfoObjects(byte* data);
  void GetDbgInfoObjectsHyst(byte* data);

  void GetDbgDiffInfoObjects(byte* data);

  void GetDbgInfoSharp(byte* data);
  void GetDbgStatSharp(byte* data);
  void GetDbgInfo(byte* data, size_t disp, size_t dispStat, int threshold);
  void GetDbgStat(byte* data, size_t disp, int threshold);

//  void GetDbgDiffM(byte* data);
  void GetDbgSharp(byte* data);
  void GetDbgSharpDiff(byte* data);
  void GetDbgFront(byte* data);

  void GetDbgDiffLayer1(byte* data);
  void GetDbgDiffLayer2(byte* data);
  void GetDbgDiffCurrentLayer(byte* data);
  void GetDbgDiffLayer0Time(byte* data);
  void GetDbgDiffLayer1Time(byte* data);
  void GetDbgObjDetect(byte* data);

  void GetDbgIgnore(byte* data);
  void GetDbgHyst(byte* data, const Hyst& hyst, int percentDraw = -1);

  bool IsPointInDoor(int ii, int jj);
  QPointF PointPercentToScreen(const QPointF& pointPercent);
  QPointF PointScreenToPercent(const QPointF& pointScreen);
  bool IsNearBorder(const Obj2Info& object);

public:
  MotionTracking();
  /*override */virtual ~MotionTracking();
};

