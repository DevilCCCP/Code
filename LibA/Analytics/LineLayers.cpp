#include <Lib/Log/Log.h>

#include "LineLayers.h"


const int kNormalDiffMax = 5;
const int kNormalDiffMid = 3;
const int kSpecialDiffMax = 5;
const int kMinSegmentLength = 6;
const int kLightningGrow = 5;
const int kLightningLimit = 30;
const int kCreateTLayerMinMs = 5 * 1000;
const int kCreateTLayerPeriodMs = 2 * 1000;
const int kLayerTFailMaxMs = 2 * 60 * 1000;
const int kLayerLifeMaxMs = 5 * 60 * 1000;

const int kMinFailSize = 6;
const int kMinFailWeight = 4;
const int kMinNoFailWeight = 8;

void LineLayers::LoadSettings(const SettingsAS& settings)
{
  Q_UNUSED(settings);
}

void LineLayers::SaveSettings(const SettingsAS& settings)
{
  Q_UNUSED(settings);
}

void LineLayers::Init(const Region<uchar>& source)
{
  mLayerB.SetSize(Width(), Height());
  for (int j = 0; j < Height(); j++) {
    memcpy(mLayerB.Line(j), source.Line(j), Width());
  }
  mLayerT.SetSize(Width(), Height());
  mLayerD.SetSize(Width(), Height());
  CreateLineInfo();
  mCreateTLayerTimeMs = 0;
}

void LineLayers::Calc(const Region<uchar>& source)
{
  mSource = &source;

  mCreateTLayerTimeMs += FrameMs();
  bool createT = false;
  if (mCreateTLayerTimeMs >= kCreateTLayerPeriodMs) {
    mCreateTLayerTimeMs = 0;
    createT = true;
  }

  mCurrentLayer = &mLayerT;
  for (mCurrentLine = 0; mCurrentLine < Height(); mCurrentLine++) {
    mCurrentLineInfo = &mLineInfo[mCurrentLine];
    CalcSegments(&mCurrentLineInfo->TSegments);
    ValidateSegments(&mCurrentLineInfo->TSegments);
    KillTSegments();
  }

  InitPre();
  mCurrentLayer = &mLayerB;
  for (mCurrentLine = 0; mCurrentLine < Height(); mCurrentLine++) {
    mCurrentLineInfo = &mLineInfo[mCurrentLine];
    CalcSegments(&mCurrentLineInfo->Segments);
    ValidateSegments(&mCurrentLineInfo->Segments);
    CreatePre();
    KillTSegments2();
    if (createT) {
      IntegrateTSegments();
      CreateNewTSegments();
    }
  }
}

void LineLayers::CalcSegments(QLinkedList<SegInfo>* segments)
{
  mFailWeight = mNoFailWeight = 0;
  mCurrentLineInfo->FailSegments.clear();
  int lastEnd = -1;
  for (auto itrs = segments->begin(); itrs != segments->end(); itrs++) {
    mCurrentSegment = &*itrs;
    if (lastEnd != mCurrentSegment->StartPoint) {
      EndLineFailPoint();
      mCurrentLight = (int)*mSource->Data(0, mCurrentLine) - (int)*mCurrentLayer->Data(0, mCurrentLine);
    }
    switch (mCurrentSegment->Type) {
    case SegInfo::eCommon:
      CalcSegmentCommon();
      break;
    case SegInfo::eConnect:
//        CalcSegmentConnect();
//        break;
    case SegInfo::eSpecial:
    default:
      CalcSegmentSpecial();
      break;
    }
    lastEnd = mCurrentSegment->EndPoint;
  }
  EndLineFailPoint();
}

void LineLayers::ValidateSegments(QLinkedList<LineLayers::SegInfo>* segments)
{
  auto itrf = mCurrentLineInfo->FailSegments.begin();
  for (auto itrs = segments->begin(); itrs != segments->end(); itrs++) {
    mCurrentSegment = &*itrs;
    int startFail = mCurrentSegment->EndPoint;
    while (itrf != mCurrentLineInfo->FailSegments.end()) {
      const QPoint& p = *itrf;
      int finishFail = p.y();
      if (mCurrentSegment->StartPoint > finishFail) {
        itrf++;
      } else {
        startFail = p.x();
        break;
      }
    }
    if (startFail < mCurrentSegment->EndPoint) {
      mCurrentSegment->LastFailMs += FrameMs();
    } else {
      mCurrentSegment->TotalOkMs += FrameMs();
      mCurrentSegment->LastFailMs = qMax(0, mCurrentSegment->LastFailMs - 4 * FrameMs());
    }
  }
}

void LineLayers::KillTSegments()
{
  QLinkedList<SegInfo>* segments = &mCurrentLineInfo->TSegments;
  for (auto itrs = segments->begin(); itrs != segments->end(); ) {
    mCurrentSegment = &*itrs;
    if (mCurrentSegment->TotalOkMs < mCurrentSegment->LastFailMs) {
#ifndef QT_NO_DEBUG
      int pos = mCurrentSegment->StartPoint;
      int size = mCurrentSegment->EndPoint - mCurrentSegment->StartPoint;
      memset(mLayerT.Data(pos, mCurrentLine), 0, size);
#endif
      itrs = segments->erase(itrs);
    } else {
      itrs++;
    }
  }
}

void LineLayers::KillTSegments2()
{
  QLinkedList<SegInfo>* segments = &mCurrentLineInfo->Segments;
  QLinkedList<SegInfo>* segments2 = &mCurrentLineInfo->TSegments;
  auto itrs2 = segments2->begin();
  for (auto itrs = segments->begin(); itrs != segments->end() && itrs2 != segments2->end(); itrs++) {
    mCurrentSegment = &*itrs;
    if (mCurrentSegment->LastFailMs > 0) {
      continue;
    }
    for (; itrs2 != segments2->end(); ) {
      SegInfo* currentSegment2 = &*itrs2;
      if (currentSegment2->StartPoint >= mCurrentSegment->EndPoint) {
        break;
      }
      int intersectStart = qMax(mCurrentSegment->StartPoint, currentSegment2->StartPoint);
      int intersectEnd = qMin(mCurrentSegment->EndPoint, currentSegment2->EndPoint);
      int intersectLen = intersectEnd - intersectStart;
      if (currentSegment2->EndPoint - currentSegment2->StartPoint < 4 * intersectLen) {
#ifndef QT_NO_DEBUG
        int pos = currentSegment2->StartPoint;
        int size = currentSegment2->EndPoint - currentSegment2->StartPoint;
        memset(mLayerT.Data(pos, mCurrentLine), 0, size);
#endif
        itrs2 = segments2->erase(itrs2);
      } else {
        itrs2++;
      }
    }
  }
}

void LineLayers::CreateNewTSegments()
{
  QLinkedList<SegInfo>* segments = &mCurrentLineInfo->TSegments;
  auto itrs = segments->begin();
  for (auto itrf = mCurrentLineInfo->FailSegments.begin(); itrf != mCurrentLineInfo->FailSegments.end(); itrf++) {
    const QPoint& p = *itrf;
    int startFail = p.x();
    int finishFail = p.y();
    bool hasIntersect = false;
    for (; itrs != segments->end(); itrs++) {
      mCurrentSegment = &*itrs;
      if (mCurrentSegment->EndPoint >= startFail) {
        hasIntersect = mCurrentSegment->StartPoint <= finishFail;
        break;
      }
    }
    if (!hasIntersect) {
      finishFail++;
      memcpy(mLayerT.Data(startFail, mCurrentLine), mSource->Data(startFail, mCurrentLine), finishFail - startFail);
      CreateLineInfoSegments(mLayerT.Line(mCurrentLine), startFail, finishFail, mCurrentLineInfo->TSegments, itrs);
    }
  }

  //mCurrentSegment->TimeFail += FrameMs();
  //if (mCurrentSegment->TimeFail > kCreateTLayerMinMs && mCreateTLayerTimeMs >= kCreateTLayerPeriodMs) {
  //  auto itrs = mLineInfo[mCurrentLine].TSegments.begin();
  //  for (; itrs != mLineInfo[mCurrentLine].TSegments.end(); itrs++) {
  //    SegInfo* currentTSegment = &*itrs;
  //    if (currentTSegment->StartPoint >= mCurrentSegment->StartPoint) {
  //      if (currentTSegment->StartPoint == mCurrentSegment->StartPoint) {
  //        return;
  //      }
  //    }
  //  }
  //  int from = mCurrentSegment->StartPoint;
  //  int end = mCurrentSegment->EndPoint;
  //  memcpy(mLayerT.Data(from, mCurrentLine), mSource->Data(from, mCurrentLine), end - from);
  //  CreateLineInfoSegments(mLayerT.Line(mCurrentLine), from, end, mLineInfo[mCurrentLine].TSegments, itrs);
  //}
}

void LineLayers::IntegrateTSegments()
{
  //int gFrame = CurrentFrame();
  //if (gFrame >= 35 && mCurrentLine >= 3) {
  //  Log.Debug(QString("Frame: %1, line: %2").arg(gFrame).arg(mCurrentLine));
  ////if (gFrame >= 718 && mCurrentLine >= 148) {
  //  int y = 1;
  //}

  QLinkedList<SegInfo>* segments = &mCurrentLineInfo->Segments;
  QLinkedList<SegInfo>* segments2 = &mCurrentLineInfo->TSegments;
  auto itrs = segments->begin();
  for (auto itrs2 = segments2->begin(); itrs2 != segments2->end(); ) {
    mCurrentSegment = &*itrs2;
    int totalLen = 0;
    int totalOkMs = 0;
    int totalFailMs = 0;
    auto itrsStart = itrs;
    auto itrsEnd = segments->end();
    int endPoint = Width();
    for (; itrs != segments->end(); itrs++) {
      SegInfo* currentSegment = &*itrs;
      if (currentSegment->StartPoint >= mCurrentSegment->EndPoint) {
        itrsEnd = itrs;
        break;
      }
      if (currentSegment->EndPoint <= mCurrentSegment->StartPoint) {
        itrsStart++;
        continue;
      }
      endPoint = currentSegment->EndPoint;
      int intersectStart = qMax(mCurrentSegment->StartPoint, currentSegment->StartPoint);
      int intersectEnd = qMin(mCurrentSegment->EndPoint, currentSegment->EndPoint);
      int intersectLen = intersectEnd - intersectStart;
      totalLen += intersectLen;
      totalOkMs += currentSegment->TotalOkMs * intersectLen;
      totalFailMs += currentSegment->LastFailMs * intersectLen;
    }
    if (!totalLen) {
      itrs2++;
      continue;
    }
    totalOkMs /= totalLen;
    totalFailMs /= totalLen;
    if (totalOkMs < mCurrentSegment->TotalOkMs
        || (qMin(kLayerLifeMaxMs, totalOkMs) < mCurrentSegment->TotalOkMs && mCurrentSegment->LastFailMs < totalFailMs)) {

      int pos = mCurrentSegment->StartPoint;
      int size = mCurrentSegment->EndPoint - mCurrentSegment->StartPoint;
      memcpy(mLayerB.Data(pos, mCurrentLine), mLayerT.Data(pos, mCurrentLine), size);
      SegInfo* startSegment = &*itrsStart;
      int startPoint = startSegment->StartPoint;
      mCurrentLineInfo->Segments.erase(itrsStart, itrsEnd);
      CreateLineInfoSegments(mLayerB.Line(mCurrentLine), startPoint, endPoint, mCurrentLineInfo->Segments, itrsEnd);
      itrs2 = segments2->erase(itrs2);
#ifndef QT_NO_DEBUG
      memset(mLayerT.Data(pos, mCurrentLine), 0, size);
#endif
    } else {
      itrs2++;
    }
  }
}

void LineLayers::CalcSegmentCommon()
{
  const uchar*  src = mSource->Data(mCurrentSegment->StartPoint, mCurrentLine);
  const uchar* layB = mCurrentLayer->Data(mCurrentSegment->StartPoint, mCurrentLine);
#ifndef QT_NO_DEBUG
  uchar*       layD = mLayerD.Data(mCurrentSegment->StartPoint, mCurrentLine);
#endif

  for (int i = mCurrentSegment->StartPoint; i < mCurrentSegment->EndPoint; i++) {
    int d;
    int diff = (int)*src - (int)*layB;
    if (qAbs(diff) <= kNormalDiffMax) {
      d = 0;
    } else if (qAbs(diff - mCurrentLight) <= kLightningGrow && qAbs(diff) < kLightningLimit) {
      d = 1;
    } else {
      d = 2;
    }
    AddLineFailPoint(d, i);

    //    if (diff < mCurrentLight - kLightningGrow) {
    //      mCurrentLight = mCurrentLight - kLightningGrow;
    //    } else if (diff > mCurrentLight + kLightningGrow) {
    //      mCurrentLight = mCurrentLight + kLightningGrow;
    //    } else {
    //      mCurrentLight = diff;
    //    }
    mCurrentLight = diff;

    src++;
    layB++;
#ifndef QT_NO_DEBUG
    *layD = d;
    layD++;
#endif
  }
}

void LineLayers::CalcSegmentSpecial()
{
  const uchar*  src = mSource->Data(mCurrentSegment->StartPoint, mCurrentLine);
  const uchar* layB = mCurrentLayer->Data(mCurrentSegment->StartPoint, mCurrentLine);
#ifndef QT_NO_DEBUG
  uchar*       layD = mLayerD.Data(mCurrentSegment->StartPoint, mCurrentLine);
#endif

  uchar srcm = (mCurrentSegment->StartPoint > 0)? *mSource->Data(mCurrentSegment->StartPoint - 1, mCurrentLine): *mSource->Data(mCurrentSegment->StartPoint, mCurrentLine);
  uchar layBm = (mCurrentSegment->StartPoint > 0)? *mCurrentLayer->Data(mCurrentSegment->StartPoint - 1, mCurrentLine): *mCurrentLayer->Data(mCurrentSegment->StartPoint, mCurrentLine);
  for (int i = mCurrentSegment->StartPoint; i < mCurrentSegment->EndPoint; i++) {
    uchar  srcn = *src;
    uchar layBn = *layB;
    int layBd = qAbs((int)layBn - (int)layBm);
    int diff = (int)*src - (int)*layB;
    int d;
    if (qAbs(diff) <= layBd/2 + kSpecialDiffMax) {
      d = 0;
    } else if (qAbs(diff - mCurrentLight) <= layBd/2 + kLightningGrow && qAbs(diff) < kLightningLimit) {
      d = 1;
    } else {
      d = 4;
    }
    AddLineFailPoint(d, i);

//    if (d < mCurrentLight - kLightningGrow) {
//      mCurrentLight = mCurrentLight - kLightningGrow;
//    } else if (d > mCurrentLight + kLightningGrow) {
//      mCurrentLight = mCurrentLight + kLightningGrow;
//    } else {
//      mCurrentLight = d;
//    }
    mCurrentLight = diff;
    srcm = srcn;
    layBm = layBn;

    src++;
    layB++;
#ifndef QT_NO_DEBUG
    *layD = d;
    layD++;
#endif
  }
}

void LineLayers::CalcSegmentConnect()
{
  const uchar* src = mSource->Data(mCurrentSegment->StartPoint, mCurrentLine);
  for (int i = mCurrentSegment->StartPoint; i < mCurrentSegment->EndPoint; i++) {
    src++;
  }
}

void LineLayers::AddLineFailPoint(int d, int i)
{
  if (d > 1) {
    if (!mFailWeight) {
      mFailStart = i;
    }
    mFailFinish = i;
    mFailWeight++;
    mNoFailWeight = 0;
  } else if (mFailWeight) {
    if (d == 1) {
      mNoFailWeight++;
    } else {
      mNoFailWeight += 2;
    }
    if (mNoFailWeight >= kMinNoFailWeight) {
      EndLineFailPoint();
      mFailWeight = 0;
    }
  }
}

void LineLayers::EndLineFailPoint()
{
  if (mFailWeight < kMinFailWeight) {
    return;
  }
  int size = mFailFinish - mFailStart + 1;
  if (size >= kMinFailSize) {
//    if (mFailStart < kMinFailSize/2) {
//      mFailStart = 0;
//    }
//    if (mFailFinish < Width() - kMinFailSize/2 - 1) {
//      mFailFinish = Width() - 1;
//    }
    mCurrentLineInfo->FailSegments.append(QPoint(mFailStart, mFailFinish));
  }
}

void LineLayers::CreateLineInfo()
{
  mLineInfo.clear();
  for (int j = 0; j < Height(); j++) {
    mLineInfo.append(LineInfo());
    mCurrentLineInfo = &mLineInfo.last();
    CreateLineInfoSegments(mLayerB.Line(j), 0, Width(), mCurrentLineInfo->Segments, mCurrentLineInfo->Segments.end());
  }
}

void LineLayers::CreateLineInfoSegments(const uchar* src, int from, int end, QLinkedList<SegInfo>& segments, const QLinkedList<SegInfo>::iterator& insIterator)
{
  QLinkedList<QPair<int, int> > csPoints;
  const uchar* layB = src + from;
  int start = from;
  int length = 0;
  int fail = 0;
  uchar layBm = *layB;
  layB++;
  for (int i = from + 1; i < end; i++) {
    uchar layBn = *layB;
    int d = (layBn >= layBm)? layBn - layBm: layBm - layBn;
    if (d > kNormalDiffMid) {
      if (d > kNormalDiffMax || length < fail * kMinSegmentLength) {
        if (length >= kMinSegmentLength) {
          csPoints.append(qMakePair(start, length));
        }
        start = i;
        length = 0;
        fail = 0;
      } else {
        fail++;
      }
    }
    length++;

    layBm = layBn;
    layB++;
  }
  if (length >= kMinSegmentLength) {
    csPoints.append(qMakePair(start, length));
  }

  QLinkedList<SegInfo>::iterator insIter = insIterator;
  int lastPoint = from;
  for (auto itr = csPoints.begin(); itr != csPoints.end(); itr++) {
    const QPair<int, int>& p = *itr;
    int start = p.first;
    int length = p.second;
    if (start > lastPoint) {
      uchar r = src[start];
      uchar l = src[lastPoint - 1];
      int totalDiff = (int)r - (int)l;
      int midDiff = totalDiff / (start - (lastPoint - 1));
      int minDiff = (midDiff > 0)? midDiff / 2: 2 * midDiff;
      int maxDiff = (midDiff > 0)? 2 * midDiff: midDiff / 2;
      int c = (int)l;
      bool straight = true;
      for (int i = lastPoint; i < start; i++) {
        int diff = (int)src[i] - c;
        if (diff < minDiff || diff > maxDiff) {
          straight = false;
          break;
        }
        c = (int)src[i];
      }
      auto itri = segments.insert(insIter, SegInfo());
      SegInfo* info = &*itri;
      info->StartPoint = lastPoint;
      info->EndPoint = start;
      info->Type = straight? SegInfo::eConnect: SegInfo::eSpecial;
    }
    auto itri = segments.insert(insIter, SegInfo());
    SegInfo* info = &*itri;
    info->StartPoint = start;
    info->EndPoint = start + length;
    lastPoint = info->EndPoint;
    info->Type = SegInfo::eCommon;
  }
  if (lastPoint < end) {
    auto itri = segments.insert(insIter, SegInfo());
    SegInfo* info = &*itri;
    info->StartPoint = lastPoint;
    info->EndPoint = end;
    info->Type = SegInfo::eSpecial;
  }

#ifndef QT_NO_DEBUG
  lastPoint = 0;
  for (auto itrs = segments.begin(); itrs != segments.end(); itrs++) {
    mCurrentSegment = &*itrs;
    if (mCurrentSegment->StartPoint < lastPoint) {
      Q_ASSERT(0);
    }
    lastPoint = mCurrentSegment->EndPoint;
  }
#endif
}

void LineLayers::InitPre()
{
  mCurrentPreId = 0;
}

void LineLayers::CreatePre()
{
  foreach (const QPoint& p, mCurrentLineInfo->FailSegments) {
    int startPoint = p.x();
    int endPoint = p.y() - 1;

  }
}

void LineLayers::GetDbgLayerB(Region<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    const uchar* layB = mLayerB.Line(j);
    memcpy(debug.Line(j), layB, Width());
  }
}

void LineLayers::GetDbgLayerT(Region<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    const uchar* layT = mLayerT.Line(j);
    memcpy(debug.Line(j), layT, Width());
  }
}

void LineLayers::GetDbgLayerD(Region<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    const uchar* layD = mLayerD.Line(j);
    memcpy(debug.Line(j), layD, Width());
  }
}

void LineLayers::GetDbgLayerBSeg(Region<uchar>& debug)
{
  debug.ZeroData();
  for (int j = 0; j < Height(); j++) {
    const LineInfo* line = &mLineInfo.at(j);
    foreach (const SegInfo& info, line->Segments) {
      uchar color;
      if (info.Type == SegInfo::eCommon) {
        color = 2;
      } else if (info.Type == SegInfo::eSpecial) {
        color = 4;
      } else {
        color = 1;
      }
      for (int i = info.StartPoint; i < info.EndPoint; i++) {
        *debug.Data(i, j) = color;
      }
    }
  }
}

void LineLayers::GetDbgLayerBDiff(Region<uchar>& debug)
{
  debug.ZeroData();
  for (int j = 0; j < Height(); j++) {
    const LineInfo* line = &mLineInfo.at(j);
    foreach (const QPoint& p, line->FailSegments) {
      int startFail = p.x();
      int finishFail = p.y();
      for (int i = startFail; i <= finishFail; i++) {
        *debug.Data(i, j) = 4;
      }
    }
  }
}

void LineLayers::GetDbgLayerBSegFail(Region<uchar>& debug)
{
  const int kFailMin = 2000;
  const int kFailMax = 5000;
  const int kOkMax = 60 * 1000;

  debug.ZeroData();
  for (int j = 0; j < Height(); j++) {
    const LineInfo* line = &mLineInfo.at(j);
    foreach (const SegInfo& info, line->Segments) {
      uchar value;
      if (info.LastFailMs > kFailMin) {
        value = (uchar)(qMin(kFailMax, info.LastFailMs) * 127 / (kFailMax - kFailMin)) | 0x80;
      } else {
        value = (uchar)(qMin(kOkMax, info.TotalOkMs) * 127 / kOkMax);
      }
      for (int i = info.StartPoint; i < info.EndPoint; i++) {
        *debug.Data(i, j) = value;
      }
    }
  }
}

void LineLayers::GetDbgLayerTSegFail(Region<uchar>& debug)
{
  const int kFailMin = 0;
  const int kFailMax = 5000;
  const int kOkMax = 60 * 1000;

  debug.ZeroData();
  for (int j = 0; j < Height(); j++) {
    const LineInfo* line = &mLineInfo.at(j);
    foreach (const SegInfo& info, line->TSegments) {
      uchar value;
      if (info.LastFailMs > kFailMin) {
        value = (uchar)(qMin(kFailMax, info.LastFailMs) * 127 / (kFailMax - kFailMin)) | 0x80;
      } else {
        value = (uchar)(qMin(kOkMax, info.TotalOkMs) * 127 / kOkMax);
      }
      for (int i = info.StartPoint; i < info.EndPoint; i++) {
        *debug.Data(i, j) = value;
      }
    }
  }
}

void LineLayers::GetDbgSourceLineWithLayer(int line, Region<uchar>& debug)
{
  const int kLineHead = 5;

  debug.ZeroData();
  Region<uchar> source;
  source.SetSource(const_cast<uchar*>(SourceImageData()), Width(), Height(), Stride());

  int j = line;
  const uchar*  src = source.Line(j);
  const uchar* layD = mLayerD.Line(j);
  int axe = debug.Height()-1;

  for (int i = 0; i < source.Width(); i++) {
    int val = axe - *src * axe / 255;
    uchar* dbg = debug.Line(0) + i;
    int k = 0;
    for (; k < val - kLineHead; k++) {
      *dbg = (*layD)? *layD: 3;
      dbg += debug.Stride();
    }
    for (; k < val; k++) {
      *dbg = 1;
      dbg += debug.Stride();
    }

    src++;
    layD++;
  }
}

void LineLayers::GetDbgLayerBLine(int line, Region<uchar>& debug)
{
  const int kLineHead = 5;

  debug.ZeroData();

  Region<uchar> source;
  source.SetSource(const_cast<uchar*>(SourceImageData()), Width(), Height(), Stride());

  int j = line;
  const uchar* src = source.Line(j);
  const uchar* layB = mLayerB.Line(j);
  const uchar* layT = mLayerT.Line(j);
  int axe = debug.Height()-1;

  for (int i = 0; i < source.Width(); i++) {
    uchar color = 3;
    const LineInfo* line = &mLineInfo.at(j);
    foreach (const SegInfo& info, line->Segments) {
      if (i < info.StartPoint) {
        break;
      }
      if (i < info.EndPoint) {
        color = (info.Type == SegInfo::eCommon)? 2: 4;
        break;
      }
    }

    int val = axe - *layB * axe / 255;
    uchar* dbg = debug.Line(0) + i;
    int k = 0;
    for (; k < val - kLineHead; k++) {
      *dbg = color;
      dbg += debug.Stride();
    }
    for (; k < val; k++) {
      *dbg = 1;
      dbg += debug.Stride();
    }

    src++;
    layB++;
    layT++;
  }
}

void LineLayers::GetDbgLayerTLine(int line, Region<uchar>& debug)
{
  debug.ZeroData();

  Region<uchar> source;
  source.SetSource(const_cast<uchar*>(SourceImageData()), Width(), Height(), Stride());

  int j = line;
  const uchar* src = source.Line(j);
  const uchar* layB = mLayerB.Line(j);
  const uchar* layT = mLayerT.Line(j);
  int axe = debug.Height()-1;

  for (int i = 0; i < source.Width(); i++) {
    if (*layT) {
      int val = axe - *src * axe / 255;
      uchar* dbg = debug.Line(0) + i;
      for (int k = 0; k < val; k++) {
        *dbg = 2;
        dbg += debug.Stride();
      }
    } else {
      int val = axe - *src * axe / 255;
      uchar* dbg = debug.Line(0) + i;
      for (int k = 0; k < val; k++) {
        *dbg = 4;
        dbg += debug.Stride();
      }
    }

    src++;
    layB++;
    layT++;
  }
}

void LineLayers::GetDbgLayerBLineDiff(int line, Region<uchar>& debug)
{
  debug.ZeroData();

  Region<uchar> source;
  source.SetSource(const_cast<uchar*>(SourceImageData()), Width(), Height(), Stride());

  int j = line;
  const uchar* src = source.Line(j);
  const uchar* layB = mLayerB.Line(j);
  const uchar* layT = mLayerT.Line(j);
  int axe = (debug.Height() - 1)/2;
  memset(debug.Line(axe), 1, debug.Width());

  const int kMaxDiff = 10;
  for (int i = 0; i < Width(); i++) {
    int v = ((int)*src - (int)*layB);
    v = qMax(-kMaxDiff, v);
    v = qMin(kMaxDiff, v);
    int val = axe - v * axe / kMaxDiff;

    {
      uchar* dbg = debug.Line(0) + i;
      for (int k = 0; k < qMin(val, axe); k++) {
        *dbg = *layT? 4: 2;
        dbg += debug.Stride();
      }
    }
    {
      int p = qMax(val + 1, axe + 1);
      uchar* dbg = debug.Line(p) + i;
      for (int k = p; k < Height(); k++) {
        *dbg = *layT? 4: 2;
        dbg += debug.Stride();
      }
    }

    src++;
    layB++;
    layT++;
  }
}


LineLayers::LineLayers(const AnalyticsA& _Analytics)
  : SceneAnalizer(_Analytics)
{
}

LineLayers::~LineLayers()
{
}
