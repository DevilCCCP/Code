#include <QProcess>

#include <Lib/Log/Log.h>

#include "LinuxUtils.h"


bool KillFileOwner(const QString& path)
{
  QString cmd = QString("sudo fuser -k \"%1\"").arg(path);
  int ret = QProcess::execute(cmd);
  if (ret == 0) {
    Log.Info(QString("File user was killed (file: '%1')").arg(path));
  } else {
    Log.Error(QString("File user was NOT killed (file: '%1')").arg(path));
  }
  return ret == 0;
}
