#pragma once

#include <Lib/Include/Names.h>
#include <LibV/Include/ModuleNames.h>

DefineModuleName(Unite, unite);

#define DAEMONS_NAMES_LIST QStringList() << kServerDaemon << kUpdateDaemon
#define DAEMONS_EXE_LIST QStringList() << kServerDaemonExe << kUpdateDaemonExe

inline const QString GetUpdateViewName()    { return QString::fromUtf8("Video update"); }
inline const QString GetUpdateDescription() { return QString::fromUtf8("Video update service"); }

inline const QString GetServerViewName()    { return QString::fromUtf8("Video server"); }
inline const QString GetServerDescription() { return QString::fromUtf8("Video server service"); }

inline const QString GetAdminName()      { return QString::fromUtf8("Admin tool"); }
