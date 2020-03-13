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
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "FileSaver"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "S"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;

public:
//  /*override */virtual void Stop() Q_DECL_OVERRIDE;

protected:
  /*override */virtual bool ProcessFrame() Q_DECL_OVERRIDE;
  /*override */virtual void OnOverflow(QList<FrameAS>& conveyorFrames) Q_DECL_OVERRIDE;

private:
  void Done();

public:
  bool HasOverflow() { return HasOverflowWarn(); }

public:
  FileSaver(const QString& _Filename, CtrlWorker* _Parent);
  /*override */virtual ~FileSaver();
};

