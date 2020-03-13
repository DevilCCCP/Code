#pragma once

#include <QList>
#include <QRect>
#include <QPointF>

#include <Lib/Include/Common.h>
#include <LibV/Include/Hyst.h>
#include <LibV/Include/Region.h>

#include "BlockSceneAnalizer.h"
#include "BlockScene.h"
#include "PointHst.h"
//#include "BlockBorder.h"


DefineClassS(BlockObj);
//DefineClassS(BlockBorder);
//DefineClassS(UinPre);

class BlockObj: public BlockSceneAnalizer
{
  struct PreObj {
    int       Id;
    QRect     Block;
    QRect     Place;
    int       Mass;
    int       Moment;
    bool      NearDoor;

    int       Used;
  };

  enum EObjType {
    eGrow,

    eGood,
    eCasper,

    eAway,
    //eLost,

    eExit,
    eObjIllegal
  };

  struct ObjStep {
    int    Mass;
    QPoint Block;
    qint64 Timestamp;
  };

  struct Obj {
    int                  Id;
    EObjType             Type;
    int                  Quality;

    int                  Mass;
    QRect                Place;
    QRect                Block;
//    Region<int>    Border;

    QPoint               First;
    bool                 GoodExit;
    QLinkedList<ObjStep> Track;
    int                  NormalMass;
    int                  MaxMass;

    QRect                Move;
    int                  PreIndex;
    int                  PreSize;
  };

  struct BarierInfo {
    QVector<QPoint> Points;
  };

  struct ObjHit {
    bool   In;
    qint64 Time;
  };

  struct ObjShot {
    Region<uchar>        BodyMark;
    QByteArray           BodyData;
    QRect                Place;
    QLinkedList<QPoint>  Trace;
    qint64               BodyTimestamp;
    qint64               TraceTimestamp;

    ObjShot(): BodyTimestamp(0), TraceTimestamp(0) { }
  };

  DefineStructS(Connect);
  struct Connect {
    int            Mass;
    QRect          Block;
    bool           NearDoor;

    Connect(): Mass(0), NearDoor(false) { }
  };

  bool                   mSmoothBlocks;
  int                    mInZoneTime;
  bool                   mUseScreenshots;

  const BlockSrc<int>&   mDiffMark;
  const ImageSrc<uchar>& mLayerDiff;
  const ImageSrc<uchar>* mLayerBackground;
  const ImageSrc<uchar>* mLayerFront;
  BlockSrc<int>          mDiffMarkSmooth;
  BlockSrc<int>          mPreObjMark;
  int                    mMarkThreshold;
  QVector<BarierInfo>    mBarierInfos;

  BlockSrc<int>          mObjEnergy;
  PROPERTY_GET_SET(int,  EnergyMs)

  QVector<int>           mObjIds;
  QVector<int>           mObjIds2;
  int                    mPreObjCount;
  QList<PreObj>          mPreObjs;
  PreObj*                mCurrentPre;

  int                    mObjectId;
  QList<Obj>             mObjs;
  Obj*                   mCurrentObj;
  typedef QMap<int, QMap<int, ConnectS> > ConnectMap;
  ConnectMap             mConnectMapObj;
  ConnectMap             mConnectMapPre;
  typedef QMap<int, QMap<int, ObjHit> > ObjHitMap;
  ObjHitMap              mObjHitMap;
  typedef QMap<int, ObjShot> ObjShotMap;
  ObjShotMap             mObjShotMap;


  int                    mReturnObjItr;

  // Scene stats
  Hyst                   mHystMoment;
  int                    mNormalMoment;
  int                    mFilterMoment;

  //// Uin
  //UinPreS                mUinPre;

public:
  const BlockSrc<int>& ObjEnergy() const { return mObjEnergy; }
  void SetSmoothBlocks(bool _SmoothBlocks);
  void SetThresholds(int _MarkThreshold);
  void SetUseScreenshots(int _UseScreenshots);
  void SetInZoneTime(int _InZoneTime);
  void SetLayerBackground(const ImageSrc<uchar>* _LayerBackground) { mLayerBackground = _LayerBackground; }
  void SetLayerFront(const ImageSrc<uchar>* _LayerFront)           { mLayerFront = _LayerFront; }
protected:
  const BlockSrc<int>& DiffMark() { return (mSmoothBlocks)? mDiffMarkSmooth: mDiffMark; }

public:
  void LoadSettings(const SettingsAS& settings);
  void SaveSettings(const SettingsAS& settings);
  void Init();

  void Analize();
private:
  void Prepare();
  void PreObjCreate();
  void ObjManage();
  //void UinManage();

public:
  qreal CalcStable();
  void ResetEnergy();

public:
  bool HavePreObj();
  bool RetrievePreObj(Object& object);
  bool HaveObj();
  bool RetrieveObj(Object& object);

private:
  void CreateMark(const BlockSrc<int>& diffMark);
  void CreateMarkSmooth();
  void PreConnect();
  void PreConstruct();
  void PreCalc();
  void PreReCalc(const QRect& preArea, int indexFirstPre, const QVector<int>& newMark);
  void PreFilter();
  void PreAdjust();

  void ObjPrepare();
  void ObjConnectPre();
  void ObjConnectAdjust();
  void ObjConnectFilterObj();
  void ObjConnectFilterPre();
  void ObjSharePre();
  void ObjDone();
  void ObjMove();
  void ObjCleanup();
  void ObjNew();
  void ObjDump();

  void ObjMoveBarriers(const QPoint& p1, const QPoint& p2);
  //void ObjMoveZone(const QPointF& p1, const QPointF& p2);
  //bool ObjInZone(int ii, int jj);
  int  ObjMoveBarierOne(const QPoint& p1, const QPoint& p2, const QVector<QPoint>& barier);
  void ObjHitBarrier(int index, bool in);
  void ObjMakeShot();
  void ObjUpdateShot();
  void ObjUpdateShotTrace(ObjShot* shot);
  void ObjUpdateShotBody(ObjShot* shot);
  void ObjApplyDetectors();

  void ObjHitBarier(const QMap<int, ObjHit>& hits);
  bool MakeShotImage(QByteArray& img);

  void ObjConfirm();
  void ObjConfirmGrow();
  void ObjConfirmGood();
  void ObjConfirmCasper();
  void ObjConfirmAway();
  void ObjConfirmExit(int iObj);
  bool ObjPromoteQuality(int qualityBase, int multiply = 1);
  bool ObjLostQuality(int qualityBase, int multiply = 1);

  void ObjCreateNewOne(const PreObj& pre);
  void ObjCreateClone();
  void ObjAddStep();
  void ObjAdjustSteps(const QLinkedList<ObjStep>& baseTrack, const QPoint& adjustVect);
  void ObjUseAllConnects(QMap<int, ConnectS>* connects);
  void ObjUseConnect(const ConnectS& connect, int iPre);
  void ObjDisonnect();

  void ObjFilterConnections(int iObj, QMap<int, ConnectS>* connects);

  bool ObjIsGood();
  int  ObjGetColor();
  const char* ObjTypeToString();

  //void UinDetect();
  //void UinAddDiffLrTop(int i, int j, int diffLeft, int diffRight);

  void BlockToPlace(const QRect& block, QRect& place);
  //void BlockToBorder(const QRect& block, int id, BlockBorder& blockBorder);

public:
  void GetDbgPreObjMark(ImageSrc<uchar>& debug);
  void GetDbgPreObj(ImageSrc<uchar>& debug);
  void GetDbgObj(ImageSrc<uchar>& debug);
  void GetDbgObjEnergy(ImageSrc<uchar>& debug);
  void GetDbgEnergyX(ImageSrc<uchar>& debug, const BlockSrc<int>& energyX);
  void GetDbgMomentHyst(byte* data);
  //void GetDbgUinDetect(ImageSrc<uchar>& debug);
  //void GetDbgUinRegions(ImageSrc<uchar>& debug);

public:
  BlockObj(const AnalyticsB& _Analytics, const BlockSrc<int>& _DiffMark, const ImageSrc<uchar>& _LayerDiff);
};

