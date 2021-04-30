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

  QString       mUsbDevice;
  qint64        mFromMs;

  bool          mFirstFrame;
  QElapsedTimer mFrameTimer;
  qint64        mFileStartTs;

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "SourceFfmpeg"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "S"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
//  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;

  /*override */virtual void Stop() Q_DECL_OVERRIDE;

protected:
  /*override */virtual void Reconnect() Q_DECL_OVERRIDE;

public:
  /*override */virtual bool NeedDecoder() Q_DECL_OVERRIDE;

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
