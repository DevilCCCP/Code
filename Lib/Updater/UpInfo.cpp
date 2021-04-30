#include <qsystemdetection.h>
#include <QCoreApplication>
#include <QFile>

#include <Lib/Dispatcher/Tools.h>
#include <Lib/Log/Log.h>
#include <Local/ModuleNames.h>

#include "UpInfo.h"


const int kValidateDelayMs = 30 * 1000;
const int kCheckDelayMs = 10 * 1000;

bool UpInfo::Create()
{
  mFileMemory.reset(new QFile(MainMemFileName()));
  if (!mFileMemory->open(QFile::ReadWrite)) {
    Log.Fatal(QString("Can't create update info shmem"));
    return false;
  }
  if (mFileMemory->setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup | QFileDevice::WriteGroup)) {
    Log.Info(QString("Set permissions on update info shmem OK"));
  } else {
    Log.Warning(QString("Set permissions on update info shmem fail"));
  }

  if (mFileMemory->size() != sizeof(Info)) {
    if (!mFileMemory->resize(sizeof(Info))) {
      Log.Fatal(QString("Can't resize update info shmem (sz: %1, info: %2)").arg(mFileMemory->size()).arg(sizeof(Info)));
      return false;
    } else {
      Log.Info(QString("Created update info shmem (sz: %1, info: %2)").arg(mFileMemory->size()).arg(sizeof(Info)));
    }
  } else {
    Log.Info(QString("Attached update info shmem (sz: %1, info: %2)").arg(mFileMemory->size()).arg(sizeof(Info)));
  }
  mInfo = new(mFileMemory->map(0, sizeof(Info))) Info;
  if (!mInfo) {
    Log.Warning(QString("Memory map update info shmem fail"));
  }
  return true;
}

void UpInfo::Validate()
{
  if (!mInfo) {
    LOG_ERROR_ONCE("Update info not opened for validate");
    return;
  }

  if (IsFileTime(kValidateDelayMs)) {
    Info readInfo;
    memcpy(&readInfo, mInfo, sizeof(Info));
    for (int i = 0; i < kUpSlotCount; i++) {
      UpSlot* upSlot = &readInfo.UpSlots[i];
      if (!upSlot->LockTimestamp) {
        continue;
      }
      bool alive;
      if (IsProcessAliveByPid(upSlot->Pid, alive) && !alive) {
        memset(upSlot, 0, sizeof(UpSlot));
      }
    }
  }
}

bool UpInfo::StartUpdateNow()
{
  if (!mInfo) {
    LOG_ERROR_ONCE("Update info not opened for starting update");
    return false;
  }

  qint64 latestTimestamp = CalcLatestTimestamp();
  if (latestTimestamp < QDateTime::currentMSecsSinceEpoch()) {
    return true;
  }
  mInfo->UpTimestamp = latestTimestamp;
  return false;
}

int UpInfo::GetReadyToUpdate()
{
  if (!mInfo) {
    LOG_ERROR_ONCE("Update info not opened for check ready to update");
    return 0;
  }

  return (int)(CalcLatestTimestamp() - QDateTime::currentMSecsSinceEpoch());
}

bool UpInfo::IsInstalling()
{
  return mInfo->UpInstalling != 0;
}

bool UpInfo::Attach()
{
  if (mInfo) {
    return true;
  }

  if (!mFileMemory) {
    mFileMemory.reset(new QFile(MainMemFileName()));

    if (!mFileMemory->open(QFile::ReadWrite)) {
      if (mAttachFail < 1) {
        Log.Warning(QString("Attach update info shmem fail (code: %1, text: '%2')").arg(mFileMemory->error()).arg(mFileMemory->errorString()));
        mAttachFail = 1;
      }
      mFileMemory.clear();
      return false;
    }
  }

  if (mFileMemory->size() < (int)sizeof(Info)) {
    if (mAttachFail < 2) {
      Log.Warning(QString("Attached update info shmem wrong size (need: %1, got: %2)").arg(sizeof(Info)).arg(mFileMemory->size()));
      mAttachFail = 2;
    }
    return false;
  }
  mInfo = (Info*)mFileMemory->map(0, sizeof(Info));
  if (!mInfo) {
    if (mAttachFail < 3) {
      Log.Warning(QString("Memory map update info shmem fail"));
      mAttachFail = 3;
    }
    return false;
  }
  if (mInfo->Magic != kUpMagic) {
    if (mAttachFail < 4) {
      Log.Warning(QString("Attached update info shmem wrong magic"));
      mAttachFail = 4;
    }
    mInfo = nullptr;
    return false;
  }
  Log.Info(QString("Attached update info shmem"));
  mAttachFail = 0;
  return true;
}

bool UpInfo::CheckAndLock(int lockPediodMs)
{
  if (!Attach()) {
    return false;
  }
  if (!IsFileTime(kCheckDelayMs)) {
    return false;
  }

  if (GetSlot()) {
    mMySlot->LockTimestamp = mLastLockTime = QDateTime::currentMSecsSinceEpoch() + lockPediodMs;
    mMySlot->Pid = mMyPid;
  }
  return mInfo->UpTimestamp > 0;
}

bool UpInfo::Unlock()
{
  if (!Attach()) {
    return false;
  }

  return ReleaseSlot();
}

bool UpInfo::Check()
{
  if (!Attach()) {
    return false;
  }

  return mInfo->UpTimestamp > 0 && mInfo->UpTimestamp < QDateTime::currentMSecsSinceEpoch();
}

int UpInfo::GetUpStart()
{
  if (!Attach()) {
    return false;
  }

  return mInfo->UpTimestamp - QDateTime::currentMSecsSinceEpoch();
}

bool UpInfo::SetUserUpStart(int lockPediodMs)
{
  if (!Attach()) {
    return false;
  }

  qint64 lockTimestamp = QDateTime::currentMSecsSinceEpoch() + lockPediodMs;
  mInfo->UserUpTimestamp = lockTimestamp;
  if (GetSlot()) {
    mMySlot->LockTimestamp = lockTimestamp;
    return true;
  }
  return false;
}

bool UpInfo::HasUserUpStart()
{
  if (!Attach()) {
    return false;
  }

  return mInfo->UserUpTimestamp > 0;
}

int UpInfo::GetUserUpStart()
{
  if (!Attach()) {
    return -1;
  }

  return mInfo->UserUpTimestamp - QDateTime::currentMSecsSinceEpoch();
}

QString UpInfo::MainMemFileName()
{
  return QCoreApplication::applicationDirPath() + "/Updates/.lock";
}

bool UpInfo::IsFileTime(int periodMs)
{
  qint64 now = QDateTime::currentMSecsSinceEpoch();
  if (now >= mNextReadTime) {
    mNextReadTime = now + periodMs;
    return true;
  }
  return false;
}

qint64 UpInfo::CalcLatestTimestamp()
{
  qint64 latestTimestamp = 0;
  for (int i = 0; i < kUpSlotCount; i++) {
    const UpSlot* upSlot = &mInfo->UpSlots[i];
    latestTimestamp = qMax(latestTimestamp, upSlot->LockTimestamp);
  }
  return latestTimestamp;
}

bool UpInfo::GetSlot()
{
  if (!mMyPid) {
    mMyPid = QCoreApplication::applicationPid();
  }
  if (mMySlot && mMySlot->Pid == mMyPid) {
    return true;
  }

  for (int i = 0; i < kUpSlotCount; i++) {
    UpSlot* upSlot = &mInfo->UpSlots[i];
    if (upSlot->Pid == 0) {
      upSlot->Pid = mMyPid;
      mMySlot = upSlot;
      Log.Info(QString("Update slot is set to %1").arg(i));
      return true;
    }
  }
  mMySlot = nullptr;
  return false;
}

bool UpInfo::ReleaseSlot()
{
  if (!mMyPid) {
    mMyPid = QCoreApplication::applicationPid();
  }
  if (!mMySlot || mMySlot->Pid != mMyPid) {
    return false;
  }
  mMySlot->Pid = 0;
  mMySlot->LockTimestamp = 0;
  mMySlot = nullptr;
  Log.Info(QString("Update slot is released"));
  return true;
}


UpInfo::UpInfo()
  : mInfo(nullptr), mMySlot(nullptr), mMyPid(0), mNextReadTime(0), mLastLockTime(0), mAttachFail(0)
{
}

UpInfo::~UpInfo()
{
  if (mMySlot) {
    ReleaseSlot();
  }
}
