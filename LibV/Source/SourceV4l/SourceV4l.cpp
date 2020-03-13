#include <qsystemdetection.h>
#include <limits>

#include <Lib/Log/Log.h>
#include <Lib/Common/FpsCalc.h>

#include "SourceV4l.h"
#include "V4lIn.h"
#ifndef Q_OS_WIN32
#include "../Linux/LinuxUsbDevice.h"
#endif

const int kCheckUsbMs = 30000;
const int kFileMinimumSleep = 2;

bool SourceV4l::DoInit()
{
  mV4lIn = V4lInS(new V4lIn(this, GetThumbnail()));
  if (!mV4lIn) {
    return false;
  }

  return Source::DoInit();
}

bool SourceV4l::DoCircle()
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

void SourceV4l::Stop()
{
  mPlayFile = false;
  AbortFile();

  Source::Stop();
}

void SourceV4l::Reconnect()
{
  mPlayFile = false;
  AbortFile();
}

bool SourceV4l::NeedDecoder()
{
  return false;
}

void SourceV4l::Init(SettingsA& settings)
{
  const static QString kV4lPrefix("v4l://");

  QString uri = settings.GetMandatoryValue("Uri", true).toString();
  QString devNumber = uri.mid(kV4lPrefix.size());
  bool ok;
  mUsbDevice = devNumber.toInt(&ok);
  if (!ok) {
    Log.Fatal(QString("Create resolve USB device number (device: %1)").arg(devNumber), true);
  }
  QString realFilename = LinuxUsbDevice(mUsbDevice);
  if (!realFilename.isEmpty()) {
    mFilename = realFilename;
    Log.Info(QString("USB device found (number %1, path: '%2')").arg(mUsbDevice).arg(mFilename));
  } else {
    Log.Info(QString("USB device not found (number %1)").arg(mUsbDevice));
  }
  mResolution = settings.GetValue("Resolution", "").toString();
  mFps = settings.GetValue("Fps", "30").toInt();
  Log.Info(QString("Created source from USB device (path: '%1')").arg(mFilename));
}

bool SourceV4l::OpenFile()
{
  Log.Trace("Opening file...");
  if (mV4lIn->Open(mFilename, mResolution, mFps)) {
    Log.Info("File opened");
    mPlayFile = true;
    mFirstFrame = true;
    return true;
  }
  return false;
}

bool SourceV4l::PrepareFilename()
{
  return InitUsbDevice();
}

void SourceV4l::PlayFile()
{
  FrameS frame;
  while (mPlayFile && mV4lIn->ReadNext(frame)) {
    CheckUsbDevice();

    OnFrame(frame);
  }
}

void SourceV4l::CloseFile()
{
  Log.Info("File closing");
  mV4lIn->Close();
}

void SourceV4l::AbortFile()
{
  mV4lIn->Abort();
}

void SourceV4l::CheckUsbDevice()
{
  if (mFirstFrame) {
    mFrameTimer.start();
    mFirstFrame = false;
  }
  if (mFrameTimer.elapsed() > kCheckUsbMs) {
    QString newFilename = LinuxUsbDevice(mUsbDevice);
    if (!newFilename.isEmpty() && mFilename != newFilename) {
      mFilename = newFilename;
      Reconnect();
    }
    mFrameTimer.restart();
  }
}

bool SourceV4l::InitUsbDevice()
{
  mFilename = LinuxUsbDevice(mUsbDevice);
  if (!mFilename.isEmpty()) {
    Log.Info(QString("USB device found (number %1, path: '%2')").arg(mUsbDevice).arg(mFilename));
    return true;
  }
  return false;
}


SourceV4l::SourceV4l(SettingsA &settings)
  : Source()
  , mUsbDevice(-1), mOpenFail(false), mPlayFile(false)
{
  Init(settings);
}

SourceV4l::~SourceV4l()
{
}
