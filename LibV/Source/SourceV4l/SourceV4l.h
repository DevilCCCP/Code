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
  QString       mUsbHubPath;
  QString       mFilename;
  V4lInS        mV4lIn;
  QString       mResolution;
  QString       mFps;
  bool          mOpenFail;
  volatile bool mPlayFile;

  bool          mFirstFrame;
  QElapsedTimer mFrameTimer;

public:
  /*override */virtual const char* Name() override { return "SourceV4l"; }
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
