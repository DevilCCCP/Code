#pragma once

#include <QMutex>
#include <QWaitCondition>
#include <QVector>

#include <Lib/Include/Common.h>
#include <LibV/Include/Hyst.h>

#include "BlockSceneAnalizer.h"
#include "BlockScene.h"


DefineClassS(DiffLayers);
DefineClassS(DiffFormula);
DefineClassS(DlWorker);
DefineClassS(SettingsA);

/* Layers
 * B - instant short live background
 * Bb - stat long live background
 * T - instant very short temporary image, it may became new B
 * F - current front image
*/
class DiffLayers: public BlockSceneAnalizer
{
  enum ELayer {
    eLayerB,
    eLayerT,
    eLayerF,
    eLayerD
  };

  struct Info {
    int    DiffB;
    int    ShadB;
    int    SumDiffB;
    int    DiffT;
    int    ShadT;
  };

  struct Stat {
    ELayer Layer;
    int    LayerBTime;
    int    LayerTTime;
    int    LayerDTime;
    int    LayerBShad;
    int    LayerBShadTime;
    int    LayerBDiff;
    int    LayerTDiff;
  };

  typedef QPair<int, int> LineRange;
  typedef void (DiffLayers::*LineJob)(const DiffLayers::LineRange&, HystFast&);

  const bool             mDoubleDiff;
  int                    mThresholdSens;
  int                    mDiffBlockThreshold;
  int                    mShadBlockThreshold;
  int                    mDiffBlockMin;
  int                    mDiffBlockMinReported;
  qint64                 mDiffBlockMinReportedTs;
  int                    mShadBlockMin;

  const ImageSrc<uchar>* mSource;
  ImageSrc<uchar>        mLayerB;
  ImageSrc<uchar>        mLayerT;
  ImageSrc<uchar>        mLayerF;
  ImageSrc<uchar>        mLayerDiff;

  // mark: [0, 4)
  BlockSrc<int>          mDiffCount;
  BlockSrc<Info>         mSceneInfo;
  BlockSrc<Stat>         mSceneStat;
  int                    mTotalDiff;
  BlockSrc<int>          mSceneEnergy;
  PROPERTY_GET(int,      EnergyMs)

  int                    mThreadsCount;
  HystFast               mDiffSumHyst;
  QVector<DlWorkerS>     mWorker;
  LineJob                mWorkerJob;
  QList<LineRange>       mWorkerList;
  QMutex                 mWorkerMutex;
  QWaitCondition         mWorkerStartWait;
  QWaitCondition         mWorkerEndWait;
  int                    mWorkersCount;
  bool                   mWorkEnds;

  DiffFormulaS           mDiffFormulaB;
  DiffFormulaS           mDiffFormulaT;

  Hyst                   mDebugHyst;

public:
  const ImageSrc<uchar>& LayerDiff() const { return mLayerDiff; }
  const ImageSrc<uchar>& LayerBackground() const { return mLayerB; }
  const ImageSrc<uchar>& LayerFront() const { return mLayerF; }
  const BlockSrc<int>& BlockDiffCount() const { return mDiffCount; }
  const BlockSrc<int>& SceneEnergy() const { return mSceneEnergy; }
  void  SetThresholdSens(int _ThresholdSens);
  void  SetThreadsCount(int _ThreadsCount) { mThreadsCount = _ThreadsCount; }
  int   BlockDiffThreshold() const { return mDiffBlockThreshold; }

public:
  void LoadSettings(const SettingsAS& settings);
  void SaveSettings(const SettingsAS& settings);
  void Init(const ImageSrc<uchar>& source);

  void Calc(const ImageSrc<uchar>& source);
private:
  void CalcSingleThread();
  void CalcMultyThreads();
  void CalcDouble(const LineRange& range, HystFast& localHyst);
  void CalcHorz(const LineRange& range, HystFast& localHyst);
  void CalcDouble2(const LineRange& range, HystFast& localHyst);
  void CalcHorz2(const LineRange& range, HystFast& localHyst);

public:
  qreal CalcStable();
  void ResetEnergy();
  void MakeDiff();

private: /*internal*/
  bool WaitWorkerStart();
  bool WaitWorkerJob();
  bool DoWorkerJob();

public:
  void Update();
  int GetMediumDiff();

public:
  void GetDbgLayerBDiff(ImageSrc<uchar>& debug);
  void GetDbgLayerBDiffShad(ImageSrc<uchar>& debug);
  void GetDbgLayerBDiffCount(ImageSrc<uchar>& debug);
  void GetDbgLayerBDiffMass(ImageSrc<uchar>& debug);
  void GetDbgLayerTDiff(ImageSrc<uchar>& debug);
  void GetDbgCurrentLayerDiff(ImageSrc<uchar>& debug);
  void GetDbgLayerB(ImageSrc<uchar>& debug);
  void GetDbgLayerT(ImageSrc<uchar>& debug);
  void GetDbgLayerT1(ImageSrc<uchar>& debug);
  void GetDbgLayerT2(ImageSrc<uchar>& debug);
  void GetDbgLayerT3(ImageSrc<uchar>& debug);
  void GetDbgLayerF(ImageSrc<uchar>& debug);
  void GetDbgLayerDiff(ImageSrc<uchar>& debug);
  void GetDbgLayerDCalc(ImageSrc<uchar>& debug);
  void GetDbgLayerBThreshold(ImageSrc<uchar>& debug);
  void GetDbgLayerBFormula(ImageSrc<uchar>& debug);
  void GetDbgLayerBFormula2(ImageSrc<uchar>& debug);
  void GetDbgGrad(ImageSrc<uchar>& debug);
  void GetDbgGradHyst(byte* data);
  void GetDbgLayerLineB(int line, ImageSrc<uchar>& debug);
  void GetDbgLayerLineT(int line, ImageSrc<uchar>& debug);
  void GetDbgLayerLineF(int line, ImageSrc<uchar>& debug);
  void GetDbgLayerLineBfDiff(int line, ImageSrc<uchar>& debug);
  void GetDbgLinePlace(int line, ImageSrc<uchar>& debug);
  void GetDbgDiffHyst(byte* data);
  void GetDbgDiffB(ImageSrc<uchar>& debug);
  void GetDbgDiffT(ImageSrc<uchar>& debug);
  void GetDbgDiffBlockB(ImageSrc<uchar>& debug);
  void GetDbgDiffMinusBlockB(ImageSrc<uchar>& debug);
  void GetDbgBlockDiffHyst(int i, int j, byte* data);
  void GetDbgCurrentLayer(ImageSrc<uchar>& debug);
  void GetDbgObjBlock(ImageSrc<uchar>& debug);
  void GetDbgEnergy(ImageSrc<uchar>& debug);

private:
  void GetDbgLayer(const ImageSrc<byte>& _Layer, ELayer layer, ImageSrc<uchar>& debug);
  void GetDbgLayerFull(const ImageSrc<byte>& _Layer, ImageSrc<uchar>& debug);
  void GetDbgLayerLine(const ImageSrc<byte>& _Layer, DiffLayers::ELayer layer, int line, ImageSrc<uchar>& debug);

public:
  DiffLayers(const AnalyticsB& _Analytics, bool _DoubleDiff);
  ~DiffLayers();

  friend class DlWorker;
};

