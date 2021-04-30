#pragma once

#include <qsystemdetection.h>

#ifdef Q_OS_WIN32
#include <Lib/UpdaterCore/Win/WinUtils.h>
#else
#include <Lib/UpdaterCore/Linux/LinuxUtils.h>
#endif
