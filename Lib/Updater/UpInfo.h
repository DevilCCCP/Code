#pragma once

#include <QDateTime>

#include <Lib/Include/Common.h>


DefineClassS(UpInfo);
DefineClassS(QFile);

const int kUpSlotCount = 8;
const int kUpMagic = 0x11223344;

class UpInfo
{
  struct UpSlot {
    qint64 Pid;
    qint64 LockTimestamp;
  };
  struct Info {
    int    Magic;
    qint64 UpTimestamp;
    qint64 UserUpTimestamp;
    qint64 UpInstalling;
    UpSlot UpSlots[kUpSlotCount];

    Info() { memset(this, 0, sizeof(Info)); Magic = kUpMagic; }
  };

  QFileS            mFileMemory;
  Info*             mInfo;
  UpSlot*           mMySlot;
  qint64            mMyPid;
  qint64            mNextReadTime;
  qint64            mLastLockTime;
  int               mAttachFail;

public:
  bool Create();
  void Validate();
  bool StartUpdateNow();
  int  GetReadyToUpdate();
  bool IsInstalling();

  bool Attach();
  bool CheckAndLock(int lockPediodMs);
  bool Unlock();
  bool Check();
  int  GetUpStart();
  bool SetUserUpStart(int lockPediodMs);
  bool HasUserUpStart();
  int  GetUserUpStart();

private:
  static QString MainMemFileName();

  bool IsFileTime(int periodMs);

  qint64 CalcLatestTimestamp();
  bool GetSlot();
  bool ReleaseSlot();

public:
  UpInfo();
  ~UpInfo();
};

