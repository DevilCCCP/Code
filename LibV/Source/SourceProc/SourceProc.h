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
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "SourceProc"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "S"; }
protected:
//  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;

//  /*override */virtual void Stop() Q_DECL_OVERRIDE;

protected:
  /*override */virtual void Reconnect() Q_DECL_OVERRIDE;

public:
  /*override */virtual bool NeedDecoder() Q_DECL_OVERRIDE;

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
