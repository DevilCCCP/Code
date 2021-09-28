#pragma once

#include <QString>

#include <LibV/Include/Frame.h>
#include <LibV/Include/ConveyorV.h>


DefineClassS(FfmpegOut);
DefineClassS(FileSaver);

class FileSaver: public ConveyorV
{
  QString       mFilename;
  CtrlWorker*   mParent;

  FfmpegOutS    mFfmpegOut;
  bool          mOpened;

public:
  /*override */virtual const char* Name() override { return "FileSaver"; }
  /*override */virtual const char* ShortName() override { return "S"; }
protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual void DoRelease() override;

public:
//  /*override */virtual void Stop() override;

protected:
  /*override */virtual bool ProcessFrame() override;
  /*override */virtual void OnOverflow(QList<FrameAS>& conveyorFrames) override;

private:
  void Done();

public:
  bool HasOverflow() { return HasOverflowWarn(); }

public:
  FileSaver(const QString& _Filename, CtrlWorker* _Parent);
  /*override */virtual ~FileSaver();
};

