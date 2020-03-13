#pragma once

#include <QStringList>

#include <Lib/Db/Db.h>


enum ESettingsType {
  eSettingsDb,
  eSettingsUser
};

enum EStyleType {
  eStyleModerm,
};

DefineClassS(PlayerSettings);
DefineClassS(SettingsA);

class PlayerSettings
{
  SettingsAS mSettings;

  PROPERTY_GET_SET(EStyleType,   Style)
  PROPERTY_GET_SET(bool,         ScaleBest)
  PROPERTY_GET_SET(bool,         ShowMouse)
  PROPERTY_GET_SET(bool,         AutoHideMouse)
  PROPERTY_GET_SET(QVector<int>, MonitorMap)
  ;

public:
  static QStringList GetTypes();
  bool LoadDbSettings(const Db& db, int armId);
  bool LoadUserSettings(bool failNotExists = true);
  bool SaveSettings();

private:
  bool LoadValues();

public:
  PlayerSettings();
};
