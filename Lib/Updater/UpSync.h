#pragma once

#include <QDateTime>
#include <QElapsedTimer>

#include <Lib/Dispatcher/Imp.h>

#include "UpInfo.h"


DefineClassS(UpSync);
DefineClassS(Updater);

class UpSync: public Imp
{
  Updater*       mUpdater;
  UpInfoS        mUpInfo;

  qint64         mNextCheck;
  bool           mUserWait;

public:
  /*override */virtual const char* Name() override { return "UpSync"; }
  /*override */virtual const char* ShortName() override { return "S"; }
protected:
//  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
//  /*override */virtual void DoRelease() override;

private:
  bool SyncUpdate();
  bool StartUpdate();

public:
  explicit UpSync(Updater* _Updater, const UpInfoS& _UpInfo);
};

