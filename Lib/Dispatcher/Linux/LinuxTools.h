#pragma once

#include <signal.h>

bool inline KillProcessByPid(pid_t pid)
{
  if (kill(pid, SIGKILL) < 0) {
    return errno == ESRCH;
  }
  return true;
}

bool inline IsProcessAliveByPid(pid_t pid, bool& result, int* errorCode = nullptr)
{
  int ret = kill(pid, 0);
  if (ret == -1) {
    if (errno == ESRCH) {
      result = false;
      return true;
    } else {
      if (errorCode) {
        *errorCode = errno;
      }
      return false;
    }
  } else if (ret == 0) {
    result = true;
    return true;
  }
  return false;
}
