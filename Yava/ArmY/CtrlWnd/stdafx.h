#pragma once

#include <QElapsedTimer>
#include <QList>
#include <QSharedPointer>
#include <QThread>
#include <QMutex>

#include <QApplication>
#include <QDesktopWidget>
#include <qsystemdetection.h>

#ifdef Q_OS_WIN32
#include <Windows.h>
#include <commctrl.h>
#endif
