#include <QImage>
#include <QPainter>

#include <Lib/Log/Log.h>
#include <LibV/Include/Region.h>
#include <LibV/Include/Tools.h>

#include "AnalyticsB.h"
#include "DiffLayers.h"


const int kDoorSize = 1;
const int kDiffCountReportThreshold = 18;
const int kStableStartMs = 15000;
const int kStableTimeMinMs = 1000;
const int kStableTimeTooOldMs = 30*60*1000;
const qreal kStableNum = 0.75;
const int kUnstableValue = 1000;

void AnalyticsB::SetBlockSize(int _BlockSize)
{
  mBlockSize = _BlockSize;
  Log.Info(QString("Block size set to %1").arg(mBlockSize));
}

void AnalyticsB::InitSettings(const SettingsAS& settings)
{
  mOpenCamera = settings->GetValue("Open", true).toBool();

  ExtraSettings(settings);
}

void AnalyticsB::InitDetectors(const QList<DetectorS>& _Detectors)
{
  mBariers.clear();
  mDoors.clear();
  mLineDoors.clear();
  mUin.clear();
  mIgnore.clear();
  mStatLineZones.clear();

  for (auto itr = _Detectors.begin(); itr != _Detectors.end(); itr++) {
    const DetectorS& detector = *itr;
    if (detector->Name == "iod") {
      Log.Info("New in/out detector:");
      Barier barier;
      barier.Detector = detector;
      if (LoadDetectorPoints(detector, barier.Points) && barier.Points.size() >= 2) {
        mBariers.append(barier);
      }
    } else if (detector->Name == "ioo") {
      Log.Info("New door:");
      PointList door;
      if (LoadDetectorPoints(detector, door)) {
        if (door.size() > 2) {
          AddZone(mDoors, door);
        } else if (door.size() == 2) {
          mLineDoors.append(door);
        }
      }
    } else if (detector->Name == "arn") {
      Log.Info("New UIN:");
      PointList uin;
      if (LoadDetectorPoints(detector, uin)) {
        if (uin.size() > 2) {
          AddZone(mUin, uin);
        }
      }
    } else if (detector->Name == "ign") {
      Log.Info("New ignore zone:");
      PointList zone;
      if (LoadDetectorPoints(detector, zone) && zone.size() >= 3) {
        AddZone(mIgnore, zone);
      }
    } else if (detector->Name == "qud") {
      Log.Info("New queue detector:");
      StatZone lineZone;
      lineZone.Detector = detector;
      if (LoadDetectorPoints(detector, lineZone.Points) && lineZone.Points.size() >= 3) {
        mStatLineZones.append(lineZone);
      }
    } else if (detector->Name == "izd") {
      Log.Info("New 'in' zone:");
      InZone inZone;
      inZone.Detector = detector;
      PointList zone;
      if (LoadDetectorPoints(detector, zone) && zone.size() >= 3) {
        AddZone(mZone, zone);
        mInZones.append(inZone);
      }
    }
  }
}

bool AnalyticsB::InitImageParameters(int width, int height, int stride)
{
  mBlockScene.SetScene(mBlockSize, width, height);
  mImageData.SetStride(stride);
  mImageData.setPacked(getCompression() == eRawYuvP);
  mDebugData.SetStride(width);

  if (!RestoreSettings()) {
    return false;
  }

  InitBlockInfo();
  return true;
}

void AnalyticsB::AnalizePrepare(const uchar* imageData)
{
  mImageData.SetSource(imageData);
}

bool AnalyticsB::AnalizeStandBy()
{
  mStableTimeMs += getFrameMs();

  AnalizeFront();
  if (TrySwitchStandByOff()) {
    SwitchStandByOff();
  }
  return true;
}

bool AnalyticsB::AnalizeNormal()
{
  mStableTimeMs += getFrameMs();

  AnalizeFront();

  if (getUseStandBy()) {
    if (TrySwitchStandByOn()) {
      SwitchStandByOn();
      return true;
    }
  }

  AnalizeScene();
  if (NeedStable()) {
    AnalizeStable();
  }
  return true;
}

void AnalyticsB::SaveVariables(const SettingsAS& settings)
{
  mDiffAvgHyst.Normalize(14*24*60*60*30);
  QString hystText = mDiffAvgHyst.Serialize(128);
  settings->SetValue("DiffHyst", hystText);
  if (!SyncBackImage()) {
    mStableTimeMs = 0;
    mStableValue = kUnstableValue;
  }
  settings->SetValue("StableTimeMs", mStableTimeMs);
  settings->SetValue("StableValue", mStableValue);

  SaveSettings(settings);
}

bool AnalyticsB::NeedStable()
{
  return false;
}

qreal AnalyticsB::CalcStable()
{
  return 0;
}

bool AnalyticsB::SerializeBlock(const SettingsAS& settings, const char* name, const BlockSrc<int>& block, int timeMs) const
{
  int multiply;
  QByteArray data;
  CompactBlock(block, data, multiply);
  bool ok = SyncImage(name, data);
  if (!ok) {
    multiply = 0;
    timeMs = 0;
  }
  settings->SetValue(name, multiply);
  settings->SetValue(QString("%1Ms").arg(name), timeMs);
  return ok;
}

bool AnalyticsB::DeserializeBlock(const SettingsAS& settings, const char* name, BlockSrc<int>& block, int& timeMs) const
{
  QByteArray data;
  data.resize(block.Width() * block.Height());
  if (!RestoreImage(name, data)) {
    timeMs = 0;
    return false;
  }
  if (int multiply = settings->GetValue(name, 0).toInt()) {
    DecompactBlock(block, data, multiply);
  }
  timeMs = settings->GetValue(QString("%1Ms").arg(name), 0).toInt();
  return true;
}

void AnalyticsB::CompactBlock(const BlockSrc<int>& block, QByteArray& data, int& multiply) const
{
  int maxValue = block.CalcMaxValue();
  if (maxValue <= 0) {
    multiply = 0;
    return;
  }

  multiply = 1 + maxValue / 256;

  data.resize(block.Width() * block.Height());
  for (int jj = 0; jj < block.Height(); jj++) {
    const int* src = block.Line(jj);
    char*      dst = data.data() + jj * block.Width();
    for (int ii = 0; ii < block.Width(); ii++) {
      *dst = (char)(uchar)(*src / multiply);
      src++;
      dst++;
    }
  }
}

void AnalyticsB::DecompactBlock(BlockSrc<int>& block, const QByteArray& data, int multiply) const
{
  for (int jj = 0; jj < block.Height(); jj++) {
    const char* src = data.data() + jj * block.Width();
    int*        dst = block.Line(jj);
    for (int ii = 0; ii < block.Width(); ii++) {
      *dst = ((int)(uchar)*src) * multiply;
      src++;
      dst++;
    }
  }
}

void AnalyticsB::MakeStatImage(const BlockSrc<int>& block, QByteArray& data)
{
  Region<uchar> background;
  background.SetSource((uchar*)getBackImage().data(), getWidth(), getHeight(), getWidth());
  QImage img(getWidth(), getHeight(), QImage::Format_ARGB32);
  std::vector<int> values;
  values.reserve(block.Height() * block.Width());
  for (int jj = 0; jj < block.Height(); jj++) {
    const int* src = block.Line(jj);
    const BlockInfo* info = mBlockInfo.Line(jj);
    for (int ii = 0; ii < block.Width(); ii++) {
      if ((info->Flag & BlockInfo::eIgnore) == 0) {
        values.push_back(*src);
      }

      src++;
      info++;
    }
  }
  std::sort(values.begin(), values.end());
  int minValue = (values.size() > 8)? values[values.size() / 8 - 1]: 0;
  int maxValue = (values.size() > 8)? values[values.size() * 7 / 8]: 1;

  for (int j = 0; j < img.height(); j++) {
    qreal jp = -0.5 + (0.5 + j) / getBlockSize();
    int j1 = jp >= 0? (int)jp: (int)jp - 1;
    int j2 = j1 + 1;
    qreal kj2 = jp - j1;
    qreal kj1 = 1.0 - kj2;
    if (j1 < 0) {
      j1 = 0;
    }
    if (j2 > block.Height() - 1) {
      j2 = block.Height() - 1;
    }

    const uchar* bgr = background.Line(j);
    QRgb* data = reinterpret_cast<QRgb*>(img.scanLine(j));
    for (int i = 0; i < img.width(); i++) {
      qreal ip = -0.5 + (0.5 + i) / getBlockSize();
      int i1 = ip >= 0? (int)ip: (int)ip - 1;
      int i2 = i1 + 1;
      qreal ki2 = ip - i1;
      qreal ki1 = 1.0 - ki2;
      if (i1 < 0) {
        i1 = 0;
      }
      if (i2 > block.Width() - 1) {
        i2 = block.Width() - 1;
      }

      qreal sum = 0;
      if ((mBlockInfo.Line(j1)[i1].Flag & BlockInfo::eIgnore) == 0) {
        sum += *(block.Line(j1) + i1) * kj1 * ki1;
      }
      if ((mBlockInfo.Line(j1)[i2].Flag & BlockInfo::eIgnore) == 0) {
        sum += *(block.Line(j1) + i2) * kj1 * ki2;
      }
      if ((mBlockInfo.Line(j2)[i1].Flag & BlockInfo::eIgnore) == 0) {
        sum += *(block.Line(j2) + i1) * kj2 * ki1;
      }
      if ((mBlockInfo.Line(j2)[i2].Flag & BlockInfo::eIgnore) == 0) {
        sum += *(block.Line(j2) + i2) * kj2 * ki2;
      }
      qreal percent = (sum - minValue) / (maxValue - minValue);
      percent = qMin(1.0, percent);
      percent = qMax(0.0, percent);
      percent = qSqrt(percent);
      int saturation = percent < 0.25? 0: 255;
      int hue = percent < 0.25? 0: (int)(120 - (120 * 100 / 75) * (percent - 0.25));
      int value = *bgr;
      *data = QColor::fromHsv(hue, saturation, value).rgb();

      data++;
      bgr++;
    }
  }

  QPainter painter(&img);
  painter.drawImage(img.width() - mLogoImg->width(), img.height() - mLogoImg->height(), *mLogoImg);

  data.clear();
  QBuffer s(&data);
  if (s.open(QBuffer::WriteOnly)) {
    img.save(&s, "jpg", 80);
  }
}

ImageSrc<uchar>& AnalyticsB::DebugData(uchar* data)
{
  mDebugData.SetSource(data);
  memset(data + mDebugData.Height() * mDebugData.Stride(), 0, (getHeight() - mDebugData.Height()) * mDebugData.Stride());
  return mDebugData;
}

void AnalyticsB::GetDbgIgnore(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < getBlockScene().Height(); j++) {
    uchar* dbg = debug.Line(j);
    const BlockInfo* info = getBlockInfo().BlockLine(j);
    for (int ii = 0; ii < getBlockScene().BlockWidth(); ii++) {
      uchar val;
      if ((info->Flag & BlockInfo::eIgnore) == BlockInfo::eIgnore) {
        val = 1;
      } else {
        val = 0;
      }

      for (int i = 0; i < getBlockScene().BlockSize(); i++) {
        *dbg++ = val;
      }
      info++;
    }
  }
}

void AnalyticsB::GetDbgDoor(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < getBlockScene().Height(); j++) {
    uchar* dbg = debug.Line(j);
    const BlockInfo* info = getBlockInfo().BlockLine(j);
    for (int ii = 0; ii < getBlockScene().BlockWidth(); ii++) {
      uchar val;
      if ((info->Flag & BlockInfo::eDoor) == BlockInfo::eDoor) {
        val = 4;
      } else if ((info->Flag & BlockInfo::eIgnore) == BlockInfo::eIgnore) {
        val = 1;
      } else {
        val = 0;
      }

      for (int i = 0; i < getBlockScene().BlockSize(); i++) {
        *dbg++ = val;
      }
      info++;
    }
  }
}

void AnalyticsB::GetDbgUin(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < getBlockScene().Height(); j++) {
    uchar* dbg = debug.Line(j);
    const BlockInfo* info = getBlockInfo().BlockLine(j);
    for (int ii = 0; ii < getBlockScene().BlockWidth(); ii++) {
      uchar val;
      if ((info->Flag & BlockInfo::eUin) == BlockInfo::eUin) {
        val = 4;
      } else {
        val = 0;
      }

      for (int i = 0; i < getBlockScene().BlockSize(); i++) {
        *dbg++ = val;
      }
      info++;
    }
  }
}

void AnalyticsB::GetDbgZone(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < getBlockScene().Height(); j++) {
    uchar* dbg = debug.Line(j);
    const BlockInfo* info = getBlockInfo().BlockLine(j);
    for (int ii = 0; ii < getBlockScene().BlockWidth(); ii++) {
      uchar val;
      if ((info->Flag & BlockInfo::eZone) == BlockInfo::eZone) {
        val = (info->Flag & 0x03) + 1;
      } else {
        val = 0;
      }

      for (int i = 0; i < getBlockScene().BlockSize(); i++) {
        *dbg++ = val;
      }
      info++;
    }
  }
}

void AnalyticsB::GetDbgStatSection(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < getBlockScene().Height(); j++) {
    uchar* dbg = debug.Line(j);
    const BlockInfo* info = getBlockInfo().BlockLine(j);
    for (int ii = 0; ii < getBlockScene().BlockWidth(); ii++) {
      uchar val;
      if ((info->Flag & BlockInfo::eStat) != 0) {
        val = ((info->Flag & BlockInfo::eValueMask) % 4) + 1;
      } else {
        val = 0;
      }

      for (int i = 0; i < getBlockScene().BlockSize(); i++) {
        *dbg++ = val;
      }
      info++;
    }
  }
}

bool AnalyticsB::RestoreSettings()
{
  QString group = QString("%1x%2").arg(getWidth()).arg(getHeight());
  if (!getLocalSettings()->BeginGroup(group)) {
    return false;
  }

  QString hystText = getLocalSettings()->GetValue("DiffHyst", QString("")).toString();
  mDiffAvgHyst.Deserialize(hystText, 128);
  mStableTimeMs = getLocalSettings()->GetValue("StableTimeMs", 0).toInt();
  mStableValue = getLocalSettings()->GetValue("StableValue", (qreal)kUnstableValue).toReal();
  if (!RestoreBackImage()) {
    mStableTimeMs = 0;
    mStableValue = kUnstableValue;
  }

  LoadSettings(getLocalSettings());

  getLocalSettings()->EndGroup();
  return true;
}

void AnalyticsB::AnalizeStable()
{
  if (mStableTimeMs >= kStableTimeMinMs && getCurrentMs() > kStableStartMs) {
    qreal newStable = CalcStable();
    qreal hasStable = (mStableTimeMs > kStableTimeTooOldMs)? mStableValue * mStableTimeMs / kStableTimeTooOldMs + 1: mStableValue;
    if (newStable < hasStable) {
      SetCurrentAsBackImage();
      mStableTimeMs = 0;
      mStableValue = kStableNum * newStable;
    }
  }
}

void AnalyticsB::InitBlockInfo()
{
  mBlockInfo.InitSource();

  if (mDoors.size() > 0) {
    for (int jj = 0; jj < mBlockInfo.Height(); jj++) {
      BlockInfo* info = mBlockInfo.Line(jj);
      for (int ii = 0; ii < mBlockInfo.Width(); ii++) {
        if (IsPointInDoor(ii, jj)) {
          info->Flag |= (int)BlockInfo::eIgnore;

          for (int jjj = qMax(0, jj - kDoorSize); jjj <= qMin(mBlockInfo.Height() - 1, jj + kDoorSize); jjj++) {
            BlockInfo* info_ = mBlockInfo.Line(jjj);
            for (int iii = qMax(0, ii - kDoorSize); iii <= qMin(mBlockInfo.Width() - 1, ii + kDoorSize); iii++) {
              info_[iii].Flag |= (int)BlockInfo::eDoor;
            }
          }
        }
        info++;
      }
    }
  }

  if (mUin.size() > 0) {
    for (int jj = 0; jj < mBlockInfo.Height(); jj++) {
      BlockInfo* info = mBlockInfo.Line(jj);
      for (int ii = 0; ii < mBlockInfo.Width(); ii++) {
        if (IsPointInUin(ii, jj)) {
          info->Flag |= (int)BlockInfo::eUin;
        }
        info++;
      }
    }
  }

  for (int i = 0; i < mLineDoors.size(); i++) {
    const PointList& line = mLineDoors.at(i);
    if (line.size() == 2) {
      QPointF p1 = PointToScreen(line.at(0));
      QPointF p2 = PointToScreen(line.at(1));
      qreal dx = p2.x() - p1.x();
      qreal dy = p2.y() - p1.y();
      if (qAbs(p1.x() - p2.x()) >= qAbs(p1.y() - p2.y())) {
        qreal k = dy / dx;
        int x1 = (p1.x() < p2.x())? (int)p1.x(): (int)(p1.x() + 0.99);
        int x2 = (p2.x() < p1.x())? (int)p2.x(): (int)(p2.x() + 0.99);
        int xsign = x1 < x2? 1: -1;
        for (int i = x1; i != x2; i += xsign) {
          qreal y = p1.y() + (i + 0.5 - p1.x()) * k;
          int j1 = (int)(y - 0.1);
          int j2 = (int)(y + 0.1);
          MarkPoint(i, j1, BlockInfo::eDoor);
          if (j2 != j1) {
            MarkPoint(i, j2, BlockInfo::eDoor);
          }
        }
      } else {
        qreal k = dx / dy;
        int y1 = (p1.y() < p2.y())? (int)p1.y(): (int)(p1.y() + 0.99);
        int y2 = (p2.y() < p1.y())? (int)p2.y(): (int)(p2.y() + 0.99);
        int ysign = y1 < y2? 1: -1;
        for (int j = y1; j != y2; j += ysign) {
          qreal x = p1.x() + (j + 0.5 - p1.y()) * k;
          int i1 = (int)(x - 0.1);
          int i2 = (int)(x + 0.1);
          MarkPoint(i1, j, BlockInfo::eDoor);
          if (i2 != i1) {
            MarkPoint(i1, j, BlockInfo::eDoor);
          }
        }
      }
    }
  }

  if (mZone.size() > 0) {
    for (int jj = 0; jj < mBlockInfo.Height(); jj++) {
      BlockInfo* info = mBlockInfo.Line(jj);
      for (int ii = 0; ii < mBlockInfo.Width(); ii++) {
        int iZone = GetPointZone(ii, jj);
        if (iZone >= 0) {
          info->Flag &= (~(int)BlockInfo::eValueMask);
          info->Flag |= ((int)BlockInfo::eZone | iZone);
        }
        info++;
      }
    }
  }

  if (mIgnore.size() > 0) {
    for (int jj = 0; jj < mBlockInfo.Height(); jj++) {
      BlockInfo* info = mBlockInfo.Line(jj);
      for (int ii = 0; ii < mBlockInfo.Width(); ii++) {
        if (IsPointInIgnore(ii, jj)) {
          info->Flag |= (int)BlockInfo::eIgnore;
        }
        info++;
      }
    }
  }

  mSectionCount = 0;
  for (auto itr = mStatLineZones.begin(); itr != mStatLineZones.end(); itr++) {
    StatZone* zone = &*itr;
    zone->SectionFirst = mSectionCount;
    InitLineZoneInfo(zone->Points, BlockInfo::eStat);
    zone->SectionLast = mSectionCount;
  }

  if (mOpenCamera) {
    BlockInfo* infot = mBlockInfo.Line(0);
    BlockInfo* infob = mBlockInfo.Line(mBlockInfo.Height() - 1);
    for (int ii = 0; ii < mBlockInfo.Width(); ii++) {
      infot->Flag |= (int)BlockInfo::eDoor;
      infob->Flag |= (int)BlockInfo::eDoor;

      infot++;
      infob++;
    }

    for (int jj = 0; jj < mBlockInfo.Height(); jj++) {
      BlockInfo* infol = mBlockInfo.Line(jj);
      BlockInfo* infor = infol + (mBlockInfo.Width() - 1);
      infol->Flag |= (int)BlockInfo::eDoor;
      infor->Flag |= (int)BlockInfo::eDoor;
    }
  }
}

void AnalyticsB::InitLineZoneInfo(const AnalyticsB::PointList& points, BlockInfo::EFlag flag)
{
  if (points.size() < 3 || (mSectionCount & BlockInfo::eValueMask) != mSectionCount) {
    return;
  }

  QPointF basePoint = points[0];
  QPointF leftPoint = points[1];
  QPointF rightPoint = points[2];
  QPointF midPoint = (leftPoint + rightPoint) * 0.5;
  leftPoint += basePoint - midPoint;
  rightPoint += basePoint - midPoint;
  QVector<PointList> sector(1, PointList());
  for (int i = 2; i < points.size(); i += 2) {
    QPointF leftNextPoint = points[i-1];
    QPointF rightNextPoint = points[i];
    sector[0] = PointList() << leftPoint << leftNextPoint << rightNextPoint << rightPoint;

    for (int jj = 0; jj < mBlockInfo.Height(); jj++) {
      BlockInfo* info = mBlockInfo.Line(jj);
      for (int ii = 0; ii < mBlockInfo.Width(); ii++) {
        if (IsPointInArea(sector, ii, jj) >= 0) {
          if ((info->Flag & BlockInfo::eIgnore) == 0) {
            info->Flag &= (~(int)BlockInfo::eValueMask);
            info->Flag |= ((int)flag | mSectionCount);
          }
        }
        info++;
      }
    }
    mSectionCount++;
    if ((mSectionCount & BlockInfo::eValueMask) != mSectionCount) {
      return;
    }

    leftPoint = leftNextPoint;
    rightPoint = rightNextPoint;
  }
}

bool AnalyticsB::InitStandByCalc()
{
  mDiffCount = GetDiffCount();
  if (qAbs(mDiffCount - mDiffCountReported) >= kDiffCountReportThreshold) {
    mDiffCountReported = mDiffCount;
    int thresholdYes = mDiffAvgHyst.GetValue(500);
    int thresholdNo = mDiffAvgHyst.GetValue(100);
    if (thresholdYes - thresholdNo <= 32) {
      Log.Info(QString("Diff is %1 (good: %2, bad: %3)").arg(mDiffCountReported).arg(thresholdYes).arg(thresholdNo));
    } else {
      int thresholdMidOff = (thresholdYes + 2*thresholdNo)/3;
      int thresholdMidOn = (thresholdYes + thresholdNo)/2;
      Log.Info(QString("Diff is %1 (on: %2, off: %3)").arg(mDiffCountReported).arg(thresholdMidOn).arg(thresholdMidOff));
    }
  }
  if (mDiffCount <= 0) {
    return false;
  }
  mDiffAvgHyst.Inc(mDiffCount);
//  Log.Debug(QString("diff: %1").arg(mDiffCount));
  return mDiffAvgHyst.TotalCount() > 2000;
}

bool AnalyticsB::TrySwitchStandByOn()
{
  if (!InitStandByCalc()) {
    return false;
  }

//  int thresholdYes = mDiffAvgHyst.GetValue(500);
//  int thresholdNo = mDiffAvgHyst.GetValue(100);
//  if (thresholdYes - thresholdNo <= 32) {
//    return false;
//  }
//  int thresholdMidOff = (thresholdYes + 2*thresholdNo)/3;
//  if (mDiffCount < thresholdMidOff) {
//    Log.Info(QString("Analize will fall asleep (diff: %1, threshold: %2)").arg(mDiffCount).arg(thresholdMidOff));
//    return true;
//  }
  return false;
}

bool AnalyticsB::TrySwitchStandByOff()
{
//  int thresholdYes = mDiffAvgHyst.GetValue(500);
//  int thresholdNo = mDiffAvgHyst.GetValue(100);
//  if (thresholdYes - thresholdNo <= 32) {
//    return false;
//  }
//  int thresholdMidOn = (thresholdYes + thresholdNo)/2;
//  if (mDiffCount > thresholdMidOn) {
//    Log.Info(QString("Analize will awake (diff: %1, threshold: %2)").arg(mDiffCount).arg(thresholdMidOn));
//    return true;
//  }
  return false;
}

bool AnalyticsB::LoadDetectorPoints(const DetectorS& detector, AnalyticsB::PointList& points)
{
  for (int i = 0; ; i++) {
    QString key = QString("Point #") + QString::number(i);
    if (detector->Settings.contains(key)) {
      QString value = detector->Settings[key];
      QStringList values = value.split(QChar(','), QString::SkipEmptyParts);
      if (values.size() == 2) {
        bool ok1, ok2;
        qreal px = values[0].toFloat(&ok1);
        qreal py = values[1].toFloat(&ok2);
        if (ok1 && ok2) {
          Log.Info(QString("Point %1 (%2, %3)").arg(i).arg(px).arg(py));
          points.append(QPointF(px, py));
        }
      }
    } else {
      break;
    }
  }
  return true;
}

bool AnalyticsB::IsPointInDoor(int ii, int jj)
{
  return IsPointInArea(mDoors, ii, jj) >= 0;
}

bool AnalyticsB::IsPointInUin(int ii, int jj)
{
  return IsPointInArea(mUin, ii, jj) >= 0;
}

int AnalyticsB::GetPointZone(int ii, int jj)
{
  return IsPointInArea(mZone, ii, jj);
}

bool AnalyticsB::IsPointInIgnore(int ii, int jj)
{
  return IsPointInArea(mIgnore, ii, jj) >= 0;
}

int AnalyticsB::IsPointInArea(const QVector<PointList>& area, int ii, int jj)
{
  QPointF testScreenPoint(0.5 + ii, 0.5 + jj);
  QPointF testPoint = ScreenToPoint(testScreenPoint);

  for (int iArea = 0; iArea < area.size(); iArea++) {
    const PointList& points = area.at(iArea);

    qreal angle = 0;
    QPointF vectFirst = (points.first() - points.last());
    QPointF vectLast = vectFirst;
    for (int i = 0; i < points.size() - 1; i++) {
      QPointF vectNext = points.at(i + 1) - points.at(i);
      angle += GetAngle(vectLast, vectNext);
      vectLast = vectNext;
    }
    angle += GetAngle(vectLast, vectFirst);

    bool directionIn = (angle > 0);
    QPointF pointl = points.last();
    for (auto itr = points.begin(); itr != points.end(); itr++) {
      const QPointF& point = *itr;

      bool intersect = false;
      QPointF pointMid = (pointl + point) * 0.5;
      QPointF pointl_ = points.last();
      for (auto itr = points.begin(); itr != points.end(); itr++) {
        QPointF point_ = *itr;
        if (GetIntersectPoint(pointl_, point_, pointMid, testPoint)) {
          intersect = true;
          break;
        }
        pointl_ = point_;
      }
      if (!intersect) {
        bool testIn = GetVectMult(point - pointl, testPoint - pointl) > 0;
        if (directionIn == testIn) {
          return iArea;
        } else {
          break;
        }
      }
      pointl = point;
    }
  }
  return -1;
}

void AnalyticsB::MarkPoint(int ii, int jj, BlockInfo::EFlag flag)
{
  if (ii >= 0 && ii < mBlockInfo.Width() && jj >= 0 && jj < mBlockInfo.Height()) {
    BlockInfo* info = mBlockInfo.Line(jj) + ii;
    info->Flag |= (int)flag;
  }
}

qreal AnalyticsB::GetVectMult(const QPointF& v1, const QPointF& v2)
{
  return v1.x() * v2.y() - v2.x() * v1.y();
}

qreal AnalyticsB::GetScalarMult(const QPointF& v1, const QPointF& v2)
{
  return v1.x() * v2.x() + v1.y() * v2.y();
}

qreal AnalyticsB::GetAngle(const QPointF& v1, const QPointF& v2)
{
  if (v1.manhattanLength() < 0.0001 || v2.manhattanLength() < 0.0001) {
    return 0;
  }
  qreal vSize = GetVectSize(v1) * GetVectSize(v2);
  qreal sinAngle = GetVectMult(v1, v2) / vSize;
  qreal cosAngle = GetScalarMult(v1, v2) / vSize;
  if (qAbs(sinAngle) > 1 || qAbs(cosAngle) > 1) {
    LOG_WARNING_ONCE(QString("Calc error (GetAngle, v1: (%1, %2), v: (%3, %4))").arg(v1.x()).arg(v1.y()).arg(v2.x()).arg(v2.y()));
  }
  return (sinAngle >= 0)? acos(cosAngle): -acos(cosAngle);
}

qreal AnalyticsB::GetVectSize(const QPointF& v)
{
  return sqrt(v.x() * v.x() + v.y() * v.y());
}

QPointF AnalyticsB::PointToScreen(const QPointF& pointPercent) const
{
  return QPointF(pointPercent.x() * (mBlockScene.BlockWidth()), pointPercent.y() * (mBlockScene.BlockHeight()));
}

QPointF AnalyticsB::ScreenToPoint(const QPointF& pointScreen) const
{
  return QPointF(pointScreen.x() / (mBlockScene.BlockWidth()), pointScreen.y() / (mBlockScene.BlockHeight()));
}

void AnalyticsB::AddZone(QVector<AnalyticsB::PointList>& zoneArray, const AnalyticsB::PointList& points)
{
  if (points.size() < 3) {
    return;
  }

  for (int i = 0; i < points.size() - 1; i++) {
    const QPointF& pointAi = points.at(i);
    const QPointF& pointBi = points.at(i + 1);

    qreal tMin = 1.0;
    int newIndex = -1;
    for (int j = i + 1; j < points.size(); j++) {
      const QPointF& pointAj = points.at(j);
      const QPointF& pointBj = (j < points.size() - 1)? points.at(j + 1): points.at(0);
      qreal t;
      if (GetIntersectPoint(pointAi, pointBi, pointAj, pointBj, &t)) {
        if (t < tMin) {
          tMin = t;
          newIndex = j;
        }
      }
    }
    if (newIndex >= 0 && newIndex < points.size()) {
      const QPointF& pointAj = points.at(i);
      const QPointF& pointBj = points.at(i + 1);
      QPointF pointC = pointAj + (pointBj - pointAj) * tMin;
      PointList left;
      for (int j = 0; j <= i; j++) {
        left.append(points.at(j));
      }
      left.append(pointC);
      for (int j = newIndex + 1; j < points.size(); j++) {
        left.append(points.at(j));
      }
      PointList right;
      right.append(pointC);
      for (int j = i + 1; j <= newIndex; j++) {
        right.append(points.at(j));
      }
      AddZone(zoneArray, left);
      AddZone(zoneArray, right);
      return;
    }
  }
  zoneArray.append(points);
}

bool AnalyticsB::GetIntersectPoint(const QPointF& a1, const QPointF& a2, const QPointF& b1, const QPointF& b2, qreal* t)
{
  qreal denum = (b2.y() - b1.y()) * (a2.x() - a1.x()) - (b2.x() - b1.x()) * (a2.y() - a1.y());
  if (qAbs(denum) < 0.00001) {
    return false;
  }
  qreal ta = ( (b2.x() - b1.x()) * (a1.y() - b1.y()) - (b2.y() - b1.y()) * (a1.x() - b1.x()) ) / denum;
  qreal tb = ( (a2.x() - a1.x()) * (a1.y() - b1.y()) - (a2.y() - a1.y()) * (a1.x() - b1.x()) ) / denum;
  if (t) {
    *t = ta;
  }
  return ta > 0 && ta < 1 && tb > 0 && tb < 1;
}

AnalyticsB::AnalyticsB(int _BlockSize)
  : mBlockSize(_BlockSize)
  , mDiffCountReported(-kDiffCountReportThreshold)
  , mSectionCount(0)
  , mImageData(mBlockScene), mBlockInfo(mBlockScene)
  , mDebugData(mBlockScene)
{
  Q_INIT_RESOURCE(Video);

  mLogoImg.reset(new QImage(":/Local/StampLogo.png"));
}

AnalyticsB::~AnalyticsB()
{
}

