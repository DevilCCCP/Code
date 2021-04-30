#include <QThread>

#include <Lib/Common/FormatTr.h>

#include "InstallerSimple.h"
#include "BackgroundLoader.h"


void InstallerSimple::ReloadSettings(QSettings* settings)
{
  mUpdateSettings.LoadSettings(settings);

  CheckAutoStart();
}

bool InstallerSimple::StartCheck()
{
  if (IsBackgroundRunning()) {
    return false;
  }

  InitPackageLoader();

  ChangeState(eChecking);

  emit BackgroundCheck();

  return true;
}

bool InstallerSimple::StartLoad()
{
  if (IsBackgroundRunning()) {
    return false;
  }

  InitPackageLoader();

  ChangeState(eLoading);

  emit BackgroundLoad();

  return true;
}

bool InstallerSimple::StartLoadAndInstall()
{
  if (IsBackgroundRunning()) {
    return false;
  }

  if (mState == eLoadTrue) {
    StartInstall();
  } else {
    mInstallAfterLoad = true;

    StartLoad();
  }

  return true;
}

bool InstallerSimple::StartInstall()
{
  if (mState != eLoadTrue) {
    return false;
  }

  ChangeState(eInstalling);

  emit BackgroundInstall();

  return true;
}

void InstallerSimple::CheckAutoStart()
{
  if (mState == eNotChecked && mUpdateSettings.getUpdateCheck() == eAutoOnStart) {
    qint64 timeoutMs = mUpdateSettings.getStartWaitMs() - mActionTimer.elapsed();
    if (timeoutMs <= 0) {
      StartCheck();
    } else {
      mAutoTimer->start(timeoutMs);
    }
  } else {
    mAutoTimer->stop();
  }
}

void InstallerSimple::InitPackageLoader()
{
  if (!mWorkerThread) {
    mWorkerThread = new QThread(this);
    mWorkerThread->start();
  }

  if (!mBackgroundLoader) {
    mBackgroundLoader = new BackgroundLoader();
    mBackgroundLoader->Init(getUpdateUri(), getUpdateLogin(), getUpdatePass());
    mBackgroundLoader->moveToThread(mWorkerThread);
    connect(mWorkerThread, &QThread::finished, mBackgroundLoader, &QObject::deleteLater);
    connect(this, &InstallerSimple::BackgroundCheck, mBackgroundLoader, &BackgroundLoader::Check, Qt::QueuedConnection);
    connect(this, &InstallerSimple::BackgroundLoad, mBackgroundLoader, &BackgroundLoader::Load, Qt::QueuedConnection);
    connect(this, &InstallerSimple::BackgroundInstall, mBackgroundLoader, &BackgroundLoader::Install, Qt::QueuedConnection);
    connect(this, &InstallerSimple::BackgroundAbort, mBackgroundLoader, &BackgroundLoader::Abort, Qt::QueuedConnection);
    connect(mBackgroundLoader, &BackgroundLoader::CheckFinished, this, &InstallerSimple::OnCheckFinished, Qt::QueuedConnection);
    connect(mBackgroundLoader, &BackgroundLoader::LoadFinished, this, &InstallerSimple::OnLoadFinished, Qt::QueuedConnection);
    connect(mBackgroundLoader, &BackgroundLoader::InstallStarted, this, &InstallerSimple::UpdateInstalling, Qt::QueuedConnection);
  }
}

void InstallerSimple::ChangeState(InstallState newState)
{
  mState = newState;

  emit StateChanged((int)mState);
}

void InstallerSimple::OnStart()
{
  mActionTimer.start();

  CheckAutoStart();
}

void InstallerSimple::OnExit()
{
  if (CanInstall() && mUpdateSettings.getUpdateInstall() == eInstallOnExit) {
    StartInstall();
  }
}

void InstallerSimple::OnAutoTimerTimeout()
{
  CheckAutoStart();
}

void InstallerSimple::OnCheckFinished(int version)
{
  if (version < 0) {
    ChangeState(eCheckError);
    return;
  } else if (version == 0) {
    ChangeState(eCheckedFalse);
    return;
  }

  ChangeState(eCheckedTrue);
  emit UpdateAvailable();

  if (mUpdateSettings.getUpdateInstall() == eAskInstall) {
    emit UpdateAskLoad();
  } else if (mUpdateSettings.getUpdateInstall() == eInstallOnExit) {
    StartLoad();
  }
}

void InstallerSimple::OnLoadFinished(QString path)
{
  if (path.isEmpty()) {
    ChangeState(eLoadError);
    return;
  }

  ChangeState(eLoadTrue);

  if (mInstallAfterLoad) {
    StartInstall();
    return;
  }

  emit UpdateReady();

  if (mUpdateSettings.getUpdateInstall() == eAskInstall) {
    emit UpdateAskInstall();
  }
}


InstallerSimple::InstallerSimple(QObject* parent)
  : QObject(parent)
  , mState(eNotChecked), mAutoTimer(new QTimer(this))
  , mWorkerThread(nullptr), mBackgroundLoader(nullptr), mInstallAfterLoad(false)
{
  Q_INIT_RESOURCE(Updater);

  mAutoTimer->setSingleShot(true);
  mActionTimer.start();

  connect(mAutoTimer, &QTimer::timeout, this, &InstallerSimple::OnAutoTimerTimeout);
}

InstallerSimple::~InstallerSimple()
{
  if (mWorkerThread) {
    mWorkerThread->quit();
    mWorkerThread->wait();
    delete mWorkerThread;
  }
}
