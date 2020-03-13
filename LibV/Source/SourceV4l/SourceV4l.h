#pragma once

#include <QSharedPointer>
#include <QElapsedTimer>

#include <Lib/Settings/SettingsA.h>
#include <LibV/Include/Frame.h>

#include "../Source.h"


DefineClassS(SourceV4l);
DefineClassS(V4lIn);
DefineClassS(Thumbnail);

class SourceV4l: public Source
{
  int           mUsbDevice;
  QString       mFilename;
  V4lInS        mV4lIn;
  QString       mResolution;
  QString       mFps;
  bool          mOpenFail;
  volatile bool mPlayFile;

  bool          mFirstFrame;
  QElapsedTimer mFrameTimer;

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "SourceV4l"; }
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
  bool OpenFile();
  bool PrepareFilename();
  void PlayFile();
  void CloseFile();
  void AbortFile();

  void CheckUsbDevice();
  bool InitUsbDevice();

public:
  SourceV4l(SettingsA& settings);
  /*override */virtual ~SourceV4l();
};
