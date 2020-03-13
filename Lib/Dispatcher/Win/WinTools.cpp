#include <QScopedPointer>

#include "WinTools.h"


struct QScopedPointerHandleDeleter
{
  static inline void cleanup(HANDLE* handle)
  {
    CloseHandle(*handle);
  }
};

bool SetDebugPrivilege(bool enable)
{
  HANDLE token;
  if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &token)) {
    if (GetLastError() == ERROR_NO_TOKEN) {
      if (!ImpersonateSelf(SecurityImpersonation)) {
        return false;
      }

      if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &token)) {
        return false;
      }
    } else {
      return false;
    }
  }
  QScopedPointer<HANDLE, QScopedPointerHandleDeleter> h1(&token);

  TOKEN_PRIVILEGES tp;
  LUID luid;
  TOKEN_PRIVILEGES tpPrevious;
  DWORD tpsz = sizeof(TOKEN_PRIVILEGES);

  if (!LookupPrivilegeValueA(nullptr, "SeDebugPrivilege", &luid)) {
    return false;
  }

  tp.PrivilegeCount           = 1;
  tp.Privileges[0].Luid       = luid;
  tp.Privileges[0].Attributes = 0;

  AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &tpPrevious, &tpsz);

  if (GetLastError() != ERROR_SUCCESS) {
    return false;
  }

  tpPrevious.PrivilegeCount       = 1;
  tpPrevious.Privileges[0].Luid   = luid;
  if (enable) {
    tpPrevious.Privileges[0].Attributes |= SE_PRIVILEGE_ENABLED;
  } else {
    tpPrevious.Privileges[0].Attributes &= (~SE_PRIVILEGE_ENABLED);
  }

  AdjustTokenPrivileges(token, FALSE, &tpPrevious, tpsz, nullptr, nullptr);

  if (GetLastError() != ERROR_SUCCESS) {
    return false;
  }

  return true;
}

bool StartProcessByPid(const QString path, const QStringList params, DWORD& pid)
{
  QString cmd = QString("\"%1\"").arg(path);
  foreach (const QString& param, params) {
    cmd.append(QString(" \"%1\"").arg(param));
  }

  STARTUPINFOW stinfo;
  memset(&stinfo, 0, sizeof(stinfo));
  stinfo.cb = sizeof(stinfo);

  PROCESS_INFORMATION pinfo;
  memset(&pinfo, 0, sizeof(pinfo));

  SECURITY_ATTRIBUTES psattr;
  memset(&psattr, 0, sizeof(psattr));
  psattr.nLength = sizeof(psattr);
  psattr.lpSecurityDescriptor = NULL;
  psattr.bInheritHandle       = FALSE;

  SECURITY_ATTRIBUTES tsattr;
  ZeroMemory(&tsattr, sizeof(tsattr));
  tsattr.nLength = sizeof(tsattr);
  tsattr.lpSecurityDescriptor = NULL;
  tsattr.bInheritHandle       = FALSE;

  BOOL result = CreateProcessW(NULL, (LPWSTR)cmd.utf16(), &psattr, &tsattr, FALSE
                               , DETACHED_PROCESS, NULL, NULL, &stinfo, &pinfo);

  if (!result) {
    return false;
  }

  pid = pinfo.dwProcessId;

  CloseHandle(pinfo.hThread);
  CloseHandle(pinfo.hProcess);
  return true;
}

BOOL KillProcessByPid(DWORD pid, UINT exitCode)
{
  //// Kill process tree (error reporting window)
  //Q_UNUSED(exitCode);
  //QString killCmd = QString("taskkill /F /T /PID %1").arg(pid);
  //return system(killCmd.toLatin1().constData()) == 0;
  HANDLE proc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
  if (proc == NULL) {
    return true;
  }

  BOOL result = TerminateProcess(proc, exitCode);

  CloseHandle(proc);
  return result;
}

bool IsProcessAliveByPid(DWORD pid, bool& result, int* retCode)
{
  HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
  if (proc == nullptr) {
    result = false;
    if (GetLastError() == ERROR_INVALID_PARAMETER) {
      return true;
    } else {
//      if (errorCode) {
//        *errorCode = GetLastError();
//      }
      return false;
    }
  }

  bool success;
  DWORD exitCode;
  if (GetExitCodeProcess(proc, &exitCode)) {
    result = exitCode == STILL_ACTIVE;
    if (retCode) {
      *retCode = exitCode;
    }
    success = true;
  } else {
//    if (errorCode) {
//      *errorCode = GetLastError();
//    }
    success = false;
  }
  CloseHandle(proc);
  return success;
}

