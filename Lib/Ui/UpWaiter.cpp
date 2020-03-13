#include <QDialog>
#include <QBoxLayout>
#include <QTimer>

#include <Lib/Log/Log.h>
#include <Lib/Updater/UpInfo.h>

#include "UpWaiter.h"
#include "FormUpdateSync.h"


const int kNotifyIntervalSec = 60;
const int kLockIntervalMs = 2 * 60 * 1000;

void UpWaiter::CreateDialog()
{
  QDialog* upDialog = new QDialog(mParent);
  FormUpdateSync* formUpdateSync = new FormUpdateSync(upDialog);

  upDialog->setWindowModality(Qt::NonModal);

  QVBoxLayout* mainDialogLayout = new QVBoxLayout(upDialog);
  mainDialogLayout->addWidget(formUpdateSync);
  upDialog->setLayout(mainDialogLayout);

  upDialog->show();
  connect(formUpdateSync, &FormUpdateSync::SelectUserUpStart, this, &UpWaiter::OnUserSetUpStart);
  connect(upDialog, &QDialog::finished, upDialog, &QDialog::deleteLater);
  connect(this, &UpWaiter::UpdateSecs, formUpdateSync, &FormUpdateSync::OnUpdateSecs);
  connect(this, &UpWaiter::CloseDialog, upDialog, &QDialog::close);
  mUpdateSeconds = mUpInfo->GetUpStart() / 1000;
  mUserDialog = true;
}

bool UpWaiter::CheckUpStart()
{
  int upStart = mUpInfo->GetUpStart() / 1000;
  if (upStart <= 0) {
    Log.Info(QString("Update started, closing"));
    mParent->close();
    return true;
  }
  if (mUserDialog) {
    if (upStart != mUpdateSeconds) {
      mUpdateSeconds = upStart;
      emit UpdateSecs(mUpdateSeconds);
    }
  }
  return false;
}

bool UpWaiter::CheckUserStart()
{
  if (!mUpUserSeconds && !mUpInfo->HasUserUpStart()) {
    if (!mUserDialog) {
      Log.Info(QString("User not informed, starting dialog"));
      CreateDialog();
    }
  } else {
    int userUpStart = mUpInfo->GetUserUpStart() / 1000;
    if (userUpStart != mUpUserSeconds) {
      mUpUserSeconds = userUpStart;
      if (mUpUserSeconds <= 0) {
        mParent->close();
        return true;
      }
      bool needDialog = mUpUserSeconds < kNotifyIntervalSec;
      if (needDialog != mUserDialog) {
        if (!mUserDialog) {
          Log.Info(QString("User delay too near, starting dialog"));
          CreateDialog();
        } else {
          emit CloseDialog();
          mUserDialog = false;
        }
      }
    }
  }
  return false;
}

void UpWaiter::OnTimer()
{
  if (!mUpdateSeconds) {
    if (!mUpInfo->CheckAndLock(mLockIntervalMs)) {
      return;
    }
  }

  if (CheckUpStart() || CheckUserStart()) {
    if (mUserDialog) {
      emit CloseDialog();
      mUserDialog = false;
    }
    return;
  }
}

void UpWaiter::OnUserSetUpStart(int timeoutMs)
{
  mUpInfo->SetUserUpStart(timeoutMs);
  mUserDialog = false;
  Log.Info(QString("User delay on %1").arg(timeoutMs));
}


UpWaiter::UpWaiter(UpInfo* _UpInfo, QWidget* parent)
  : QObject(parent)
  , mUpInfo(_UpInfo), mParent(parent), mLockIntervalMs(kLockIntervalMs)
  , mTimer(new QTimer(parent)), mFormUpdateSync(nullptr)
  , mUpdateSeconds(0), mUpUserSeconds(0), mUserDialog(false)
{
  mTimer->setInterval(100);
  mTimer->setSingleShot(false);
  connect(mTimer, &QTimer::timeout, this, &UpWaiter::OnTimer);
  mTimer->start();
}

UpWaiter::~UpWaiter()
{
}

