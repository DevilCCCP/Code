#pragma once

#include <qsystemdetection.h>

#ifdef Q_OS_WIN32
#include <Lib/Dispatcher/Win/WinTools.h>
#else
#include <Lib/Dispatcher/Linux/LinuxTools.h>
#endif
