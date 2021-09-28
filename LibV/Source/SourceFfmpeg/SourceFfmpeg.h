#pragma once

#include <QSharedPointer>
#include <QElapsedTimer>

#include <Lib/Settings/SettingsA.h>
#include <LibV/Include/Frame.h>

#include "../Source.h"


DefineClassS(SourceFfmpeg);
DefineClassS(FfmpegIn);
DefineClassS(Thumbnail);

class SourceFfmpeg: public Source
{
  QString       mFilename;
  St::EType     mType;
  FfmpegInS     mFfmpeg;
  QString       mSettings;
  bool          mOpenFail;
  volatile bool mPlayFile;

  QString       mUsbHubPath;
  qint64        mFromMs;

  bool          mFirstFrame;
  QElapsedTimer mFrameTimer;
  qint64        mFileStartTs;

public:
  /*override */virtual const char* Name() override { return "SourceFfmpeg"; }
  /*override */virtual const char* ShortName() override { return "S"; }
protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
//  /*override */virtual void DoRelease() override;

  /*override */virtual void Stop() override;

protected:
  /*override */virtual void Reconnect() override;

public:
  /*override */virtual bool NeedDecoder() override;

private:
  void Init(SettingsA& settings);
  bool TestPrefix(const QString& uri, const QString& prefix);
  void InitFile(SettingsA& settings);
  void InitRtsp(SettingsA& settings);
  void InitUsb(SettingsA& settings);
  bool OpenFile();
  bool PrepareFilename();
  void PlayFile();
  void CloseFile();
  void AbortFile();

  void WaitNextFrame(const qint64& ts);
  void CheckUsbDevice();
  bool InitUsbDevice();

public:
  SourceFfmpeg(SettingsA& settings);
  /*override */virtual ~SourceFfmpeg();
};
