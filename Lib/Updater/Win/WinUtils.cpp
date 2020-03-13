#include <QVector>
#include <Windows.h>
#include <RestartManager.h>

#include <Lib/Log/Log.h>
#include <Lib/Dispatcher/Win/WinTools.h>

#include "WinUtils.h"


bool KillFileOwner(const QString& path)
{
  DWORD sessionHandle;
  WCHAR sessionKey[CCH_RM_SESSION_KEY + 1] = { 0 };
  if (RmStartSession(&sessionHandle, 0, sessionKey) != ERROR_SUCCESS) {
    return false;
  }

  LPCWSTR paths[1] = { (LPCWSTR)path.utf16() };
  if (RmRegisterResources(sessionHandle, 1, (LPCWSTR*)paths, 0, nullptr, 0, nullptr) != ERROR_SUCCESS) {
    return false;
  }

  UINT procInfoCount = 1;
  for (int i = 0; i < 3; i++) {
    UINT procInfoNeeded;
    DWORD reason;
    QVector<RM_PROCESS_INFO> procInfos(procInfoCount);
    DWORD err = RmGetList(sessionHandle, &procInfoNeeded, &procInfoCount, procInfos.data(), &reason);
    if (err == ERROR_SUCCESS) {
      for (int j = 0; j < (int)procInfoCount; j++) {
        QString appName = QString::fromUtf16((const ushort*)procInfos.at(j).strAppName);
        DWORD pid = procInfos.at(j).Process.dwProcessId;
        if (KillProcessByPid(pid)) {
          Log.Info(QString("Kill process locking file (file: '%1', pid: %2, app: '%3'").arg(path).arg(pid).arg(appName));
          return true;
        } else {
          Log.Warning(QString("Kill process locking file fail (file: '%1', pid: %2, app: '%3'").arg(path).arg(pid).arg(appName));
        }
      }
    } else if (err == ERROR_MORE_DATA) {
      procInfoCount = procInfoNeeded;
    }
  }
  return false;
}
