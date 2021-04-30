#include <QSettings>

#include "UpdateSettings.h"


const UpdateCheck kDefaultUpdateCheck = eAutoOnStart;
const UpdateInstall kDefaultUpdateInstall = eAskInstall;
const int kDefaultStartWaitMs = 10 * 1000;

void UpdateSettings::DefaultSettings()
{
  mUpdateCheck = kDefaultUpdateCheck;
  mUpdateInstall = kDefaultUpdateInstall;
  mStartWaitMs = kDefaultStartWaitMs;
}

void UpdateSettings::LoadSettings(QSettings* settings)
{
  mUpdateCheck = (UpdateCheck)settings->value("UpdateCheck", kDefaultUpdateCheck).toInt();
  mUpdateInstall = (UpdateInstall)settings->value("UpdateInstall", kDefaultUpdateInstall).toInt();
}

void UpdateSettings::SaveSettings(QSettings* settings)
{
  settings->setValue("UpdateCheck", (int)mUpdateCheck);
  settings->setValue("UpdateInstall", (int)mUpdateInstall);
}


UpdateSettings::UpdateSettings()
{
  DefaultSettings();
}
