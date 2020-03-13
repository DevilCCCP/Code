#pragma once

#include <QVector>

#include <Lib/Include/Common.h>
#include <LibV/Va/SceneAnalizer.h>
#include <LibV/Va/Region.h>

#include "Hyst.h"


DefineClassS(LineLayers);
DefineClassS(SettingsA);

/* Layers
 * B - instant short live background
 * Bb - stat long live background
 * T - instant very short temporary image, it may became new B
 * F - current front image
*/
class LineLayers: public SceneAnalizer
{
  enum ELayer {
    eLayerB,
    eLayerT,
    eLayerF,
    eLayerD
  };

  struct SegInfo {
    enum EType {
      eCommon,
      eSpecial,
      eConnect,
      eTypeIllegal
    };

    int   StartPoint;
    int   EndPoint;
    EType Type;
    int   TotalOkMs;
    int   LastFailMs;

    SegInfo()
      : TotalOkMs(0), LastFailMs(0)
    { }
  };

  struct LineInfo {
    QLinkedList<SegInfo> Segments;
    QLinkedList<SegInfo> TSegments;
    QLinkedList<QPoint>  FailSegments;
  };

  const Region<uchar>*  mSource;
  Region<uchar>*        mCurrentLayer;
  Region<uchar>         mLayerB;
  Region<uchar>         mLayerT;
  Region<uchar>         mLayerD;
  QVector<LineInfo>     mLineInfo;

  int                   mCurrentLine;
  LineInfo*             mCurrentLineInfo;
  SegInfo*              mCurrentSegment;
  int                   mCurrentLight;
  int                   mCreateTLayerTimeMs;
  bool                  mCreateTLayerNow;
  int                   mFailStart;
  int                   mFailFinish;
  int                   mFailWeight;
  int                   mNoFailWeight;

  struct Pre {
    QRect  Border;
    QPoint Center;
    int    Count;
  };

  int                   mCurrentPreId;
  QVector<int>          mPreMap;
  QVector<Pre>          mPres;

public:
  void LoadSettings(const SettingsAS& settings);
  void SaveSettings(const SettingsAS& settings);
  void Init(const Region<uchar>& source);
  void Calc(const Region<uchar>& source);

private:
  void CalcSegments(QLinkedList<SegInfo>* segments);
  void ValidateSegments(QLinkedList<SegInfo>* segments);
  void KillTSegments();
  void KillTSegments2();
  void CreateNewTSegments();
  void IntegrateTSegments();
  void CalcSegmentCommon();
  void CalcSegmentSpecial();
  void CalcSegmentConnect();
  void AddLineFailPoint(int d, int i);
  void EndLineFailPoint();

  void CreateLineInfo();
  void CreateLineInfoSegments(const uchar* src, int from, int end, QLinkedList<SegInfo>& segments, const QLinkedList<SegInfo>::iterator& insIterator);

  void InitPre();
  void CreatePre();

public:
  void GetDbgLayerB(Region<uchar>& debug);
  void GetDbgLayerT(Region<uchar>& debug);
  void GetDbgLayerD(Region<uchar>& debug);
  void GetDbgLayerBSeg(Region<uchar>& debug);
  void GetDbgLayerBDiff(Region<uchar>& debug);
  void GetDbgLayerBSegFail(Region<uchar>& debug);
  void GetDbgLayerTSegFail(Region<uchar>& debug);
  void GetDbgSourceLineWithLayer(int line, Region<uchar>& debug);
  void GetDbgLayerBLine(int line, Region<uchar>& debug);
  void GetDbgLayerTLine(int line, Region<uchar>& debug);
  void GetDbgLayerBLineDiff(int line, Region<uchar>& debug);

public:
  LineLayers(const AnalyticsA& _Analytics);
  ~LineLayers();
};

