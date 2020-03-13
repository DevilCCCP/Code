#pragma once

#include <QString>

#include <Lib/Include/Common.h>


enum EProcStatus {
  eProcNone = 0,

  eWaitStart = 0x10,
  eStart = 0x11,

  eInitialize = 0x20,
  eLive = 0x21,
  eMining = 0x22,

  eStop = 0x40,
  eRestart = 0x41,
  eClear = 0x42,

  eZombie = 0x80,
  eRip = 0x81,
  eCrushed = 0x82,

  eFlagStarting = 0x10,
  eFlagLive = 0x20,
  eFlagStop = 0x40,
  eFlagEnded = 0x80,

  eIllegal = 0xff
};

inline const char* EProcStatus_ToString(EProcStatus status)
{
  switch (status) {
  case eProcNone: return "None";
  case eWaitStart: return "Wait start";
  case eStart: return "Start";
  case eInitialize: return "Initialize";
  case eLive: return "Live";
  case eMining: return "Mining";
  case eStop: return "Stop";
  case eRestart: return "Restart";
  case eClear: return "Clear";
  case eZombie: return "Zombie";
  case eRip: return "Rip";
  case eCrushed: return "Crushed";
  case eIllegal: return "Illegal";
  default: return "unknown";
  }
}

struct ProcessInfo
{
  static const int kMagicC = 0x666cc666;

  int         MagicC;
  int         Id;
  qint64      Pid;
  EProcStatus DemandStatus;
  EProcStatus CurrentStatus;
  qint64      LastAliveMs;
  int         LiveCounter;
  int         LiveCounterRead;

  // Use from dispatcher
  void Init(int id, qint64 timeMs, EProcStatus status = eWaitStart);
  bool IsInit() { return DemandStatus != eProcNone; }
  void Clear();
  bool IsLive(qint64& timeMs);

  // Use from overseer
  bool SayLive() { LiveCounter++; return !IsStop(); }
  bool IsStop() { return DemandStatus & eFlagStop; }

  ProcessInfo();
};

struct MainInfo
{
  static const int kVersion = 1;
  static const int kMagicA = 0x66613666 + kVersion;
  static const int kMagicB = 0x66693666 + kVersion;

  int         MagicA;
  qint64      Pid;
  qint64      LastAlive;
  bool        Next;
  int         ProcessCount;

private:
  ProcessInfo mProcessInfo[1];
public:
  ProcessInfo& Process(int i) { return mProcessInfo[i]; }

  static int Size(int processCount);
  int Size();

  static QString GetShmemName(const QString &daemonName, int pageIndex);
  bool Validate(int size);

  MainInfo(int _ProcessCount, qint64 _Pid);
};

