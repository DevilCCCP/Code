#pragma once

#include <Lib/Include/Names.h>
#include <LibV/Include/ModuleNames.h>

DefineModuleName(VideoA, va);

#define DAEMONS_NAMES_LIST QStringList() << kServerDaemon << kArmDaemon
#define DAEMONS_EXE_LIST QStringList() << kServerDaemonExe << kArmDaemonExe

inline const QString GetArmViewName()    { return QString::fromUtf8("АРМ СНА"); }
inline const QString GetArmDescription() { return QString::fromUtf8("Сервис СНА, осуществляющий контроль работы компонент АРМ"); }
inline const QString GetAdminName()      { return QString::fromUtf8("АРМ администратора СНА"); }
