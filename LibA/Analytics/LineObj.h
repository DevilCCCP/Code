#pragma once

#include <QMap>

#include <Lib/Include/Common.h>
#include <LibV/Include/Rect.h>
#include <LibV/Va/Region.h>

#include "BlockSceneAnalizer.h"
#include "Hyst.h"


static const int kPoinsCount = 4;

struct InfoPoint {
  int X;
  int Y;

  inline const int& x() const { return X; }
  inline const int& y() const { return Y; }
  inline int& rx() { return X; }
  inline int& ry() { return Y; }
  inline void setX(int _X) { X = _X; }
  inline void setY(int _Y) { Y = _Y; }
  int	manhattanLength() const { return qAbs(X) + qAbs(Y); }
  int manhattanLengthTo(const InfoPoint& other) const { return qAbs(X - other.X) + qAbs(Y - other.Y); }

  bool operator==(const InfoPoint& other) const { return X == other.X && Y == other.Y; }
  InfoPoint operator+(const InfoPoint& other) const { InfoPoint result; result.X = X + other.X; result.Y = Y + other.Y; return result; }
  InfoPoint operator-(const InfoPoint& other) const { InfoPoint result; result.X = X - other.X; result.Y = Y - other.Y; return result; }
};

enum Type1Position {
  eTopPoint    = 0,
  eRightPoint  = 1,
  eBottomPoint = 2,
  eLeftPoint   = 3,
};

struct Type1Info {
  union {
    struct {
      InfoPoint Top;
  //    InfoPoint TopRight;
      InfoPoint Right;
  //    InfoPoint BottomRight;
      InfoPoint Bottom;
  //    InfoPoint BottomLeft;
      InfoPoint Left;
  //    InfoPoint TopLeft;
  //    InfoPoint Center;
    };
    InfoPoint Points[kPoinsCount];
  };
};

class LineObj: public BlockSceneAnalizer
{
  const ImageSrc<uchar>& mLayerF;
  ImageSrc<uchar>        mLayerMark;
  ImageSrc<int>          mPreObjMark;
  ImageSrc<int>          mPreObjMarkSolid;

  bool                   mMarkConnect;

  QVector<int>           mObjIds;
  int                    mPreObjCount;

  struct PreObj { // ctor with memset(0)
    Rectangle Dimentions;
    int       Count;
    int       Length0;
    int       Length1;
    bool      Taken;

    Type1Info Info;
  };

  enum ObjType {
    eTest,
    eNormal,
    eCasper,
    eOtIllegal
  };

  struct Obj {
    ObjType   Type;
    int       Stage;
    int       Mass;
    Type1Info Info;
  };

  struct ObjRef {
    QList<int> PointsRef[kPoinsCount];
  };

  QVector<PreObj>        mPreObj;
  QVector<int>           mPreObj1;
  QVector<int>           mPreObj1Map;
  int                    mCurrentPreIndex;
  PreObj*                mCurrentPre;
  int                    mCurrentPreLength;
  int                    mMinObjLength;
  Hyst                   mPreMomentHyst;
  Hyst                   mPreMomentHyst2;

  Region<int>            mLineTmp;
  QPoint                 mTmpBasePoint;

  QList<Obj>             mObj;
  int                    mCurrentObjIndex;
  Obj*                   mCurrentObj;
  QMap<int, ObjRef>      mPreObjPointRef;
  QMap<int, ObjRef>      mObjPrePointRef;

  int                    mReturnObjItr;

public:
  void Init();

  void LoadSettings(const SettingsAS& settings);
  void SaveSettings(const SettingsAS& settings);

  void Analize();
  qreal CalcStable();

  bool HaveObj();
  bool RetrieveObj(Object& object);

private:
  void Prepare();
  void MakeMarkSmooth();
  void MakeMarkConnect();
  void MakeMarkMedian();
  void ConnectPreObj(const ImageSrc<uchar>& layerMark);
  void MakePreObj();
  void Filter1PreObj();
  void Mark1PreObj();
  void Mark1PreObjOne();
  void Mark1NewSolid(int iCenter, int jCenter);
  void FillPreObj();
  void FillPreObjOne();
  void BorderPreObj();
  void BorderPreObjOne();
  void MakePreType1();
  void MakePreType1One();
  void MakePreType1OneTop(InfoPoint& cmass, int minMass);
  void MakePreType1OneBottom(InfoPoint& cmass, int minMass);
  void MakePreType1OneLeft(InfoPoint& cmass, int minMass);
  void MakePreType1OneRight(InfoPoint& cmass, int minMass);
  void AddPreType1OneHorzLine(int j, InfoPoint& cmass, int& count);
  void AddPreType1OneVertLine(int i, InfoPoint& cmass, int& count);

  void ConnectLines();
  void ConnectLineOne();
  void FindMidPoint(QPoint& midPoint);
  void MarkPoints(const QPoint& startPoint, QPoint& endPoint);
  void MarkPointsLength(QLinkedList<QPoint>& currentPoints);
  void MarkObjPointsLength(QLinkedList<QPoint>& currentPoints);
  void MarkPath(QLinkedList<QPoint>* points);
  void DumpMark();
  bool FindPathEndPoint(QPoint& endPoint);
  void ApplyPath(const QPoint& endPoint, QLinkedList<QPoint>* points);
  //void SplitPath(QLinkedList<Obj*>& usedObjs, const QPoint& splitPoint);

  void ObjPrepareOne();
  void ObjTakePre();
  void ObjTakePreOne();
  void ObjAddRefPrePoint(int pointPosition);
  void ObjApplyPre();
  void ObjPreRefValidateOne(ObjRef* objRef);
  void ObjApplyPreOne();
  void ObjFromUnusedPre();
  void ObjFromUnusedPreOne();
  void ObjFromUnusedPreTakePre(const Type1Info& obj1, QLinkedList<PreObj*>& takenPre);
  void ObjFromUnusedPreTakePreOne(const Type1Info& obj1, QLinkedList<PreObj*>& takenPre);

  bool ObjSetTypeIfNearBorder(ObjType newType);

public:
  void GetDbgPreObjMark(ImageSrc<uchar>& debug);
  void GetDbgPreObjMarkSolid(ImageSrc<uchar>& debug);
  void GetDbgPreObj(ImageSrc<uchar>& debug);
  void GetDbgAllObjType1(ImageSrc<uchar>& debug);
private:
  void GetDbgType1One(ImageSrc<uchar>& debug, const Type1Info& info, uchar color, int inc = 0);
public:
  void GetDbgPreHyst(byte* data);
  void GetDbgPreHyst2(byte* data);
  void GetDbgObj(ImageSrc<uchar>& debug);

public:
  LineObj(const AnalyticsB& _Analytics, const ImageSrc<uchar>& _LayerF);

  friend class PreLength0More;
};
