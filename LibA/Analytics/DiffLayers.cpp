#include <QMutexLocker>

#include <Lib/Log/Log.h>
#include <LibV/Include/Tools.h>

#include "DiffLayers.h"
#include "DiffFormula.h"
#include "DlWorker.h"
#include "AnalyticsB.h"


const int kShadThreshold = 5;
const int kObjThreshold = 25;
const int kDiffBlockThresholdMin = 25;
const int kDiffBlockThresholdMax = 100;
const int kShadBlockThresholdMin = 50;
const int kShadBlockThresholdMax = 200;
const int kLayerSwitchTime = 30000;
const int kLayerDMax = 2 * 60 * 1000;
const int kDiffBlockMinChangeReport = 8;
const int kDiffBlockMinChangeReportMs = 5*1000;

void DiffLayers::SetThresholdSens(int _ThresholdSens)
{
  mThresholdSens = _ThresholdSens;
}

void DiffLayers::LoadSettings(const SettingsAS& settings)
{
  mSceneEnergy.InitSource();

  DeserializeBlock(settings, "SceneEnergy", mSceneEnergy, mEnergyMs);
}

void DiffLayers::SaveSettings(const SettingsAS& settings)
{
  SerializeBlock(settings, "SceneEnergy", mSceneEnergy, mEnergyMs);
}

void DiffLayers::Init(const ImageSrc<uchar>& source)
{
  mDiffBlockThreshold = BlockSize() * (kDiffBlockThresholdMin + (kDiffBlockThresholdMax - kDiffBlockThresholdMin) * mThresholdSens / 100) / 100;
  mShadBlockThreshold = BlockSize() * (kShadBlockThresholdMin + (kShadBlockThresholdMax - kShadBlockThresholdMin) * mThresholdSens / 100) / 100;
  Log.Info(QString("Block diff thresolds set to %1 / %2").arg(mDiffBlockThreshold).arg(mShadBlockThreshold));

  mLayerB.InitSource();
  mLayerT.InitSource();
  mLayerF.InitSource();
  mLayerDiff.InitSource();

  for (int j = 0; j < Height(); j++) {
    memcpy(mLayerB.Line(j), source.Line(j), source.Width());
    memset(mLayerT.Line(j), 0, mLayerT.Width());
    memset(mLayerF.Line(j), 0, mLayerT.Width());
  }

  mDiffCount.InitSource();
  mSceneInfo.InitSource();
  mSceneStat.InitSource();

  mDiffFormulaB.reset(new DiffFormula(&mDiffSumHyst, BlockSize()));
  mDiffFormulaT.reset(new DiffFormula(&mDiffSumHyst, BlockSize()));
}

void DiffLayers::Calc(const ImageSrc<uchar>& source)
{
  mSource = &source;
  mSceneInfo.ZeroSource();
  mDiffFormulaB->Init();
  mDiffFormulaT->Init();
  mDiffSumHyst.Clear();

//  if (mDoubleDiff) {
//    mWorkerJob = source.isPacked()? &DiffLayers::CalcDouble2: &DiffLayers::CalcDouble;
//  } else {
//    mWorkerJob = source.isPacked()? &DiffLayers::CalcHorz2: &DiffLayers::CalcHorz;
//  }
  mWorkerJob = source.isPacked()? &DiffLayers::CalcHorz2: &DiffLayers::CalcHorz;

  if (mThreadsCount <= 1) {
    CalcSingleThread();
  } else {
    CalcMultyThreads();
  }
}

void DiffLayers::CalcSingleThread()
{
  int first = mDoubleDiff? 1: 0;
  (this->*mWorkerJob)(qMakePair(first, Height()), mDiffSumHyst);
}

void DiffLayers::CalcMultyThreads()
{
  int parts = mThreadsCount;
  QMutexLocker lock(&mWorkerMutex);
  if (mWorker.isEmpty()) {
    mWorker.resize(parts);
    for (int i = 0; i < mWorker.size(); i++) {
      mWorker[i].reset(new DlWorker(this));
      mWorker[i]->start();
    }
  }

  int last = mDoubleDiff? 1: 0;
  for (int i = 0; i < parts; i++) {
    int next = (Height() * (i + 1) / parts + BlockSize() - 1) / BlockSize() * BlockSize();
    next = qMin(next, Height());
    if (next > last) {
      mWorkerList.append(qMakePair(last, next));
      last = next;
    }
  }

  mWorkersCount = mWorker.size();
  mWorkerStartWait.wakeAll();
  mWorkerEndWait.wait(&mWorkerMutex);
}

void DiffLayers::CalcDouble(const LineRange& range, HystFast& localHyst)
{
  for (int j = range.first; j < range.second; j++) {
    const uchar* imgb = mSource->Line(j - 1);
    const uchar* img = mSource->Line(j);
    uchar bip = *img;
    const uchar* layB = mLayerB.Line(j);
    const uchar* layT = mLayerT.Line(j);
    uchar* layF = mLayerF.Line(j);
    Info* info = mSceneInfo.BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      for (int i = 0; i < BlockSize(); i++) {
        uchar bi = *img;
        uchar bib = *imgb;
        uchar bb = *layB;
        uchar tb = *layT;

        uchar bfh = (bip > bi)? bip - bi: bi - bip;
        uchar bfv = (bib > bi)? bib - bi: bi - bib;
        uchar bf = qMax(bfh, bfv);
        *layF = bf;

        if (int value = mDiffFormulaB->Calc(bf, bb)) {
          info->DiffB += value;
        }
        if (int value = mDiffFormulaT->Calc(bf, tb)) {
          info->DiffT += value;
        }
        localHyst.Inc((int)bf);

        bip = bi;
        img++;
        imgb++;
        layB++;
        layT++;
        layF++;
      }
      info++;
    }
  }
}

void DiffLayers::CalcHorz(const LineRange& range, HystFast& localHyst)
{
  for (int j = range.first; j < range.second; j++) {
    const uchar* img = mSource->Line(j);
    const uchar* layB = mLayerB.Line(j);
    const uchar* layT = mLayerT.Line(j);
    uchar* layF = mLayerF.Line(j);
    uchar* layD = mLayerDiff.Line(j);
    Info* info = mSceneInfo.BlockLine(j);
    Stat* stat = mSceneStat.BlockLine(j);
    uchar bp = *img;

    for (int ii = 0; ii < BlockWidth(); ii++) {
      for (int i = 0; i < BlockSize(); i++) {
        uchar bi = *img;
        uchar bb = *layB;
        uchar tb = *layT;

        uchar bf = bi;
        *layF = bf;
        int dB = (int)bf - (int)bb - stat->LayerBShad;
        info->SumDiffB += dB;
        int diffB = qAbs(dB);
        if (diffB > kShadThreshold) {
          if (diffB > kObjThreshold) {
            info->DiffB++;
            *layD = 2;
          } else {
            info->ShadB++;
            *layD = 1;
          }
        } else {
          *layD = 0;
        }
        int diffT = qAbs((int)bf - (int)tb);
        if (diffT > kShadThreshold) {
          if (diffT > kObjThreshold) {
            info->DiffT++;
          } else {
            info->ShadT++;
          }
        }
        localHyst.Inc(qAbs((int)bi - (int)bp));

        bp = bi;
        img++;
        layB++;
        layT++;
        layF++;
        layD++;
      }
      info++;
      stat++;
    }
  }
}

void DiffLayers::CalcDouble2(const LineRange& range, HystFast& localHyst)
{
  for (int j = range.first; j < range.second; j++) {
    const uchar* imgb = mSource->Line(j - 1);
    const uchar* img = mSource->Line(j);
    uchar bip = *img;
    const uchar* layB = mLayerB.Line(j);
    const uchar* layT = mLayerT.Line(j);
    uchar* layF = mLayerF.Line(j);
    Info* info = mSceneInfo.BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      for (int i = 0; i < BlockSize(); i++) {
        uchar bi = *img;
        uchar bib = *imgb;
        uchar bb = *layB;
        uchar tb = *layT;

        uchar bfh = (bip > bi)? bip - bi: bi - bip;
        uchar bfv = (bib > bi)? bib - bi: bi - bib;
        uchar bf = qMax(bfh, bfv);
        *layF = bf;

        if (int value = mDiffFormulaB->Calc(bf, bb)) {
          info->DiffB += value;
        }
        if (int value = mDiffFormulaT->Calc(bf, tb)) {
          info->DiffT += value;
        }
        localHyst.Inc((int)bf);

        bip = bi;
        img += 2;
        imgb += 2;
        layB++;
        layT++;
        layF++;
      }
      info++;
    }
  }
}

void DiffLayers::CalcHorz2(const LineRange& range, HystFast& localHyst)
{
  for (int j = range.first; j < range.second; j++) {
    const uchar* img = mSource->Line(j);
    const uchar* layB = mLayerB.Line(j);
    const uchar* layT = mLayerT.Line(j);
    uchar* layF = mLayerF.Line(j);
    uchar* layD = mLayerDiff.Line(j);
    Info* info = mSceneInfo.BlockLine(j);
    Stat* stat = mSceneStat.BlockLine(j);
    uchar bp = *img;

    for (int ii = 0; ii < BlockWidth(); ii++) {
      for (int i = 0; i < BlockSize(); i++) {
        uchar bi = *img;
        uchar bb = *layB;
        uchar tb = *layT;

        uchar bf = bi;
        *layF = bf;
        int dB = (int)bf - (int)bb - stat->LayerBShad;
        info->SumDiffB += dB;
        int diffB = qAbs(dB);
        if (diffB > kShadThreshold) {
          if (diffB > kObjThreshold) {
            info->DiffB++;
            *layD = 2;
          } else {
            info->ShadB++;
            *layD = 1;
          }
        } else {
          *layD = 0;
        }
        int diffT = qAbs((int)bf - (int)tb);
        if (diffT > kShadThreshold) {
          if (diffT > kObjThreshold) {
            info->DiffT++;
          } else {
            info->ShadT++;
          }
        }
        localHyst.Inc(qAbs((int)bi - (int)bp));

        bp = bi;
        img += 2;
        layB++;
        layT++;
        layF++;
        layD++;
      }
      info++;
      stat++;
    }
  }
}

qreal DiffLayers::CalcStable()
{
  return mTotalDiff / 255.0;
}

void DiffLayers::ResetEnergy()
{
  mEnergyMs = 0;
  mSceneEnergy.InitSource();
}

void DiffLayers::MakeDiff()
{
  const int kDiffShift = 4;
  for (int j = 0; j < Height(); j++) {
    const uchar* layF = mLayerF.Line(j);
    const uchar* layB = mLayerB.Line(j);
    uchar* diff = mLayerDiff.Line(j);
    for (int i = 0; i < Width(); i++) {
      const uchar& bi = *layF;
      const uchar& bb = *layB;
      if (bi > bb) {
        int d = ((bi - bb) >> kDiffShift);
        *diff = qMin(d, 3);
      } else {
        *diff = 0;
      }

      diff++;
      layF++;
      layB++;
    }
  }
}

bool DiffLayers::WaitWorkerStart()
{
  QMutexLocker lock(&mWorkerMutex);
  return !mWorkEnds;
}

bool DiffLayers::WaitWorkerJob()
{
  QMutexLocker lock(&mWorkerMutex);
  mWorkerStartWait.wait(&mWorkerMutex);
  return !mWorkEnds;
}

bool DiffLayers::DoWorkerJob()
{
  HystFast localHyst;
  QMutexLocker lock(&mWorkerMutex);
  forever {
    if (mWorkerList.isEmpty()) {
      mDiffSumHyst.Add(localHyst);
      break;
    }
    LineRange range = mWorkerList.takeFirst();
    lock.unlock();
    (this->*mWorkerJob)(range, localHyst);
    lock.relock();
  }

  mWorkersCount--;
  if (mWorkersCount <= 0) {
    mWorkerEndWait.wakeOne();
  }
  if (!mWorkEnds) {
    mWorkerStartWait.wait(&mWorkerMutex);
  }
  return !mWorkEnds;
}

inline void CopyLayerPart(int ii, int jj, ImageSrc<uchar>& _DiffLayerDst, const ImageSrc<uchar>& _DiffLayerSrc, int blockSize)
{
  uchar* layd = _DiffLayerDst.Line(jj * blockSize) + (ii * blockSize);
  const uchar* lays = _DiffLayerSrc.Line(jj * blockSize) + (ii * blockSize);
  for (int j = 0; j < blockSize; j++) {
    memcpy(layd, lays, blockSize);

    layd += _DiffLayerDst.Stride();
    lays += _DiffLayerSrc.Stride();
  }
}

inline void MergeLayerPart(int ii, int jj, ImageSrc<uchar>& _DiffLayerDst, const ImageSrc<uchar>& _DiffLayerSrc, int blockSize, DiffFormula* diffFormula)
{
  uchar* layd = _DiffLayerDst.Line(jj * blockSize) + (ii * blockSize);
  const uchar* lays = _DiffLayerSrc.Line(jj * blockSize) + (ii * blockSize);
  for (int j = 0; j < blockSize; j++) {
    for (int i = 0; i < blockSize; i++) {
      diffFormula->Merge(*lays, *layd);

      layd++;
      lays++;
    }

    layd += _DiffLayerDst.Stride();
    lays += _DiffLayerSrc.Stride();
  }
}

void DiffLayers::Update()
{
  int time = FrameMs();
  int energy = time;
  mEnergyMs += energy;
  mTotalDiff = 0;
  Hyst diffHyst(BlockSize() * BlockSize() + 1);
  Hyst shadHyst(BlockSize() * BlockSize() + 1);
  for (int jj = 0; jj < BlockHeight(); jj++) {
    const Info* info = mSceneInfo.Line(jj);
    Stat* stat = mSceneStat.Line(jj);
    int* diff = mDiffCount.Line(jj);
    int* eng = mSceneEnergy.Line(jj);
    for (int ii = 0; ii < BlockWidth(); ii++) {
      mTotalDiff += info->DiffB;
      diffHyst.Inc(info->DiffB);
      shadHyst.Inc(info->ShadB);
      *diff = info->DiffB;
      int diffB = info->DiffB - (mDiffBlockMin + stat->LayerBDiff);
      if (diffB >= mDiffBlockThreshold) {
        *eng += energy;
        // layB miss
        stat->LayerBTime -= time / 4;
        int diffT = info->DiffT - (mDiffBlockMin + stat->LayerTDiff);

        if (diffT < mDiffBlockThreshold) {
          if (stat->LayerTTime > stat->LayerBTime || stat->LayerTTime > kLayerSwitchTime) {
            // layB <- layT
            stat->LayerBTime = stat->LayerTTime;
            stat->LayerTTime = 0;
            stat->LayerBDiff = stat->LayerTDiff;
            stat->LayerTDiff = 0;

            CopyLayerPart(ii, jj, mLayerB, mLayerT, BlockSize());
            stat->Layer = eLayerB;
            stat->LayerBTime += time;
          } else {
            stat->Layer = eLayerT;
            stat->LayerTTime += time;
          }
        } else {
          // layT new
          CopyLayerPart(ii, jj, mLayerT, mLayerF, BlockSize());

          stat->Layer = eLayerF;
          stat->LayerTTime = 0;
          stat->LayerTDiff = 0;
        }
      } else {
//        if (info->ShadB > mShadBlockThreshold) {
//          *diff += 1;
//        }
        int newShad = info->SumDiffB / (BlockSize()*BlockSize());
        int shadDiff = newShad - stat->LayerBShad;
        if (shadDiff > 0) {
          if (stat->LayerBShadTime < 0) {
            stat->LayerBShadTime = 0;
          }
          stat->LayerBShadTime += time;
          if (stat->LayerBShadTime > 500) {
            stat->LayerBShad++;
            stat->LayerBShadTime = 0;
          }
        } else if (shadDiff < 0) {
          if (stat->LayerBShadTime > 0) {
            stat->LayerBShadTime = 0;
          }
          stat->LayerBShadTime -= time;
          if (stat->LayerBShadTime < -500) {
            stat->LayerBShad--;
            stat->LayerBShadTime = 0;
          }
        } else {
          stat->LayerBShadTime = 0;
        }

        stat->Layer = eLayerB;
        stat->LayerBTime += time;
      }

      diff++;
      info++;
      stat++;
      eng++;
    }
  }

  mDiffBlockMin = 3*diffHyst.GetValue(100);
//  Log.Debug(QString("Noise threshold set to %1, %2, %3").arg(mDiffBlockMin).arg(diffHyst.GetValue(400)).arg(diffHyst.GetValue(700)));
  if (qAbs(mDiffBlockMin - mDiffBlockMinReported) >= kDiffBlockMinChangeReport && CurrentMs() < mDiffBlockMinReportedTs + kDiffBlockMinChangeReportMs) {
    mDiffBlockMinReported = mDiffBlockMin;
    mDiffBlockMinReportedTs = CurrentMs();
    Log.Info(QString("Noise threshold set to %1").arg(mDiffBlockMinReported));
  }
  mShadBlockMin = shadHyst.GetValue(100);
}

int DiffLayers::GetMediumDiff()
{
  return mDiffSumHyst.GetValue(990) + mDiffSumHyst.GetValue(950);
}

void DiffLayers::GetDbgLayerBDiff(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layF = mLayerF.Line(j);
    const uchar* layB = mLayerB.Line(j);
    for (int i = 0; i < Width(); i++) {
      const uchar& bi = *layF;
      const uchar& bb = *layB;
      uchar d;
      if (bi > bb) {
        d = bi - bb;
        d = (d > 50)? 127: d * 5 / 2;
      } else {
        d = bb - bi;
        d = (d > 50)? 127: d * 5 / 2;
        d |= 0x80;
      }

      *dbg = d;
      dbg++;
      layF++;
      layB++;
    }
  }
}

void DiffLayers::GetDbgLayerBDiffShad(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layF = mLayerF.Line(j);
    const uchar* layB = mLayerB.Line(j);
    const Stat* stat = mSceneStat.BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      int shad = stat->LayerBShad;
      for (int i = 0; i < BlockSize(); i++) {
        const uchar& bi = *layF;
        uchar bb = (uchar)qBound(0, *layB + shad, 255);
        uchar d;
        if (bi > bb) {
          d = bi - bb;
          d = (d > 50)? 127: d * 5 / 2;
        } else {
          d = bb - bi;
          d = (d > 50)? 127: d * 5 / 2;
          d |= 0x80;
        }

        *dbg = d;

        dbg++;
        layF++;
        layB++;
      }
      stat++;
    }
  }
}

void DiffLayers::GetDbgLayerBDiffCount(ImageSrc<uchar>& debug)
{
  debug.LineZero(0);
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const Info* info = mSceneInfo.BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      uchar v = (uchar)(qMin(255, 255 * (info->DiffB * 4 + info->ShadB) / (BlockSize() * BlockSize())));
      for (int i = 0; i < BlockSize(); i++) {
        *dbg = v;

        dbg++;
      }
      info++;
    }
  }
}

void DiffLayers::GetDbgLayerBDiffMass(ImageSrc<uchar>& debug)
{
  debug.LineZero(0);
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const Info* info = mSceneInfo.BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      uchar v = (uchar)(qMin(127, qAbs(info->SumDiffB / (4 * BlockSize()))));
      if (info->SumDiffB < 0) {
        v |= 0x80;
      }
      for (int i = 0; i < BlockSize(); i++) {
        *dbg = v;

        dbg++;
      }
      info++;
    }
  }
}

void DiffLayers::GetDbgLayerTDiff(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layF = mLayerF.Line(j);
    const uchar* layT = mLayerT.Line(j);
    for (int i = 0; i < Width(); i++) {
      const uchar& bi = *layF;
      const uchar& bb = *layT;
      uchar d;
      if (bi > bb) {
        d = bi - bb;
        d = (d > 50)? 127: d * 5 / 2;
      } else {
        d = bb - bi;
        d = (d > 50)? 127: d * 5 / 2;
        d |= 0x80;
      }

      *dbg = d;
      dbg++;
      layF++;
      layT++;
    }
  }
}

void DiffLayers::GetDbgCurrentLayerDiff(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layF = mLayerF.Line(j);
    const uchar* layB = mLayerB.Line(j);
    const uchar* layT = mLayerB.Line(j);
    const Stat* stat = mSceneStat.BlockLine(j);
    for (int ii = 0; ii < BlockWidth(); ii++) {
      for (int i = 0; i < BlockSize(); i++) {
        const uchar& bi = *layF;
        const uchar& bb = (stat->Layer == eLayerT)? *layT: *layB;
        uchar d;
        if (bi > bb) {
          d = bi - bb;
          d = (d > 50)? 127: d * 5 / 2;
        } else {
          d = bb - bi;
          d = (d > 50)? 127: d * 5 / 2;
          d |= 0x80;
        }

        *dbg = d;
        dbg++;
        layF++;
        layB++;
        layT++;
      }
      stat++;
    }
  }
}

void DiffLayers::GetDbgLayerB(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layF = mLayerF.Line(j);
    const uchar* layB = mLayerB.Line(j);
    const Stat* stat = mSceneStat.BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      for (int i = 0; i < BlockSize(); i++) {
        int val = qBound(0, ((int)*layB + stat->LayerBShad)>>1, 127);

//        int d = qAbs((int)*layF - (int)*layX);
//        *dbg = (uchar)(char)((d <= kShadThreshold)? val: (val | 0x80));
        *dbg = (uchar)(char)((stat->Layer == eLayerB)? val: (val | 0x80));

        layF++;
        layB++;
        dbg++;
      }
      stat++;
    }
    if (SrcWidth() > Width()) {
      memset(dbg, 0, SrcWidth() - Width());
    }
  }
  for (int j = Height(); j < SrcHeight(); j++) {
    memset(debug.Line(0) + j * debug.Stride(), 0, debug.Stride());
  }
}

void DiffLayers::GetDbgLayerT(ImageSrc<uchar>& debug)
{
  return GetDbgLayer(mLayerT, eLayerT, debug);
}

void DiffLayers::GetDbgLayerT1(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layT = mLayerT.Line(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      for (int i = 0; i < BlockSize(); i++) {
        int val = *layT;
        *dbg = val;

        layT++;
        dbg++;
      }
    }
    if (SrcWidth() > Width()) {
      memset(dbg, 0, SrcWidth() - Width());
    }
  }
  for (int j = Height(); j < SrcHeight(); j++) {
    memset(debug.Line(j), 0, debug.Stride());
  }
}

void DiffLayers::GetDbgLayerT2(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layF = mLayerF.Line(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      for (int i = 0; i < BlockSize(); i++) {
        int val = *layF;
        *dbg = val;

        layF++;
        dbg++;
      }
    }
    if (SrcWidth() > Width()) {
      memset(dbg, 0, SrcWidth() - Width());
    }
  }
  for (int j = Height(); j < SrcHeight(); j++) {
    memset(debug.Line(j), 0, debug.Stride());
  }
}

void DiffLayers::GetDbgLayerT3(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layT = mLayerT.Line(j);
    const uchar* layF = mLayerF.Line(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      for (int i = 0; i < BlockSize(); i++) {
        int val = qAbs((int)*layF - (int)*layT);
        *dbg = (val <= kShadThreshold)? 0x7f: 0xff;

        layT++;
        layF++;
        dbg++;
      }
    }
    if (SrcWidth() > Width()) {
      memset(dbg, 0, SrcWidth() - Width());
    }
  }
  for (int j = Height(); j < SrcHeight(); j++) {
    memset(debug.Line(j), 0, debug.Stride());
  }
}

void DiffLayers::GetDbgLayerF(ImageSrc<uchar>& debug)
{
  return GetDbgLayer(mLayerF, eLayerF, debug);
}

void DiffLayers::GetDbgLayerDiff(ImageSrc<uchar>& debug)
{
  return GetDbgLayerFull(mLayerDiff, debug);
}

void DiffLayers::GetDbgLayerDCalc(ImageSrc<uchar>& debug)
{
  debug.LineZero(0);
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layD = mLayerDiff.Line(j);
    for (int i = 0; i < Width(); i++) {
      if (*layD) {
        *dbg = *layD;
      }

      dbg++;
      layD++;
    }
  }
}

void DiffLayers::GetDbgLayerBThreshold(ImageSrc<uchar>& debug)
{
  debug.ZeroSource();
  for (int jj = 0; jj < BlockHeight(); jj++) {
    const Stat* stat = mSceneStat.Line(jj);
    for (int ii = 0; ii < BlockWidth(); ii++) {
      for (int k = 0; k < stat->LayerBDiff; k++) {
        *debug.Data(ii*BlockSize() + qrand()%BlockSize(), jj*BlockSize() + qrand()%BlockSize()) = 1;
      }
      stat++;
    }
  }
}

void DiffLayers::GetDbgLayerBFormula(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layB = mLayerB.Line(j);
    const uchar* layF = mLayerF.Line(j);

    for (int i = 0; i < Width(); i++) {
      int val = mDiffFormulaB->Calc(*layF, *layB);
      if (val > 0) {
        *dbg = (uchar)qMin(4, val);
      }

      layB++;
      layF++;
      dbg++;
    }
  }
}

void DiffLayers::GetDbgLayerBFormula2(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layB = mLayerB.Line(j);
    const uchar* layF = mLayerF.Line(j);

    for (int i = 0; i < Width(); i++) {
      int val = mDiffFormulaB->CalcFormula2(*layF, *layB);
      if (val > 0) {
        *dbg = (uchar)qMin(4, val);
      }

      layB++;
      layF++;
      dbg++;
    }
  }
}

void DiffLayers::GetDbgGrad(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layF = mLayerF.Line(j);
    uchar bp = *layF;

    for (int i = 0; i < Width(); i++) {
      uchar bi = *layF;
      *dbg = qAbs((int)bi - (int)bp);

      bp = bi;
      layF++;
      dbg++;
    }
  }
}

void DiffLayers::GetDbgGradHyst(byte* data)
{
  int* datai = reinterpret_cast<int*>(data);

  int totalCount = mDiffSumHyst.TotalCount();
  datai[0] = totalCount;
  for (int i = 0; i < 256; i++) {
    datai[i + 1] = mDiffSumHyst.Data()[i * mDiffSumHyst.GetLength() / 256];
  }
//  Log.Debug(QString("990 = %1, 950 = %2, 900 = %3").arg(mDiffSumHyst.GetValue(990)).arg(mDiffSumHyst.GetValue(950)).arg(mDiffSumHyst.GetValue(900)));
}

void DiffLayers::GetDbgLayerLineB(int line, ImageSrc<uchar>& debug)
{
  return GetDbgLayerLine(mLayerB, eLayerB, line, debug);
}

void DiffLayers::GetDbgLayerLineT(int line, ImageSrc<uchar>& debug)
{
  return GetDbgLayerLine(mLayerT, eLayerT, line, debug);
}

void DiffLayers::GetDbgLayerLineF(int line, ImageSrc<uchar>& debug)
{
  return GetDbgLayerLine(mLayerF, eLayerF, line, debug);
}

void DiffLayers::GetDbgLayerLineBfDiff(int line, ImageSrc<uchar>& debug)
{
  const int kDiffMark = 5;
  const int kMaschtab = 4;
  debug.ClearSource();
  int axe = (Height() - 1)/2;
  memset(debug.Line(axe), 1, debug.Width());
  memset(debug.Line(axe - kDiffMark * kMaschtab), 1, debug.Width());
  memset(debug.Line(axe + kDiffMark * kMaschtab), 1, debug.Width());

  int j = line;
  const uchar* layB = mLayerB.Line(j);
  const uchar* layF = mLayerF.Line(j);

  for (int i = 0; i < Width(); i++) {
    int d = kMaschtab * (*layF - *layB);
    if (d >= 0) {
      int val = qMax(axe - d * axe / 255, 0);
      uchar* dbg = debug.Line(val) + i;
      for (int k = val; k < axe; k++) {
        *dbg = 2;
        dbg += debug.Stride();
      }
    } else {
      int val = qMin(axe - d * axe / 255, Height() - 1);
      uchar* dbg = debug.Line(axe + 1) + i;
      for (int k = axe + 1; k <= val; k++) {
        *dbg = 4;
        dbg += debug.Stride();
      }
    }

    layF++;
    layB++;
  }
}

void DiffLayers::GetDbgLinePlace(int line, ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* src = ImageData().Line(j);
    if (j == line + 1) {
      memset(dbg, 0, debug.Stride());
    } else {
      memcpy(dbg, src, debug.Stride());
    }
  }
}

void DiffLayers::GetDbgDiffHyst(uchar* data)
{
  int* datai = reinterpret_cast<int*>(data);
  Hyst hyst;
  for (int j = 1; j < Height() - 1; j++) {
    const uchar* layF = mLayerF.Line(j);
    const uchar* layB = mLayerB.Line(j);
    for (int i = 1; i < Width() - 1; i++) {
      int b = *layB;
      int f = *layF;
      hyst.Inc(qAbs(f - b));

      layF++;
      layB++;
    }
  }

  int totalCount = hyst.TotalCount();
  datai[0] = totalCount;
  memset(datai + 1, 0, sizeof(int) * 256);
  memcpy(datai + 1, hyst.Data(), sizeof(int) * hyst.Size());

//  int lowLevel = mDiffSumHyst.GetValue(500) + 2;
//  int highLevel = mDiffSumHyst.GetValue(990);

//  datai[1 + lowLevel] = totalCount / 2;
//  datai[1 + lowLevel+1] = totalCount * 3/2;
//  datai[1 + highLevel] = totalCount / 2;
//  datai[1 + highLevel+1] = totalCount * 3/2;
}

void DiffLayers::GetDbgDiffB(ImageSrc<uchar>& debug)
{
  debug.LineZero(0);
  for (int j = 0; j < Height(); j++) {
    const uchar* layF = mLayerF.Line(j);
    const uchar* layB = mLayerB.Line(j);
    uchar* dbg = debug.Line(j);

    for (int i = 0; i < Width(); i++) {
      *dbg = qAbs((int)*layF - (int)*layB);

      dbg++;
      layF++;
      layB++;
    }
  }
}

void DiffLayers::GetDbgDiffT(ImageSrc<uchar>& debug)
{
  debug.LineZero(0);
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const Info* info = mSceneInfo.BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      for (int i = 0; i < BlockSize(); i++) {
        *dbg = (uchar)qMin(255, info->DiffT);

        dbg++;
      }
      info++;
    }
  }
}

void DiffLayers::GetDbgDiffBlockB(ImageSrc<uchar>& debug)
{
  debug.LineZero(0);
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const Info* info = mSceneInfo.BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      for (int i = 0; i < BlockSize(); i++) {
        *dbg = (uchar)qMin(255, qAbs(info->SumDiffB) / 8);

        dbg++;
      }
      info++;
    }
  }
}

void DiffLayers::GetDbgDiffMinusBlockB(ImageSrc<uchar>& debug)
{
  debug.LineZero(0);
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layF = mLayerF.Line(j);
    const uchar* layB = mLayerB.Line(j);
    const Info* info = mSceneInfo.BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      int blockDiff = info->SumDiffB / (BlockSize() * BlockSize());
      for (int i = 0; i < BlockSize(); i++) {
        *dbg = qAbs((int)*layF - (int)*layB - blockDiff);

        dbg++;
        layF++;
        layB++;
      }
      info++;
    }
  }
}

void DiffLayers::GetDbgBlockDiffHyst(int i, int j, uchar* data)
{
  static Hyst gHyst(3 * BlockSize() * BlockSize() + 1);

  int* datai = reinterpret_cast<int*>(data);
  int value = mSceneInfo.Line(j)[i].DiffB;
  gHyst.Inc(value);

  int totalCount = gHyst.TotalCount();
  datai[0] = totalCount/2;
  memset(datai + 1, 0, sizeof(int) * 256);
  memcpy(datai + 1, gHyst.Data(), sizeof(int) * gHyst.Size());
}

void DiffLayers::GetDbgCurrentLayer(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    Stat* stat = mSceneStat.BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      uchar val = (uchar)stat->Layer;

      for (int i = 0; i < BlockSize(); i++) {
        *dbg++ = val;
      }
      stat++;
    }
  }
}

void DiffLayers::GetDbgObjBlock(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    int* diff = mDiffCount.BlockLine(j);
    const BlockInfo* info = GetBlockInfo().BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      int object;
      if ((info->Flag & BlockInfo::eIgnore) == 0) {
        object = *diff;
      } else {
        object = 0;
      }

      uchar val = qMin(object, 2);
      for (int i = 0; i < BlockSize(); i++) {
        *dbg = val;
        dbg++;
      }
      diff++;
      info++;
    }
  }
}

void DiffLayers::GetDbgEnergy(ImageSrc<uchar>& debug)
{
  int maxValue = mSceneEnergy.CalcMaxValue();

  debug.ClearSource();
  if (maxValue <= 0) {
    return;
  }

  int values[5];
  for (int j = BlockSize(); j < Height() - BlockSize(); j++) {
    uchar* dbg = debug.Line(j) + BlockSize();
    const int* en[5];
    en[0] = mSceneEnergy.BlockLine(j) + 1;
    en[1] = mSceneEnergy.BlockLine(j - BlockSize()) + 1;
    en[2] = mSceneEnergy.BlockLine(j + BlockSize()) + 1;
    en[3] = en[0] - 1;
    en[4] = en[0] + 1;

    for (int ii = 1; ii < BlockWidth() - 1; ii++) {
      for (int i = 0; i < 5; i++) {
        values[i] = *en[i];
        en[i]++;
      }
      std::sort(values, values + 5);
      uchar val = values[0] * 255 / maxValue;
      for (int i = 0; i < BlockSize(); i++) {
        *dbg = val;
        dbg++;
      }
    }
  }
}

void DiffLayers::GetDbgLayer(const ImageSrc<uchar>& _Layer, DiffLayers::ELayer layer, ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layF = mLayerF.Line(j);
    const uchar* layX = _Layer.Line(j);
    const Stat* stat = mSceneStat.BlockLine(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      for (int i = 0; i < BlockSize(); i++) {
        int val = ((*layX)>>1);

//        int d = qAbs((int)*layF - (int)*layX);
//        *dbg = (uchar)(char)((d <= kShadThreshold)? val: (val | 0x80));
        *dbg = (uchar)(char)((stat->Layer == layer)? val: (val | 0x80));

        layF++;
        layX++;
        dbg++;
      }
      stat++;
    }
    if (SrcWidth() > Width()) {
      memset(dbg, 0, SrcWidth() - Width());
    }
  }
  for (int j = Height(); j < SrcHeight(); j++) {
    memset(debug.Line(0) + j * debug.Stride(), 0, debug.Stride());
  }
}

void DiffLayers::GetDbgLayerFull(const ImageSrc<uchar>& _Layer, ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dbg = debug.Line(j);
    const uchar* layX = _Layer.Line(j);

    for (int ii = 0; ii < BlockWidth(); ii++) {
      for (int i = 0; i < BlockSize(); i++) {
        *dbg = *layX;

        layX++;
        dbg++;
      }
    }
  }
}

void DiffLayers::GetDbgLayerLine(const ImageSrc<uchar>& _Layer, DiffLayers::ELayer layer, int line, ImageSrc<uchar>& debug)
{
  debug.ClearSource();

  int j = line;
  const uchar* layX = _Layer.Line(j);
  const Stat* stat = mSceneStat.BlockLine(j);

  for (int ii = 0; ii < BlockWidth(); ii++) {
    for (int i = 0; i < BlockSize(); i++) {
      int val = Height()-1 - *layX * (Height()-1) / 255;
      uchar* dbg = debug.Line(val) + ii * BlockSize() + i;
      for (int k = val; k < Height(); k++) {
        *dbg = 1;

        dbg += debug.Stride();
      }

      int layVal = (stat->Layer == layer)? 2: 4;
      dbg = debug.Line(0) + ii * BlockSize() + i;
      for (int k = 0; k < val - 10; k++) {
        *dbg = layVal;

        dbg += debug.Stride();
      }

      layX++;
    }
    stat++;
  }
}


DiffLayers::DiffLayers(const AnalyticsB& _Analytics, bool _DoubleDiff)
  : BlockSceneAnalizer(_Analytics)
  , mDoubleDiff(_DoubleDiff), mThresholdSens(50), mDiffBlockMin(0), mShadBlockMin(0), mDiffBlockMinReported(-kDiffBlockMinChangeReport), mDiffBlockMinReportedTs(0)
  , mLayerB(GetBlockScene()), mLayerT(GetBlockScene()), mLayerF(GetBlockScene()), mLayerDiff(GetBlockScene())
  , mDiffCount(GetBlockScene()), mSceneInfo(GetBlockScene()), mSceneStat(GetBlockScene()), mSceneEnergy(GetBlockScene())
  , mThreadsCount(1), mWorkEnds(false)
{
}

DiffLayers::~DiffLayers()
{
  QMutexLocker lock(&mWorkerMutex);
  mWorkEnds = true;
  mWorkersCount = mWorker.size();
  if (mWorkersCount > 0) {
    mWorkerStartWait.wakeAll();
    mWorkerEndWait.wait(&mWorkerMutex);
    for (int i = 0; i < mWorker.size(); i++) {
      mWorker[i]->wait();
    }
  }
}

