#pragma once

#include <QSharedPointer>
#include <QElapsedTimer>

#include <Lib/Settings/SettingsA.h>
#include <LibV/Include/Frame.h>

#include "../Source.h"


DefineClassS(SourceProc);
DefineClassS(Thumbnail);
DefineClassS(QProcess);

class SourceProc: public Source
{
  QString       mProcessExe;
  QString       mProcessParams;
  QString       mProcessFolder;

  QProcessS     mProcess;
  volatile bool mRestartProcess;

public:
  /*override */virtual const char* Name() override { return "SourceProc"; }
  /*override */virtual const char* ShortName() override { return "S"; }
protected:
//  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
  /*override */virtual void DoRelease() override;

//  /*override */virtual void Stop() override;

protected:
  /*override */virtual void Reconnect() override;

public:
  /*override */virtual bool NeedDecoder() override;

private:
  bool CheckProcess();
  bool StartProcess();
  bool StopProcess();

  bool DoPath();
  bool DoFile(const QString& path);

public:
  SourceProc(SettingsA& settings);
  /*override */virtual ~SourceProc();
};
