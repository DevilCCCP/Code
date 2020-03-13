#pragma once

#include <Lib/Include/Common.h>
#include <LibV/Va/Va.h>

#include "BlockSceneAnalizer.h"
#include "BlockScene.h"
#include "CarDef.h"


DefineClassS(SettingsA);

class CarTracker: public BlockSceneAnalizer
{
public:
  void LoadSettings(const SettingsAS& settings);
  void SaveSettings(const SettingsAS& settings);
  void Init();

  void Analize(const QList<CarInfo>& carList);

  bool HaveObj();
  bool RetrieveObj(Object& object);

private:

public:
  CarTracker(const AnalyticsB& _Analytics);
};

