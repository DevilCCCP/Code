#pragma once

#include <QList>

#include <Lib/Include/Common.h>

#include "BlockSceneAnalizer.h"
#include "BlockScene.h"
#include "CarDef.h"


DefineClassS(Analyser);
DefineClassS(SettingsA);

class CarDetection: public BlockSceneAnalizer
{
  Region<uchar>   mRegion;
  AnalyserS       mAnalyser;
  int             mPlateWidth;

  PROPERTY_GET(QList<CarInfo>, CarList)
  ;
public:
  void LoadSettings(const SettingsAS& settings);
  void SaveSettings(const SettingsAS& settings);
  void Init(const ImageSrc<uchar>& source);

  void Analize(const ImageSrc<uchar>& source);

  void GetDbgUinPre(ImageSrc<uchar>& debug);
  void GetDbgUinPreImg(ImageSrc<uchar>& debug);

private:

public:
  CarDetection(const AnalyticsB& _Analytics);
};

