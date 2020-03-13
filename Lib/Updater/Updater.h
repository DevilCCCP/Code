#pragma once

#include <QMutex>
#include <QString>
#include <QElapsedTimer>

#include <Lib/Dispatcher/Imp.h>
#include <Lib/Db/Db.h>
#include <Lib/Common/Version.h>


DefineClassS(Updater);
DefineClassS(UpChecker);
DefineClassS(UpInfo);
DefineClassS(UpSync);

enum EUpdateState {
  eUpWait,
  eUpReady,
  eUpSync,
  eUpStarting
};

class Updater: public Imp
{
  DbS                   mDb;
  ObjectTableS          mObjectTable;
  ObjectTypeTableS      mObjectTypeTable;
  ObjectSettingsTableS  mObjectSettingsTable;
  int                   mPointTypeId;
  int                   mDefaultPointId;

  qint64                mNextValidateInfo;
  qint64                mNextReloadPoints;
  bool                  mDbConnected;
  QList<ObjectItemS>    mUpdatePoints;
  QMap<int, UpCheckerS> mCheckers;

  QMutex                mUpdateMutex;
  EUpdateState          mUpdateState;
  Version               mVersion;
  QString               mUpdatePath;
  QString               mInstallerPath;

  UpInfoS               mUpInfo;
  UpSyncS               mUpSync;

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "Updater"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "U"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;

private:
  void CheckUpdate();
  bool CreateSync();

public: /*external thread*/
  void GetCurrentVersion(Version& version);
  void OnUpdateAvailable(const Version& newVersion, const QString& updatePath, const QString& installerPath);
  void OnUpdateSync();

private:
  void DoWork();

  void AddChecker(const ObjectItemS& item);
  void RemoveChecker(int id);

  bool NotifyNewUpdate(const Version& newVersion, const QString& updatePath, const QString& installerPath);
  bool NotifyUpdateSync();
  bool DoUpdate();
  void WaitInstaller();

public:
  explicit Updater(const DbS& _Db);
};

