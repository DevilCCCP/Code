#pragma once

#include <QString>
#include <QElapsedTimer>

#include <Lib/Dispatcher/Imp.h>
#include <Lib/Common/Version.h>


DefineClassS(UpChecker);
DefineClassS(PackLoaderA);
DefineClassS(Updater);

class UpChecker: public Imp
{
  Updater*      mUpdater;
  ObjectItemS   mPointItem;

  Version       mUpVersion;
  Version       mCurrentVersion;
  int           mPointId;
  int           mRevision;
  int           mPeriodMs;
  QElapsedTimer mCheckTimer;
  bool          mUnreach;
  bool          mError;

  PackLoaderAS  mLoader;

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "UpdateChecker"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "Ch"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
//  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;
public:
  /*override */virtual void Stop() Q_DECL_OVERRIDE;

public:
  void UpdatePoint(const ObjectItemS& item);

private:
  bool ReloadPointInfo(const ObjectItemS& item);
  bool CheckUpdate();
  bool LoadUpdate();

public:
  explicit UpChecker(Updater* _Updater, const ObjectItemS& _PointItem);
};

