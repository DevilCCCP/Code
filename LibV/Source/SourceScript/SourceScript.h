#pragma once

#include <QSharedPointer>

#include <Lib/Settings/SettingsA.h>
#include <LibV/Include/Frame.h>
#include "../Source.h"

DefineClassS(ScriptIn);
DefineClassS(SourceScript);


class SourceScript: public Source
{
  QString       mFilename;
  ScriptInS     mScript;

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "SourceScript"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "S"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
//  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;

  /*override */virtual void Stop() Q_DECL_OVERRIDE;

protected:
  /*override */virtual void Reconnect() Q_DECL_OVERRIDE;

public:
  /*override */virtual bool NeedDecoder() Q_DECL_OVERRIDE { return false; }

private:
  bool OpenFile();
  void PlayFile();

public:
  SourceScript(SettingsA& settings);
  /*override */virtual ~SourceScript();
};
