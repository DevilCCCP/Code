#pragma once

#include <Lib/Include/Common.h>


enum UpdateCheck {
  eManualCheck = 0,
  eAutoOnStart
};

enum UpdateInstall {
  eManualInstall = 0,
  eAskInstall,
  eInstallOnExit
};

class QSettings;

class UpdateSettings
{
  PROPERTY_GET(UpdateCheck,   UpdateCheck)
  PROPERTY_GET(UpdateInstall, UpdateInstall)
  PROPERTY_GET(int,           StartWaitMs)
  ;
public:
  void DefaultSettings();
  void LoadSettings(QSettings* settings);
  void SaveSettings(QSettings* settings);

public:
  UpdateSettings();
};
