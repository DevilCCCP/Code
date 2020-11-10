#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>

#include <Lib/CoreUi/Version.h>

#include "Core.h"


Core* Core::mSelf = nullptr;

bool Core::InitVersion()
{
  Version ver;
  if (!ver.LoadFromThis()) {
    mVersion = QString("<недоступно>");
    return false;
  }

  mVersion = ver.ToString();
  return true;
}


Core::Core()
  : mProgramName("Мир")
{
  if (!mSelf) {
    mSelf = this;
  }

  InitVersion();
}
