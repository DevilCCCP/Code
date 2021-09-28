#include <qsystemdetection.h>
#include <limits>

#include <Lib/Log/Log.h>
#include <Lib/Common/FpsCalc.h>

#include "SourceFfmpeg.h"
#include "FfmpegIn.h"
#ifndef Q_OS_WIN32
#include "../Linux/LinuxUsbDevice.h"
#endif

const int kCheckUsbMs = 30000;
const int kFileMinimumSleep = 2;

bool SourceFfmpeg::DoInit()
{
  mFfmpeg = FfmpegInS(new FfmpegIn(this, GetThumbnail()));
  if (!mFfmpeg) {
    return false;
  }

  return Source::DoInit();
}

bool SourceFfmpeg::DoCircle()
{
  if (mFilename.isEmpty()) {
    if (!PrepareFilename()) {
      return true;
    }
  }
  if (OpenFile()) {
    mOpenFail = false;
    PlayFile();
    CloseFile();
  } else if (mPlayFile || !mOpenFail) {
    mOpenFail = true;
    Log.Error("Open file fail");
  }
  return true;
}

void SourceFfmpeg::Stop()
{
  mPlayFile = false;
  AbortFile();

  Source::Stop();
}

void SourceFfmpeg::Reconnect()
{
  mPlayFile = false;
  AbortFile();
}

bool SourceFfmpeg::NeedDecoder()
{
  return mType != St::eUsb;
}

void SourceFfmpeg::Init(SettingsA& settings)
{
  const static QString kFilePrefix("file://");
  const static QString kRtspPrefix("rtsp://");
  const static QString kUsbPrefix("usb://");

  QString uri = settings.GetMandatoryValue("Uri", true).toString();
  mFromMs = 0;
  if (TestPrefix(uri, kFilePrefix)) {
    InitFile(settings);
  } else if (TestPrefix(uri, kRtspPrefix)) {
    InitRtsp(settings);
  } else if (TestPrefix(uri, kUsbPrefix)) {
    InitUsb(settings);
  } else {
    Log.Fatal(QString("Create source failed, unknown uri (uri: %1)").arg(uri), true);
  }
}

bool SourceFfmpeg::TestPrefix(const QString& uri, const QString& prefix)
{
  if (uri.startsWith(prefix)) {
    mFilename = uri.mid(prefix.size());
    return true;
  }
  return false;
}

void SourceFfmpeg::InitFile(SettingsA& settings)
{
  Q_UNUSED(settings);

  mType = St::eFile;
  mFromMs = settings.GetValue("From", 0).toLongLong();
  Log.Info(QString("Created source from file (filename: '%1')").arg(mFilename));
}

void SourceFfmpeg::InitRtsp(SettingsA& settings)
{
  QString login = settings.GetValue("Login", "").toString();
  QString pass = settings.GetValue("Password", "").toString();
  mType = (settings.GetValue("Transport", "0").toBool() == 0)? St::eRtspTcp: St::eRtspUdp;
  if (!login.isEmpty()) {
    if (!pass.isEmpty()) {
      mFilename = QString("rtsp://%2:%3@%1").arg(mFilename).arg(login).arg(pass);
    } else {
      mFilename = QString("rtsp://%2@%1").arg(mFilename).arg(login);
    }
  } else {
    mFilename = QString("rtsp://") + mFilename;
  }
  Log.Info(QString("Created source from rtsp uri (uri: '%1', tcp: %2)").arg(mFilename).arg(mType == St::eRtspTcp));
}

void SourceFfmpeg::InitUsb(SettingsA& settings)
{
#ifndef Q_OS_WIN32
  QRegExp usbHubPath("[\\d\\.\\*]+");
  if (usbHubPath.exactMatch(mFilename)) {
    mUsbHubPath = mFilename;
    QString realFilename = LinuxUsbDevice(mUsbHubPath);
    if (!realFilename.isEmpty()) {
      mFilename = realFilename;
      Log.Info(QString("USB device found (pattern: '%1', path: '%2')").arg(mUsbHubPath).arg(mFilename));
    } else {
      Log.Warning(QString("USB device not found (pattern: '%1')").arg(mUsbHubPath));
    }
  }
#endif
  QString resolution = settings.GetValue("Resolution", "").toString();
  int fps = settings.GetValue("Fps", 30).toInt();
  QString format = settings.GetValue("Format", "").toString();
  mSettings = QString("%1|%2|%3").arg(resolution).arg(fps).arg(format);
  mType = St::eUsb;
  Log.Info(QString("Created source from USB device (path: '%1')").arg(mFilename));
}

bool SourceFfmpeg::OpenFile()
{
  Log.Trace("Opening file...");
  if (mFfmpeg->Open(mFilename, mType, mSettings)) {
    Log.Info("File opened");
    if (mFromMs > 0) {
      mFfmpeg->SeekTime(mFromMs);
    }
    mPlayFile = true;
    mFirstFrame = true;
    return true;
  }
  return false;
}

bool SourceFfmpeg::PrepareFilename()
{
  switch (mType) {
#ifndef Q_OS_WIN32
    case St::eUsb:  return InitUsbDevice();
#endif
  case St::eFile:
  default:          break;
  }
  return false;
}

void SourceFfmpeg::PlayFile()
{
  FrameS frame;
  while (mPlayFile && mFfmpeg->ReadNext(frame)) {
    switch (mType) {
    case St::eFile: WaitNextFrame(frame->GetHeader()->Timestamp); break;
#ifndef Q_OS_WIN32
    case St::eUsb:  CheckUsbDevice(); break;
#endif
    default:        break;
    }

    OnFrame(frame);
  }
}

void SourceFfmpeg::CloseFile()
{
  Log.Info("File closed");
}

void SourceFfmpeg::AbortFile()
{
  mFfmpeg->CloseStream();
}

void SourceFfmpeg::WaitNextFrame(const qint64& ts)
{
  if (mFirstFrame) {
    mFileStartTs = ts;
    mFrameTimer.start();
    mFirstFrame = false;
  }
  qint64 fileTs = ts - mFileStartTs;
  qint64 realTs = mFrameTimer.elapsed();
  if (fileTs > realTs) {
    msleep(qMin(fileTs - realTs, (qint64)500));
  } else {
    msleep(kFileMinimumSleep);
  }
}

void SourceFfmpeg::CheckUsbDevice()
{
#ifndef Q_OS_WIN32
  if (mFirstFrame) {
    mFrameTimer.start();
    mFirstFrame = false;
  }
  if (mFrameTimer.elapsed() > kCheckUsbMs) {
    QString newFilename = LinuxUsbDevice(mUsbHubPath);
    if (!newFilename.isEmpty() && mFilename != newFilename) {
      mFilename = newFilename;
      Reconnect();
    }
    mFrameTimer.restart();
  }
#endif
}

bool SourceFfmpeg::InitUsbDevice()
{
#ifndef Q_OS_WIN32
  mFilename = LinuxUsbDevice(mUsbHubPath);
  if (!mFilename.isEmpty()) {
    Log.Info(QString("USB device found (number %1, path: '%2')").arg(mUsbHubPath).arg(mFilename));
    return true;
  }
#endif
  return false;
}


SourceFfmpeg::SourceFfmpeg(SettingsA &settings)
  : Source()
  , mOpenFail(false), mPlayFile(false)
{
  Init(settings);
}

SourceFfmpeg::~SourceFfmpeg()
{
}
