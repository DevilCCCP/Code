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
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "UpSync"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "S"; }
protected:
//  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
//  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;

private:
  bool SyncUpdate();
  bool StartUpdate();

public:
  explicit UpSync(Updater* _Updater, const UpInfoS& _UpInfo);
};

