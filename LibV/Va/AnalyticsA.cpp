#include <QDir>
#include <QFile>
#include <QImage>
#include <QPainter>

#include <Lib/Log/Log.h>
#include <Lib/Common/Var.h>
#include <Lib/Common/Format.h>
#include <Lib/Ctrl/CtrlManager.h>
#include <Lib/Db/Files.h>
#include <Lib/Db/VaStatType.h>
#include <Lib/Db/VaStat.h>
#include <Lib/Db/VaStatHours.h>
#include <Lib/Db/VaStatDays.h>
#include <LibV/Include/Region.h>

#include "AnalyticsA.h"
#ifdef ANAL_DEBUG
#include "DebugWnd.h"
#endif


#ifdef QT_NO_DEBUG
const qint64 kSettingsSyncPeriod = 15 * 60 * 1000;
const int kMinStatMs = 2 * 60 * 60 * 1000;
#else
const qint64 kSettingsSyncPeriod = 60 * 1000;
const int kMinStatMs = 2 * 60 * 1000;
#endif
const int kResetStatInfoMs = 5 * 60 * 1000;
const int kStatTypeCount = 4;

void AnalyticsA::SetId(int _Id)
{
  mId = _Id;
}

void AnalyticsA::SetDb(const DbS& _Db)
{
  mFilesTable.reset(new FilesTable(*_Db));
  mVaStatTypeTable.reset(new VaStatTypeTable(*_Db));
  mVaStatTable.reset(new VaStatTable(*_Db));
  mVaStatHoursTable.reset(new VaStatHoursTable(*_Db));
  mVaStatDaysTable.reset(new VaStatDaysTable(*_Db));
}

void AnalyticsA::SetSettings(const SettingsAS& _Settings)
{
  mUseStandBy = _Settings->GetValue("Standby", false).toBool();
  QString statCreateText = _Settings->GetValue("StatCreate", "23:55").toString();
  auto statCreateTextPair = statCreateText.split(':');
  int h = 23;
  int m = 55;
  if (statCreateTextPair.size() == 2) {
    bool ok1, ok2;
    int h_ = statCreateTextPair.at(0).toInt(&ok1);
    int m_ = statCreateTextPair.at(1).toInt(&ok2);
    if (ok1 && ok2 && h_ >= 0 && h_ < 24 && m_ >= 0 && m_ < 60) {
      h = h_;
      m = m_;
    }
  }
  mStatCreateMs = (h * 60 + m) * 60000;
  SwitchStandByOff();

  InitSettings(_Settings);
}

void AnalyticsA::SetDetectors(const QList<DetectorS>& _Detectors)
{
  InitDetectors(_Detectors);
}

void AnalyticsA::SetDebug(CtrlManager* _CtrlManager)
{
#ifndef ANAL_DEBUG
  Q_UNUSED(_CtrlManager);
#else
  if (_CtrlManager) {
    mDebugWnd = DebugWndS(new DebugWnd());
    _CtrlManager->RegisterWorker(mDebugWnd);
  }
#endif
}

bool AnalyticsA::GetStatAbbr(int type, QString& abbr)
{
  Q_UNUSED(type);
  Q_UNUSED(abbr);

  return false;
}

int AnalyticsA::GetStatTimeMs(int type)
{
  Q_UNUSED(type);

  return 0;
}

bool AnalyticsA::GetStatImage(int type, QByteArray& image)
{
  Q_UNUSED(type);
  Q_UNUSED(image);

  return false;
}

void AnalyticsA::ResetStat(int type)
{
  Q_UNUSED(type);
}

int AnalyticsA::GetDebugFrameCount()
{
  return 5;
}

bool AnalyticsA::GetDebugFrame(const int index, QString &text, EImageType &imageType, uchar *data, bool &save)
{
  Q_UNUSED(index);
  Q_UNUSED(text);
  Q_UNUSED(imageType);
  Q_UNUSED(data);
  Q_UNUSED(save);

  return false;
}

void AnalyticsA::Finish()
{
  SyncSettings();
}

void AnalyticsA::Init()
{
  QString abbr;
  mUsingStats = GetStatAbbr(0, abbr);
}

bool AnalyticsA::AnalizeFrame(Frame &frame)
{
  Frame::Header* header = frame.GetHeader();
  if (header->VideoDataSize == 0) {
    return false;
  }
//  static int gCounter = 0;
//  if (++gCounter <= 10) {
//    QString filename = QString("file_%1.bin").arg(gCounter, 3, 10, QChar('0'));
//    QFile file(filename);
//    if (file.open(QFile::WriteOnly)) {
//      file.write(frame.VideoData(), frame.VideoDataSize());
//    }
//  }
  int stride = 0;
  if (header->Compression == eRawNv12 || header->Compression == eRawYuv) {
    stride = header->VideoDataSize / (header->Height * 3 / 2);
  } else if (header->Compression == eRawYuvP) {
    stride = header->VideoDataSize / header->Height;
  } else if (header->Compression == eRawNv12A) {
    stride = (header->Width + 0x1f) & (~0x1f);
  } else {
    if (!mWarning) {
      Log.Error(QString("Unimplemeted image type (compr: %1)").arg(header->Compression));
      mWarning = true;
    }
    return false;
  }

  if (header->Compression != mCompression || header->Width != mWidth || header->Height != mHeight || stride != mStride) {
    SyncSettings();
    mCompression = header->Compression;
    mWidth = header->Width;
    mHeight = header->Height;
    mStride = stride;
    if (!OnUpdateDimentions()) {
      return false;
    }
  }
  //static int gCounter = 0;
  //Log.Trace(QString("Counter: %1").arg(++gCounter));
  bool analized = Analize(header->Timestamp, reinterpret_cast<const uchar*>(frame.VideoData()));
#ifdef ANAL_DEBUG
  if (mDebugWnd) {
    mDebugWnd->SetDimentions(mWidth, mHeight, mStride);
    mDebugWnd->SetFrameCount(GetDebugFrameCount());

    Object obj;
    mDebugObj.resize(0);
    if (analized) {
      while (HaveNextObject() && RetrieveNextObject(obj)) {
        mDebugObj.append((char*)&obj, sizeof(Object));
      }
    }
    QByteArrayS srcFrame(new QByteArray());
    srcFrame->resize(mHeight * mStride);
    switch (mCompression) {
    case eRawYuvP:
      for (int j = 0; j < mHeight; j++) {
        const char* src = frame.VideoData() + mStride * j;
        char* dst = srcFrame->data() + mStride * j;
        for (int i = 0; i < mWidth; i++) {
          *dst = *src;

          src += 2;
          dst++;
        }
      }
      break;
    case eRawNv12:
    case eRawNv12A:
    case eRawYuv:
      memcpy(srcFrame->data(), frame.VideoData(), mHeight * mStride);
      break;
    default:
      LOG_WARNING_ONCE(QString("No convertion to Y from %1").arg(CompressionToString(mCompression)));
      memset(srcFrame->data(), 0, mHeight * mStride);
      break;
    }
    mDebugWnd->DrawWindow(0, "Source Y", eValue, 0, srcFrame, mStride, mDebugObj.constData(), mDebugObj.size());

    QString caption;
    EImageType imageType;
    for (int i = 1; true; i++) {
      QByteArrayS frame(new QByteArray());
      frame->resize(qMax(mHeight * mWidth, 256 * 255 * (int)sizeof(int)));
      bool save = false;
      if (!GetDebugFrame(i, caption, imageType, reinterpret_cast<uchar*>(frame->data()), save)) {
        break;
      }
      mDebugWnd->DrawWindow(i, caption, imageType, save? mCurrentFrame: 0, frame, mWidth);
    }
  }
#endif
  return analized;
}

bool AnalyticsA::CreateSnapshot(const Region<uchar>& region, QByteArray& data) const
{
  QImage img(region.Width(), region.Height(), QImage::Format_ARGB32);
  for (int j = 0; j < region.Height(); j++) {
    const uchar* src = region.Line(j);
    QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(j));
    for (int i = 0; i < region.Width(); i++) {
      int g = *src;
      *line = QColor(g, g, g).rgb();

      src++;
      line++;
    }
  }

  QBuffer s(&data);
  if (s.open(QBuffer::WriteOnly)) {
    img.save(&s, "jpg", 80);
    return true;
  }
  return false;
}

bool AnalyticsA::CreateSnapshotNv12(const Region<uchar>& regionY, const Region<uchar>& regionUv, QByteArray& data) const
{
  QImage img(regionY.Width(), regionY.Height(), QImage::Format_ARGB32);
  for (int j = 0; j < regionY.Height(); j++) {
    const uchar* srcY = regionY.Line(j);
    const uchar* srcUv = regionUv.Line(j/2);
    QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(j));
    for (int i = 0; i < regionY.Width(); i++) {
      int y = (int)*srcY;
      int u = (int)srcUv[0];
      int v = (int)srcUv[1];

      int r = qBound(0, y + 1407 * (v - 128) / 1000, 255);
      int g = qBound(0, y + (-345 * (u - 128) - (717 * (v - 128))) / 1000, 255);
      int b = qBound(0, y + 1779 * (u - 128) / 1000, 255);
      *line = QColor(r, g, b).rgb();

      if ((i % 2) == 0) {
        srcY++;
      } else {
        srcY++;
        srcUv += 2;
      }
      line++;
    }
  }

  QBuffer s(&data);
  if (s.open(QBuffer::WriteOnly)) {
    img.save(&s, "jpg", 80);
    return true;
  }
  return false;
}

bool AnalyticsA::CreateImageNv12(const Region<uchar>& regionY, const Region<uchar>& regionUv, QImage& img) const
{
  img = QImage(regionY.Width(), regionY.Height(), QImage::Format_ARGB32);
  for (int j = 0; j < regionY.Height(); j++) {
    const uchar* srcY = regionY.Line(j);
    const uchar* srcUv = regionUv.Line(j/2);
    QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(j));
    for (int i = 0; i < regionY.Width(); i++) {
      int y = (int)*srcY;
      int u = (int)srcUv[0];
      int v = (int)srcUv[1];

      int r = qBound(0, y + 1407 * (v - 128) / 1000, 255);
      int g = qBound(0, y + (-345 * (u - 128) - (717 * (v - 128))) / 1000, 255);
      int b = qBound(0, y + 1779 * (u - 128) / 1000, 255);
      *line = QColor(r, g, b).rgb();

      if ((i % 2) == 0) {
        srcY++;
      } else {
        srcY++;
        srcUv += 2;
      }
      line++;
    }
  }
  return true;
}

void AnalyticsA::SwitchStandByOn()
{
  mStandByMode = true;
}

void AnalyticsA::SwitchStandByOff()
{
  mStandByMode = false;
  mStartTimestamp = 0;
}

void AnalyticsA::SetCurrentAsBackImage()
{
  if (mWidth == mStride) {
    const uchar* src = mImageData;
    char*       dest = mBackImage.data();
    memcpy(dest, src, mWidth * mHeight);
  } else {
    for (int j = 0; j < mHeight; j++) {
      const uchar* src = mImageData + mStride * j;
      char*       dest = mBackImage.data() + mWidth * j;
      memcpy(dest, src, mWidth);
    }
  }
  mHasBackImage = true;
}

bool AnalyticsA::SyncBackImage()
{
  return SyncImage("StableBackground", mBackImage);
}

bool AnalyticsA::RestoreBackImage()
{
  return mHasBackImage = RestoreImage("StableBackground", mBackImage);
}

QString AnalyticsA::GetVaFilename(const char* name) const
{
  return GetVarFileBin(QString("%1_%2").arg(getId(), 6, 10, QChar('0')).arg(name));
}

bool AnalyticsA::SyncImage(const char* name, const QByteArray& data) const
{
  return SyncImage(name, data.constData(), data.size());
}

bool AnalyticsA::SyncImage(const char* name, const char* data, int dataSize) const
{
  QFile file(GetVaFilename(name));
  if (!file.open(QFile::WriteOnly)) {
    return false;
  }
  if (file.write(data, dataSize) != dataSize) {
    return false;
  }
  return file.flush();
}

bool AnalyticsA::RestoreImage(const char* name, QByteArray& data) const
{
  return RestoreImage(name, data.data(), data.size());
}

bool AnalyticsA::RestoreImage(const char* name, char* data, int dataSize) const
{
  QFile file(GetVaFilename(name));
  if (!file.open(QFile::ReadOnly) || file.size() != dataSize) {
    return false;
  }
  return (file.read(data, dataSize) == dataSize);
}

void AnalyticsA::GetDbgBackImage(uchar* data)
{
  memcpy(data, mBackImage.constData(), mBackImage.size());
}

void AnalyticsA::GetDbgSource(uchar* data)
{
  if (mWidth == mStride) {
    memcpy(data, mImageData, mHeight * mWidth);
    return;
  }

  Region<uchar> dbgRegion;
  dbgRegion.SetSource(data, mWidth, mHeight, mWidth);
  Region<uchar> srcRegion;
  srcRegion.SetSource(const_cast<uchar*>(mImageData), mWidth, mHeight, mStride);
  for (int j = 0; j < mHeight; j++) {
    memcpy(dbgRegion.Line(j), srcRegion.Line(j), mWidth);
  }
}

void AnalyticsA::GetDbgSourceWithLine(int line, uchar* data)
{
  Region<uchar> dbgRegion;
  dbgRegion.SetSource(data, mWidth, mHeight, mWidth);
  Region<uchar> srcRegion;
  srcRegion.SetSource(const_cast<uchar*>(mImageData), mWidth, mHeight, mStride);
  for (int j = 0; j < mHeight; j++) {
    if (j == line + 1) {
      memset(dbgRegion.Line(j), 0, mWidth);
    } else {
      memcpy(dbgRegion.Line(j), srcRegion.Line(j), mWidth);
    }
  }
}

void AnalyticsA::GetDbgSourceLine(int line, uchar* data)
{
  Region<uchar> dbgRegion;
  dbgRegion.SetSource(data, mWidth, mHeight, mWidth);
  Region<uchar> srcRegion;
  srcRegion.SetSource(const_cast<uchar*>(mImageData), mWidth, mHeight, mStride);

  dbgRegion.ZeroData();
  int j = line;
  const uchar* src = srcRegion.Line(j);
  int axe = dbgRegion.Height()-1;

  for (int i = 0; i < srcRegion.Width(); i++) {
    int val = axe - *src * axe / 255;
    uchar* dbg = dbgRegion.Line(0) + i;
    for (int k = 0; k < val; k++) {
      *dbg = 1;
      dbg += dbgRegion.Stride();
    }

    src++;
  }
}

void AnalyticsA::UpdateStats(const qint64& timestamp)
{
  if (!mUsingStats) {
    return;
  }

  if (!mStatTimestamp) {
    mStatTimestamp = getLocalSettings()->GetValue("StatTimestamp", 0).toLongLong();
    mStatDone = getLocalSettings()->GetValue("StatDone", false).toBool();
    PrepareStatTimestamp(mStatTimestamp);
  }
  if (timestamp < mStatTimestamp - 5 * 60 * 1000) {
    Log.Warning(QString("Discard stats (ts: %1, stats: [%2, %3])")
                .arg(QDateTime::fromMSecsSinceEpoch(timestamp).toString())
                .arg(QDateTime::fromMSecsSinceEpoch(mStatTimestamp).toString())
                .arg(QDateTime::fromMSecsSinceEpoch(mStatEndTimestamp).toString()));
    ResetStats();
    PrepareStatTimestamp(timestamp);
  }

  if (timestamp > mStatMakeTimestamp) {
    if (!mStatDone) {
      if (MakeStats()) {
        mStatMakeTimestamp = mStatEndTimestamp;
        PrepareStatTimestamp(timestamp);
      } else {
        mStatMakeTimestamp += 2 * 1000;
        mStatEndTimestamp  += 2 * 1000;
      }
    }
    if (timestamp > mStatEndTimestamp) {
      mStatDone = false;
      Log.Info(QString("End stats without create"));

      PrepareStatTimestamp(timestamp);
    }
  }
}

void AnalyticsA::ResetStats()
{
  for (int i = 0; i  < kStatTypeCount; i++) {
    ResetStat(i);
  }
}

bool AnalyticsA::MakeStats()
{
  for (int i = 0; i  < kStatTypeCount; i++) {
    QString abbr;
    if (!GetStatAbbr(i, abbr)) {
      break;
    }
    VaStatTypeS statType;
    if (!mVaStatTypeTable->GetByAbbr(abbr, statType)) {
      LOG_WARNING_ONCE(QString("VA stat not found (abbr: '%1')").arg(abbr));
      return false;
    }

    int ms = GetStatTimeMs(i);
    if (ms > kMinStatMs) {
      Log.Info(QString("Make stat image (abbr: '%1', time: %2)").arg(abbr).arg(FormatTime(ms)));
      if (GetStatImage(i, mStatImage)) {
        Log.Info(QString("Make stat image ok"));
        if (!SaveStatImage(statType->Id)) {
          Log.Warning(QString("Save stat image fail"));
          return false;
        }
      } else {
        Log.Warning(QString("Make stat image fail"));
      }
    } else if (ms > kResetStatInfoMs) {
      Log.Info(QString("Reset stat image (abbr: '%1', time: %2)").arg(abbr).arg(FormatTime(ms)));
    }

    ResetStat(i);
  }
  return true;
}

bool AnalyticsA::SaveStatImage(int statTypeId)
{
  VaStatS statItem;
  if (!mVaStatTable->GetSimple(mId, statTypeId, statItem)) {
    return false;
  }

  FilesS fileItem(new Files());
  fileItem->Data = mStatImage;
  fileItem->ObjectId = mId;
  if (!mFilesTable->Insert(fileItem)) {
    return false;
  }

  VaStatDaysS statDayItem(new VaStatDays());
  statDayItem->VstatId = statItem->Id;
  statDayItem->FimageId = fileItem->Id;
  statDayItem->Day = QDateTime::fromMSecsSinceEpoch(mStatTimestamp);
  if (!mVaStatDaysTable->Insert(statDayItem)) {
    mFilesTable->Delete(fileItem->Id);
    return false;
  }
  return true;
}

void AnalyticsA::PrepareStatTimestamp(const qint64& timestamp)
{
  QDateTime ts = QDateTime::fromMSecsSinceEpoch(timestamp + 2 * 60 * 1000);
  qint64 oldStatTimestamp = mStatTimestamp;
  mStatTimestamp = QDateTime(ts.date(), QTime(0, 0)).toMSecsSinceEpoch();
  mStatMakeTimestamp = mStatTimestamp + mStatCreateMs;
  mStatEndTimestamp = QDateTime(ts.date(), QTime(23, 59, 59)).toMSecsSinceEpoch();
  if (mStatTimestamp != oldStatTimestamp) {
    mStatDone = false;
    getLocalSettings()->SetValue("StatTimestamp", mStatTimestamp);
    getLocalSettings()->SetValue("StatDone", mStatDone);
    getLocalSettings()->Sync();

    Log.Info(QString("Begin stats [%1, %2], create %3")
             .arg(QDateTime::fromMSecsSinceEpoch(mStatTimestamp).toString())
             .arg(QDateTime::fromMSecsSinceEpoch(mStatEndTimestamp).toString())
             .arg(QDateTime::fromMSecsSinceEpoch(mStatMakeTimestamp).toString()));
  }
}

bool AnalyticsA::OnUpdateDimentions()
{
  mHasBackImage = false;
  mBackImage.resize(mWidth * mHeight);
  mStatImage.resize(mWidth * mHeight);

  if (!InitImageParameters(mWidth, mHeight, mStride)) {
    mCompression = eCmprNone;
    if (!mWarning) {
      Log.Error(QString("SetImageParameters fail (compr: %1, w: %2, h: %3, s: %4)")
                .arg(mCompression).arg(mWidth).arg(mHeight).arg(mStride));
      mWarning = true;
    }
    return false;
  }

  Log.Info(QString("Set image parameters (size: %1x%2, codec: %3)").arg(mWidth).arg(mHeight).arg(CompressionToString(mCompression)));
  Reset();
  mSyncSettings.start();
  return true;
}

bool AnalyticsA::Analize(const qint64& timestamp, const uchar* imageData)
{
  mImageData = imageData;
  AnalizePrepare(imageData);

  if (!mStartTimestamp) {
    if (!mHasBackImage) {
      SetCurrentAsBackImage();
    }
    InitTime(timestamp);
    AnalizeInit();
    return false;
  }

  UpdateTime(timestamp);

  if (mStandByMode) {
    if (mFrameMs < 1000) {
      return false;
    }
    AnalizeStandBy();
  } else {
    AnalizeNormal();
  }

  if (mSyncSettings.elapsed() > kSettingsSyncPeriod) {
    SyncSettings();
  }
  return true;
}

void AnalyticsA::Reset()
{
  mStartTimestamp = 0;
}

void AnalyticsA::InitTime(const qint64& timestamp)
{
  mFrameMs = 0;
  mFrameSec = 0;
  mStartTimestamp = mLastTimestamp = timestamp;
  mCurrentFrame = 0;
}

bool AnalyticsA::UpdateTime(const qint64& timestamp)
{
  UpdateStats(timestamp);

  mCurrentMs = timestamp - mStartTimestamp;
  mFrameMs = timestamp - mLastTimestamp;
  mFrameMs = qMin((qint64)500, mFrameMs);
  mFrameSec = mFrameMs * 0.001f;
  mLastTimestamp = timestamp;
  mCurrentFrame++;
  return true;
}

void AnalyticsA::SyncSettings()
{
  if (mWidth <= 0 || mHeight <= 0) {
    return;
  }

  Log.Info(QString("Sync settings"));
  getLocalSettings()->SetValue("StatTimestamp", mStatTimestamp);
  getLocalSettings()->SetValue("StatDone", mStatDone);
  QString group = QString("%1x%2").arg(mWidth).arg(mHeight);
  if (getLocalSettings()->BeginGroup(group)) {
    SaveVariables(getLocalSettings());
    getLocalSettings()->EndGroup();
  }

  if (!getLocalSettings()->Sync()) {
    Log.Warning(QString("Sync settings fail"));
  }
  mSyncSettings.start();
}


AnalyticsA::AnalyticsA()
  : mCompression(eCmprNone), mWidth(-1), mHeight(-1), mStride(-1), mWarning(false)
  , mHasBackImage(false), mStatTimestamp(0), mStatMakeTimestamp(0), mStatEndTimestamp(0), mStatDone(false)
{
  Reset();
}

AnalyticsA::~AnalyticsA()
{
}
