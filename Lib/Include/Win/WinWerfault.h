#pragma once

#include <QCoreApplication>
#include <QFileInfo>

#include <Windows.h>
//#include <Werapi.h>

#include <Lib/Log/Log.h>

inline void TurnOffWerFault()
{
  SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_NOALIGNMENTFAULTEXCEPT | SEM_NOOPENFILEERRORBOX);
  //QString filepath = QFileInfo(qApp->applicationFilePath()).absoluteFilePath();
  //if (int err = WerAddExcludedApplication(filepath.utf16(), TRUE)) {
  //  if (err == E_ACCESSDENIED) {
  //    Log.Info("WerAddExcludedApplication denied");
  //  } else {
  //    Log.Warning(QString("WerAddExcludedApplication fail (code: 0x%1)").arg(err, 0, 16));
  //  }
  //}
}
