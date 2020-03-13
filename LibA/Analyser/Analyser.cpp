#include "Analyser.h"
#include "ObjConnect.h"
#include "UinAreaStat.h"
#include "Uin.h"
#include "SignalMark3.h"
#include "UinPlate.h"
#include "UinMetrics.h"


const int kPlateLeftCharsIndex  = 1;
const int kPlateMainDigitsIndex = 4;
const int kPlateRightCharsIndex = 6;
const int kPlateLeftCharsCount  = 1;
const int kPlateMainDigitsCount = 3;
const int kPlateRightCharsCount = 2;

const int kMainPlateDigitsCountMin   = 6;
const int kMainPlateDigitsCountMax   = 6;
const int kRegionPlateDigitsCountMin = 2;
const int kRegionPlateDigitsCountMax = 3;

const UinS& Analyser::GetUin()
{
  if (!mUin) {
    UinInit();
  }
  return mUin;
}

void Analyser::TakeUin(UinS& uin)
{
  UinInit();
  uin.swap(mUin);
}

void Analyser::Init(const uchar* data, int width, int height, int stride)
{
  mWidth  = width;
  mHeight = height;
  mSrc.SetSource(const_cast<uchar*>(data), mWidth, mHeight, stride);
  mDst.SetSize(mWidth, mHeight);
}

void Analyser::Init(const Region<uchar>& region)
{
  mWidth  = region.Width();
  mHeight = region.Height();
  mSrc.SetSource(region);
  mDst.SetSize(mWidth, mHeight);
}

void Analyser::Init(const Region<uchar>& region, int x, int y, int width, int height)
{
  mWidth  = width;
  mHeight = height;
  mSrc.SetSource(region, x, y, mWidth, mHeight);
  mDst.SetSize(mWidth, mHeight);
}

void Analyser::MakeGrad()
{
  mHyst.Clear();
  for (int j = 0; j < Height() - 1; j++) {
    const uchar* srcnn = mSrc.Line(j);
    const uchar* srcnp = mSrc.Line(j + 1);
    const uchar* srcpn = mSrc.Line(j) + 1;
    uchar* img = mDst.Line(j);
    for (int i = 0; i < Width() - 1; i++) {
      int h = qAbs((int)*srcnn - (int)*srcpn);
      int v = qAbs((int)*srcnn - (int)*srcnp);
      int d = qMax(h, v);
      d = (d > 5)? qMin(255, 127 + d): 5*d;
      mHyst.Inc(d);
      *img = (uchar)(uint)d;

      srcnn++;
      srcnp++;
      srcpn++;
      img++;
    }
    *img = 0;
  }
  memset(mDst.Line(Height() - 1), 0, Width());
}

void Analyser::Make2Color(int minPerc, int maxPerc)
{
  int med = mHyst.GetMidValue(minPerc, maxPerc);

  for (int j = 0; j < Height(); j++) {
    uchar* dst = mDst.Line(j);
    for (int i = 0; i < Width(); i++) {
      if (*dst < med) {
        *dst = 0;
      } else {
        *dst = 255;
      }

      dst++;
    }
  }
}

void Analyser::MakeMedian(int len)
{
  QVector<int> values;
  for (int j = 0; j < Height(); j++) {
    uchar* dst = mDst.Line(j);
    for (int i = 0; i < Width(); i++) {
      values.clear();
      for (int jj = qMax(0, j - len); jj <= qMin(Height() - 1, j + len); jj++) {
        for (int ii = qMax(0, i - len); ii <= qMin(Width() - 1, i + len); ii++) {
          values.append((int)*mSrc.Data(ii, jj));
        }
      }
      std::sort(values.begin(), values.end());
      *dst = values.at(values.size()/2);

      dst++;
    }
  }
}

void Analyser::MakeLower(int len)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dst = mDst.Line(j);
    for (int i = 0; i < Width(); i++) {
      uchar minValue = 255;
      for (int jj = qMax(0, j - len); jj <= qMin(Height() - 1, j + len); jj++) {
        for (int ii = qMax(0, i - len); ii <= qMin(Width() - 1, i + len); ii++) {
          minValue = qMin(minValue, *mSrc.Data(ii, jj));
        }
      }
      *dst = minValue;

      dst++;
    }
  }
}

void Analyser::MakeHigher(int len)
{
  for (int j = 0; j < Height(); j++) {
    uchar* dst = mDst.Line(j);
    for (int i = 0; i < Width(); i++) {
      uchar maxValue = 0;
      for (int jj = qMax(0, j - len); jj <= qMin(Height() - 1, j + len); jj++) {
        for (int ii = qMax(0, i - len); ii <= qMin(Width() - 1, i + len); ii++) {
          maxValue = qMax(maxValue, *mSrc.Data(ii, jj));
        }
      }
      *dst = maxValue;

      dst++;
    }
  }
}

void Analyser::MakeWhiteBallance(int white, int black)
{
  CalcHyst(0, 0, mSrc.Width(), mSrc.Height());

  int whiteValue = mHyst.GetValue(white);
  int blackValue = mHyst.GetValue(black);
  int denum = qMax(whiteValue - blackValue, 1);

  for (int j = 0; j < Height(); j++) {
    const uchar* src = mSrc.Line(j);
    uchar*       dst = mDst.Line(j);
    for (int i = 0; i < Width(); i++) {
      int v = qMax(0, *src - blackValue);
      *dst = qMin(255, v * 255 / denum);

      src++;
      dst++;
    }
  }
}

void Analyser::CalcHyst(int x, int y, int width, int height)
{
  mHyst.Clear();
  for (int j = 0; j < height; j++) {
    const uchar* src = mSrc.Data(x, y + j);
    for (int i = 0; i < width; i++) {
      mHyst.Inc(*src);
      src++;
    }
  }
}

bool Analyser::FindUinRu(const Region<uchar>& region, int width)
{
  Init(region);

  mUinAreaStat.reset(new UinAreaStat(this));
  UinMetrics normalMetrics(width);
  mUinAreaStat->Calc(normalMetrics);

//  if (!mSignalMark3) {
//    mSignalMark3.reset(new SignalMark3());
//  }

//  mSignalMark3->RegionCalc1(&region);
//  UinMetrics minMetrics(width * 100/150);
//  UinMetrics normalMetrics(width);
//  UinMetrics maxMetrics(width * 150/100);
//  mSignalMark3->setSignalTopMin(normalMetrics.SymbolValue());
//  mSignalMark3->setSignalWidthMax(maxMetrics.DigitThick());
//  mSignalMark3->setSignalHeightMin(minMetrics.CharHeight()/3);
//  mSignalMark3->setPackWidth(normalMetrics.Width());
//  mSignalMark3->setPackMinSpace(3*minMetrics.DigitWidth());
//  mSignalMark3->setPackMinCount(5);
//  mSignalMark3->setAreaMinHeight(minMetrics.CharHeight()/4);
//  mSignalMark3->RegionCalc2();

//  mConnectRects.clear();
//  mSelectedRects.clear();
//  mObjRects.clear();
//  mObjRectsFixed.clear();

//  const QVector<QRect>& rects = mSignalMark3->ResultAreas();
//  foreach (const QRect& rect, rects) {
//    QRect plate = ExpandToPlate(rect);
//    CalcUinRu(region, rect, plate);
//    Region<uchar> plate;
//    if (ExtractPlate(rect, plate)) {
//      CalcPlateRu(plate);
//    }
//  }

  return true;
}

QRect Analyser::AreaToPlate(const QRect& rect)
{
//  UinMetrics moreMetrics(2*rect.width());
//  int cx = (rect.left() + rect.right())/2;
//  int cy = (rect.top() + rect.bottom())/2;
//  QRect plate(QPoint(qMax(0, cx - moreMetrics.Width()/2), qMax(0, cy - moreMetrics.Height()/3))
//              , QPoint(qMin(region.Width() - 1, cx + moreMetrics.Width()/2), qMin(region.Height() - 1, cy + moreMetrics.Height()/3)));
//  return plate;
  return rect;
}

bool Analyser::ExtractPlate(const QRect& rect, Region<uchar>& region)
{
  Q_UNUSED(rect);
  Q_UNUSED(region);

  return false;
}

bool Analyser::UinInit()
{
  mUin.reset(new Uin(this));
  const char kSymbols[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
                            , 'A', 'B', 'E','K', 'M', 'H', 'O', 'P', 'C', 'T', 'Y', 'X', '\0' };
  for (int i = 0; kSymbols[i]; i++) {
    QImage img(QString(":/UinFont/Uin %1.png").arg(kSymbols[i]));
    if (!img.isNull()) {
      mUin->AddChar(QChar(kSymbols[i]), img);
    }
  }
  return true;
}

bool Analyser::CalcUinRu(const Region<uchar>& region, const QRect& center, const QRect& plate)
{
  Init(region);
  mCurrentPlate = plate;

  if (!mUin) {
    UinInit();
  }

  CalcHyst(center.x(), center.y(), center.width(), center.height());
  int minValue  = mHyst.GetValue(300);
  int maxValue  = mHyst.GetValue(400);
  int minExpect = mHyst.GetLocalMax(0, minValue);
  int maxExpect = mHyst.GetLocalMax(maxValue, 255);

  mBlackThreshold      = (2*minExpect + maxExpect)/3;
  mWhiteThreshold      = (minExpect + 2*maxExpect)/3;
  mBackWhiteThreshold  = (minExpect + maxExpect)/2;

  if (!mObjConnect) {
    mObjConnect.reset(new ObjConnect());
  }
  Region<uchar> expendedRegion(region, mCurrentPlate);
  mObjConnect->ConnectBlack(expendedRegion, mBlackThreshold);
  const QVector<QRect>& preRects = mObjConnect->GetObjList();
  QVector<QRect> rects;
  foreach (const QRect& rect, preRects) {
    if (rect.width() >= 4 && rect.height() >= 6 && rect.width() < center.width()/2) {
      int count = (10 * rect.width()) / (6 * rect.height());
      if (count > 1) {
        for (int i = 0; i < count; i++) {
          int x1 = rect.left() + i * (rect.right() - rect.left()) / count;
          int x2 = rect.left() + (i + 1) * (rect.right() - rect.left()) / count;
          QRect r(x1, rect.top(), x2 - x1, rect.height());
          rects.append(r);
        }
      } else {
        rects.append(rect);
      }
    }
  }

  if (mDump) {
    mConnectRects.append(center);
    mConnectRects.append(plate);
    foreach (const QRect& rect, rects) {
      mConnectRects.append(rect.adjusted(mCurrentPlate.x(), mCurrentPlate.y(), mCurrentPlate.x(), mCurrentPlate.y()));
    }
  }

  mUin->BeginTest();
  int cx = (center.left() + center.right())/2 - mCurrentPlate.x();
  int cy = (center.top() + center.bottom())/2 - mCurrentPlate.y();
  while (rects.size() >= kMainPlateDigitsCountMin + kRegionPlateDigitsCountMin) {
    int bestDiff = mCurrentPlate.width() + mCurrentPlate.height();
    QRect bestRect;
    foreach (const QRect& rect, rects) {
      if (rect.width() > center.width()/2) {
        continue;
      }
      int x = (rect.left() + rect.right())/2;
      int y = (rect.top() + rect.bottom())/2;
      int diff = qAbs(cx - x) + qAbs(cy - y);
      if (diff < bestDiff) {
        bestDiff = diff;
        bestRect = rect;
      }
    }
    if (!bestRect.isValid()) {
      break;
    }
    rects.removeOne(bestRect);
    if (FindUinRuDigits(bestRect, rects)) {
      return true;
    }
  }

  return false;
}

bool Analyser::CalcPlateRu(const Region<uchar>& region)
{
  Q_UNUSED(region);

  return false;
}

bool Analyser::FindUinRuDigits(const QRect& digitRect, const QVector<QRect>& otherRects)
{
  mObjRects.clear();
  mObjRects.append(digitRect.adjusted(mCurrentPlate.x(), mCurrentPlate.y(), mCurrentPlate.x(), mCurrentPlate.y()));
  for (QRect neighbour = digitRect; FindUinNeighbourLeft(digitRect, otherRects, neighbour); ) {
    mObjRects.prepend(neighbour.adjusted(mCurrentPlate.x(), mCurrentPlate.y(), mCurrentPlate.x(), mCurrentPlate.y()));
  }
  for (QRect neighbour = digitRect; FindUinNeighbourRight(digitRect, otherRects, neighbour); ) {
    mObjRects.append(neighbour.adjusted(mCurrentPlate.x(), mCurrentPlate.y(), mCurrentPlate.x(), mCurrentPlate.y()));
  }

  if (mObjRects.isEmpty()) {
    return false;
  }

  QRect rightRect   = mObjRects.last();
  QRect biggestRect = mObjRects.last();
  foreach (const QRect& rect, mObjRects) {
    if (rect.right() > rightRect.right()) {
      rightRect = rect;
    }
    if (rect.height() > biggestRect.height()) {
      biggestRect = rect;
    }
  }
  int spaceWidth = biggestRect.width() * 3/7;
  int topDelta   = rightRect.height() * 1/3;
  rightRect.adjust(spaceWidth - mCurrentPlate.x(), -topDelta - mCurrentPlate.y()
                   , spaceWidth - mCurrentPlate.x(), -topDelta - mCurrentPlate.y());

  int regionCount = 0;
  for (int i = 0; i < 4; i++) {
    for (QRect neighbour = rightRect.adjusted(spaceWidth * i/2, 0, spaceWidth * i/2, 0)
      ; FindUinNeighbourRight(rightRect, otherRects, neighbour); ) {
      mObjRects.append(neighbour.adjusted(mCurrentPlate.x(), mCurrentPlate.y(), mCurrentPlate.x(), mCurrentPlate.y()));
      regionCount++;
    }
    if (regionCount > 0) {
      break;
    }
  }

//  if (regionCount < kRegionPlateDigitsCountMin || regionCount > kRegionPlateDigitsCountMax) {
//    return false;
//  }

  if (mDump) {
    mSelectedRects = mObjRects.toVector();
  }

  UinMetrics uinMetrics(mObjRects.last().right() - mObjRects.first().left());

  mObjRectsFixed.clear();
  for (int i = 0; i < mObjRects.size(); i++) {
    const QRect& rect = mObjRects.at(i);
    QRect rectFixed;
    QSize expectedSize;
    if (i < kPlateLeftCharsIndex || (i >= kPlateMainDigitsIndex && i < kPlateRightCharsIndex)) {
      expectedSize = QSize(uinMetrics.CharWidth(), uinMetrics.CharHeight());
    } else if (i >= kPlateLeftCharsIndex && i < kPlateMainDigitsIndex) {
      expectedSize = QSize(uinMetrics.DigitWidth(), uinMetrics.DigitHeight());
    } else {
      expectedSize = QSize(uinMetrics.PrefixWidth(), uinMetrics.PrefixHeight());
    }
    rectFixed = QRect(rect.topLeft(), QSize(qMax(rect.width(), expectedSize.width()), qMax(rect.height(), expectedSize.height())));
    mObjRectsFixed.append(rectFixed);
    mUin->BeginChar(Region<uchar>(mSrc, rectFixed), mWhiteThreshold, mBlackThreshold);
    mUin->TestCharRadius(2);
  }

  return true;
//  QVector<QRect> neighbourRects;
//  foreach (const QRect& rect, rects) {
//    if (rect.width()*2 >= digitRect.width() && rect.width() <= digitRect.width()*2
//        && rect.height()*2 >= digitRect.height() && rect.height() <= digitRect.height()*2) {
//      neighbourRects.append(rect);
//    }
//  }

  //    if (rect.width() < Width()/4 && rect.height() > 4) {
  //      QRect digitRect = rect.adjusted(mCurrentPlate.x(), mCurrentPlate.y(), mCurrentPlate.x(), mCurrentPlate.y());
  //      mObjRects.append(digitRect);
  //      mUin->TestChar(Region<uchar>(mSrc, digitRect.x(), digitRect.y(), digitRect.width(), digitRect.height()), mWhiteThreshold, mBlackThreshold);
  //    }

  return false;
}

bool Analyser::FindUinNeighbourRight(const QRect& digitRect, const QVector<QRect>& otherRects, QRect& neighbourRect)
{
  const QRect* bestRect = nullptr;
  int bestFit = digitRect.width()/2;
  foreach (const QRect& rect, otherRects) {
    if (rect == neighbourRect) {
      continue;
    }
    int fit = rect.left() - neighbourRect.right();
    if (fit < 0) {
      fit = -2*fit;
    }
    if (fit < bestFit
        && qAbs((rect.top() + rect.bottom()) - (digitRect.top() + digitRect.bottom())) < digitRect.height()
        && rect.width() >= digitRect.width()/2 && rect.width() <= digitRect.width()*2
        && rect.height() >= digitRect.height()/2 && rect.height() <= digitRect.height()*2) {
      bestRect = &rect;
      bestFit  = fit;
    }
  }
  if (bestRect) {
    neighbourRect = *bestRect;
    return true;
  }
  return false;
}

bool Analyser::FindUinNeighbourLeft(const QRect& digitRect, const QVector<QRect>& otherRects, QRect& neighbourRect)
{
  const QRect* bestRect = nullptr;
  int bestFit = digitRect.width();
  foreach (const QRect& rect, otherRects) {
    if (rect == neighbourRect) {
      continue;
    }
    int fit = neighbourRect.left() - rect.right();
    if (fit < 0) {
      fit = -2*fit;
    }
    if (fit < bestFit
        && qAbs((rect.top() + rect.bottom()) - (digitRect.top() + digitRect.bottom())) < digitRect.height()
        && rect.width() >= digitRect.width()/2 && rect.width() <= digitRect.width()*2
        && rect.height() >= digitRect.height()/2 && rect.height() <= digitRect.height()*2) {
      bestRect = &rect;
      bestFit  = fit;
    }
  }
  if (bestRect) {
    neighbourRect = *bestRect;
    return true;
  }
  return false;
}

void Analyser::DumpUinStatRaw(Region<uchar>* region, int minDiff, int)
{
  mUinAreaStat->DumpRaw(region, minDiff);
}

void Analyser::DumpUinStatRaw2(Region<uchar>* region, int minDiff, int)
{
  mUinAreaStat->DumpRaw2(region, minDiff);
}

void Analyser::DumpUinStatRaw23(Region<uchar>* region, int minDiff, int)
{
  mUinAreaStat->DumpRaw23(region, minDiff);
}

void Analyser::DumpUinStatSignal(Region<uchar>* region, int, int)
{
  mUinAreaStat->DumpSignal(region);
}

void Analyser::DumpUinStatSignalLevel(Region<uchar>* region, int level, int)
{
  mUinAreaStat->DumpSignalLevel(region, level);
}

void Analyser::DumpUinStatThickness2(Region<uchar>* region, int level, int threshold)
{
  mUinAreaStat->DumpThickness2(region, level, threshold);
}

void Analyser::DumpUinStatThickness23(Region<uchar>* region, int level, int threshold)
{
  mUinAreaStat->DumpThickness23(region, level, threshold);
}

void Analyser::DumpUinStatEdge(Region<uchar>* region, int, int)
{
  mUinAreaStat->DumpThicknessEdge(region);
}

void Analyser::DumpUinStatEdgeFiltered(Region<uchar>* region, int, int)
{
  mUinAreaStat->DumpThicknessEdgeFiltered(region);
}

void Analyser::DumpUinStatBlack(Region<uchar>* region, int ,int)
{
  mUinAreaStat->DumpBlack(region);
}

void Analyser::DumpUinStatWhite(Region<uchar>* region, int ,int)
{
  mUinAreaStat->DumpWhite(region);
}

void Analyser::DumpUinStatCountBlack(Region<uchar>* region, int, int)
{
  mUinAreaStat->DumpCountBlack(region);
}

void Analyser::DumpUinStatCountWhite(Region<uchar>* region, int, int)
{
  mUinAreaStat->DumpCountWhite(region);
}

void Analyser::DumpUinStatMiddle(Region<uchar>* region, int ,int)
{
  mUinAreaStat->DumpMiddle(region);
}

void Analyser::DumpUinStatDiff(Region<uchar>* region, int ,int)
{
  mUinAreaStat->DumpDiff(region);
}

void Analyser::DumpUinStatWhiteLevel(Region<uchar>* region, int level, int diffLevel)
{
  mUinAreaStat->DumpWhiteLevel(region, level, diffLevel);
}

void Analyser::DumpUinStatBothLevel(Region<uchar>* region, int whiteLevel, int blackLevel)
{
  mUinAreaStat->DumpBothLevel(region, whiteLevel, blackLevel);
}

void Analyser::DumpUinStatWhiteLevelCut(Region<uchar>* region, int level, int)
{
  mUinAreaStat->DumpWhiteLevelCut(region, level);
}

void Analyser::DumpUinStatPlate(Region<uchar>* region, int index, int)
{
  mUinAreaStat->DumpPlate(region, index);
}

void Analyser::DumpUinStatPlateNormal(Region<uchar>* region, int indexPlate, int indexChar)
{
  mUinAreaStat->DumpPlateNormal(region, indexPlate, indexChar);
}

void Analyser::DumpUinCutLevel(Region<uchar>* region, int whiteLevel, int blackLevel)
{
  mUinAreaStat->DumpCutLevel(region, whiteLevel, blackLevel);
}

void Analyser::DumpUinCutLevel2(Region<uchar>* region, int blackWhiteLevel, int)
{
  mUinAreaStat->DumpCutLevel2(region, blackWhiteLevel);
}

void Analyser::DumpUinColorLevel(Region<uchar>* region, int level, int)
{
  mUinAreaStat->DumpColorLevel(region, level);
}

void Analyser::DumpSignalValue(Region<uchar>* region)
{
  mSignalMark3->DumpRegionValue(region);
}

void Analyser::DumpSignalHeight(Region<uchar>* region, int, int)
{
  mSignalMark3->DumpRegionHeight(region);
}

void Analyser::DumpSignalPack(Region<uchar>* region, int, int)
{
  mSignalMark3->DumpRegionPack(region);
}

void Analyser::DumpSignalArea(Region<uchar>* region, int, int)
{
  mSignalMark3->DumpRegionArea(region);
}

void Analyser::DumpHyst(Region<uchar>* region)
{
  mDst.SetSize(255, 255);
  mDst.ZeroData();

  int totalCount = mHyst.TotalCount() / 4;
  if (totalCount <= 0) {
    return;
  }
  for (int j = 0; j < 255; j++) {
    int v = mHyst.GetHyst(j) * 255 / totalCount;
    uchar* dst = mDst.Line(j);
    for (int i = 0; i < v; i++) {
      *dst = 127;
      dst++;
    }
  }

  region->SetSource(mDst);
}

void Analyser::DumpBlackWhite(Region<uchar>* region)
{
  for (int j = 0; j < Height(); j++) {
    const uchar* src = mSrc.Line(j);
    uchar* dst = mDst.Line(j);
    for (int i = 0; i < Width(); i++) {
      if (*src <= mBlackThreshold) {
        *dst = 0;
      } else if (*src >= mWhiteThreshold) {
        *dst = 255;
      } else {
        *dst = 127;
      }

      src++;
      dst++;
    }
  }

  region->SetSource(mDst);
}

void Analyser::DumpUinSolids(Region<uchar>* region, int, int)
{
  for (int j = 0; j < Height(); j++) {
    const uchar* src = mSrc.Line(j);
    uchar* dst = mDst.Line(j);
    memcpy(dst, src, Width());
  }

  foreach (const QRect& rect, mConnectRects) {
    int l = qMax(0, rect.left() - 1);
    int t = qMax(0, rect.top() - 1);
    int r = qMin(mDst.Width() - 1, rect.right() + 1);
    int b = qMin(mDst.Height() - 1, rect.bottom() + 1);
    QRect boundingRect(l, t, r - l + 1, b - t + 1);
    mDst.FillRectBorder(boundingRect, 0);
  }

  region->SetSource(mDst);
}

void Analyser::DumpUinSymbols(Region<uchar>* region, int, int)
{
  for (int j = 0; j < Height(); j++) {
    const uchar* src = mSrc.Line(j);
    uchar* dst = mDst.Line(j);
    memcpy(dst, src, Width());
  }

  foreach (const QRect& rect, mSelectedRects) {
    int l = qMax(0, rect.left() - 1);
    int t = qMax(0, rect.top() - 1);
    int r = qMin(mDst.Width() - 1, rect.right() + 1);
    int b = qMin(mDst.Height() - 1, rect.bottom() + 1);
    QRect boundingRect(l, t, r - l + 1, b - t + 1);
    mDst.FillRectBorder(boundingRect, 255);
  }

  region->SetSource(mDst);
}

void Analyser::DumpUinSymbolsFixed(Region<uchar>* region, int, int)
{
  for (int j = 0; j < Height(); j++) {
    const uchar* src = mSrc.Line(j);
    uchar* dst = mDst.Line(j);
    memcpy(dst, src, Width());
  }

  foreach (const QRect& rect, mObjRectsFixed) {
    int l = qMax(0, rect.left() - 1);
    int t = qMax(0, rect.top() - 1);
    int r = qMin(mDst.Width() - 1, rect.right() + 1);
    int b = qMin(mDst.Height() - 1, rect.bottom() + 1);
    QRect boundingRect(l, t, r - l + 1, b - t + 1);
    mDst.FillRectBorder(boundingRect, 255);
  }

  region->SetSource(mDst);
}

void Analyser::DumpPlate(Region<uchar>* region, int, int)
{
  mDst.FillData(255);
//  for (int j = 0; j < Height(); j++) {
//    const uchar* src = mSrc.Line(j);
//    uchar* dst = mDst.Line(j);
//    memcpy(dst, src, Width());
//  }

  const QVector<QRect>& rects = mSignalMark3->ResultAreas();
  foreach (const QRect& rect, rects) {
    UinMetrics moreMetrics(2*rect.width());
    int cx = (rect.left() + rect.right())/2;
    int cy = (rect.top() + rect.bottom())/2;
    QRect plate(QPoint(qMax(0, cx - moreMetrics.Width()/2), qMax(0, cy - moreMetrics.Height()/3))
                , QPoint(qMin(mSrc.Width() - 1, cx + moreMetrics.Width()/2), qMin(mSrc.Height() - 1, cy + moreMetrics.Height()/3)));

    CalcHyst(rect.x(), rect.y(), rect.width(), rect.height());
    int minValue  = mHyst.GetValue(300);
    int maxValue  = mHyst.GetValue(400);
    int minExpect = mHyst.GetLocalMax(0, minValue);
    int maxExpect = mHyst.GetLocalMax(maxValue, 255);

    mBlackThreshold      = (2*minExpect + maxExpect)/3;
    mWhiteThreshold      = (minExpect + 2*maxExpect)/3;
    mBackWhiteThreshold  = (minExpect + maxExpect)/2;

    for (int j = plate.top(); j <= plate.bottom(); j++) {
      const uchar* src = mSrc.Data(plate.left(), j);
      uchar*       dst = mDst.Data(plate.left(), j);
      for (int i = plate.left(); i <= plate.right(); i++) {
        if (*src > mWhiteThreshold) {
          *dst = 255;
        } else if (*src <= mBlackThreshold) {
          *dst = 0;
        } else {
          *dst = 127;
        }
        src++;
        dst++;
      }
    }
  }

  region->SetSource(mDst);
}

void Analyser::DumpUinPrepare(Region<uchar>* region, int, int)
{
  if (!mUin) {
    UinInit();
  }
  if (!mUin) {
    return;
  }

  mUin->DumpBase(region);
}

void Analyser::DumpUinTest(Region<uchar>* region, int index, int)
{
  mUinAreaStat->DumpPlateUinTest(region, index);
}

void Analyser::DumpUinDigits(Region<uchar>* region, int index, int)
{
  mUinAreaStat->DumpPlateUinDigits(region, index);
}


Analyser::Analyser(bool _Dump)
  : mDump(_Dump), mWidth(0), mHeight(0)
{
}

