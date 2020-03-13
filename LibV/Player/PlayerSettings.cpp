#include <QDir>

#include <Lib/Settings/DbSettings.h>
#include <Lib/Settings/FileSettings.h>
#include <Local/ModuleNames.h>

#include "PlayerSettings.h"


QStringList PlayerSettings::GetTypes()
{
  return QStringList() << "База данных" << "Локальный пользователь";
}

bool PlayerSettings::LoadDbSettings(const Db& db, int armId)
{
  mSettings.reset(new DbSettings(db));
  if (!mSettings->Open(QString::number(armId))) {
    return false;
  }
  return LoadValues();
}

bool PlayerSettings::LoadUserSettings(bool failNotExists)
{
  QString iniFilePath = QDir(QStandardPaths::writableLocation(
    QStandardPaths::GenericConfigLocation)).absoluteFilePath(QString("%1.ini").arg(kPlayer));
  if (failNotExists && !QFile::exists(iniFilePath)) {
    return false;
  }
  mSettings.reset(new FileSettings());
  if (!mSettings->Open(iniFilePath)) {
    return false;
  }
  return LoadValues();
}

bool PlayerSettings::SaveSettings()
{
  mSettings->SetValue("Style", mStyle);
  mSettings->SetValue("ScaleBest", mScaleBest);
  mSettings->SetValue("ShowMouse", mShowMouse);
  mSettings->SetValue("AutoHideMouse", mAutoHideMouse);
  for (int i = 0; i < mMonitorMap.size(); i++) {
    mSettings->SetValue(QString("Monitor %1").arg(i + 1), mMonitorMap.at(i));
  }
  return mSettings->Sync();
}

bool PlayerSettings::LoadValues()
{
  mStyle         = (EStyleType)mSettings->GetValue("Style", eStyleModerm).toInt();
  mScaleBest     = mSettings->GetValue("ScaleBest", true).toBool();
  mShowMouse     = mSettings->GetValue("ShowMouse", true).toBool();
  mAutoHideMouse = mSettings->GetValue("AutoHideMouse", true).toBool();
  mMonitorMap.clear();
  for (int i = 0; ; i++) {
    if (int monitorIndex = mSettings->GetValue(QString("Monitor %1").arg(i + 1), 0).toInt()) {
      mMonitorMap.append(monitorIndex);
    } else {
      break;
    }
  }
  return true;
}


PlayerSettings::PlayerSettings()
{
}

