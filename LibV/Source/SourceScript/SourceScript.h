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
  /*override */virtual const char* Name() override { return "SourceScript"; }
  /*override */virtual const char* ShortName() override { return "S"; }
protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
//  /*override */virtual void DoRelease() override;

  /*override */virtual void Stop() override;

protected:
  /*override */virtual void Reconnect() override;

public:
  /*override */virtual bool NeedDecoder() override { return false; }

private:
  bool OpenFile();
  void PlayFile();

public:
  SourceScript(SettingsA& settings);
  /*override */virtual ~SourceScript();
};
