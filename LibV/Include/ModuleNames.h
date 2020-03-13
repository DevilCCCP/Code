#pragma once

#include <qsystemdetection.h>

#include <Lib/Include/Names.h>
#include <Lib/Include/ModuleNames.h>


// ArmV
DefineModuleName(AdminUi, admin);
DefineModuleName(ArmDaemon, armd);
DefineModuleName(Player, plr);
DefineModuleName(Control, ctrl);
DefineModuleName(Monitoring, mon);
DefineModuleName(StorageUi, rep_ui);

// ServerV
DefineModuleName(ServerDaemon, srvd);
DefineModuleName(Video, video);
DefineModuleName(StoreCreator, repc);


#ifdef Q_OS_WIN32
inline QString GetPlayerWndClassName(int id)
{
  return QString("%1_%2").arg(kControl).arg(id, 3, 10, QChar('0'));
}
#endif

inline QString GetMonitorShmemName(int index)
{
  return QString("%1_mon_%2").arg(kPlayer).arg(index, 3, 10, QChar('0'));
}
