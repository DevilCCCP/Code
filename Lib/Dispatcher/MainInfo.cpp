#include "MainInfo.h"


void ProcessInfo::Init(int id, qint64 timeMs, EProcStatus status)
{
  Id = id;
  Pid = 0;
  CurrentStatus = eWaitStart;
  DemandStatus = status;
  LastAliveMs = timeMs;
  LiveCounter = 0;
  LiveCounterRead = 0;
}

void ProcessInfo::Clear()
{
  Id = -1;
  Pid = 0;
  CurrentStatus = eProcNone;
  DemandStatus = eProcNone;
  LiveCounter = 0;
  LiveCounterRead = 0;
}

bool ProcessInfo::IsLive(qint64 &timeMs)
{
  if (LiveCounterRead != LiveCounter) {
    LiveCounterRead = LiveCounter;
    LastAliveMs = timeMs;
    return true;
  } else {
    return false;
  }
}

ProcessInfo::ProcessInfo()
  : MagicC(kMagicC), Id(-1), DemandStatus(eProcNone), CurrentStatus(eProcNone), LiveCounter(0), LiveCounterRead(0)
{
}


int MainInfo::Size(int processCount)
{
  Q_ASSERT(processCount > 0);
  return sizeof(MainInfo) + sizeof(ProcessInfo) * (processCount - 1);
}

int MainInfo::Size()
{
  return Size(ProcessCount);
}

QString MainInfo::GetShmemName(const QString& daemonName, int pageIndex)
{
  return QString("%1_%2").arg(daemonName).arg(pageIndex);
}

bool MainInfo::Validate(int size)
{
  return MagicA == kMagicA && Size() <= size && mProcessInfo[ProcessCount].MagicC == kMagicB;
}

MainInfo::MainInfo(int _ProcessCount, qint64 _Pid)
  : MagicA(kMagicA), Pid(_Pid), Next(false), ProcessCount(_ProcessCount)
{
  mProcessInfo[ProcessCount].MagicC = kMagicB;
}
