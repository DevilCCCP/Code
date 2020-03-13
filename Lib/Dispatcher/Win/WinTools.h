#pragma once

#include <QString>
#include <QStringList>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 1
#include <Windows.h>


bool SetDebugPrivilege(bool enable);
bool StartProcessByPid(const QString path, const QStringList params, DWORD& pid);
BOOL KillProcessByPid(DWORD pid, UINT exitCode = -13);
bool IsProcessAliveByPid(DWORD pid, bool& result, int* retCode = nullptr);
