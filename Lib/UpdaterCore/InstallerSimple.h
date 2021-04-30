#pragma once

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QSettings>

#include <Lib/Include/Common.h>

#include "UpdateSettings.h"


enum InstallState {
  eNotChecked,
  eChecking,
  eCheckedFalse,
  eCheckedTrue,
  eCheckError,
  eLoading,
  eLoadTrue,
  eLoadError,
  eInstalling
};

class BackgroundLoader;

class InstallerSimple: public QObject
{
  PROPERTY_GET_SET(QString, UpdateUri)
  PROPERTY_GET_SET(QString, UpdateLogin)
  PROPERTY_GET_SET(QString, UpdatePass)

  UpdateSettings            mUpdateSettings;
  volatile int              mState;
  QElapsedTimer             mActionTimer;
  QTimer*                   mAutoTimer;

  QThread*                  mWorkerThread;
  BackgroundLoader*         mBackgroundLoader;
  bool                      mInstallAfterLoad;

  Q_OBJECT

public:
  InstallState State() const { return (InstallState)mState; }
  bool CanCheck() const { return mState != eChecking && mState != eLoading; }
  bool CanLoad() const { return mState == eCheckedTrue; }
  bool CanInstall() const { return mState == eLoadTrue; }
  bool CanLoadAndInstall() const { return mState == eCheckedTrue || mState == eLoadTrue || mState == eLoadError; }
  bool IsBackgroundRunning() const { return mState == eChecking || mState == eLoading; }
  UpdateSettings* Settings() { return  &mUpdateSettings; }

public:
  void ReloadSettings(QSettings* settings);
  bool StartCheck();
  bool StartLoad();
  bool StartLoadAndInstall();
  bool StartInstall();

private:
  void CheckAutoStart();

  void InitPackageLoader();
  void ChangeState(InstallState newState);

public slots:
  void OnStart();
  void OnExit();

private slots:
  void OnAutoTimerTimeout();
  void OnCheckFinished(int version);
  void OnLoadFinished(QString path);

signals:
  void BackgroundCheck();
  void BackgroundLoad();
  void BackgroundInstall();
  void BackgroundAbort();

  void StateChanged(int state);
  void UpdateAvailable();
  void UpdateAskLoad();
  void UpdateReady();
  void UpdateAskInstall();
  void UpdateInstalling();

public:
  explicit InstallerSimple(QObject* parent = nullptr);
  virtual ~InstallerSimple();
};
