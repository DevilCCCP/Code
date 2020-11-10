#pragma once

#include <Lib/Include/Common.h>


DefineClassS(MapParameters);

class MapParameters
{
  PROPERTY_GET    (int,  GlobeRadius)
  PROPERTY_GET    (int,  GlobeSector)
  PROPERTY_GET    (int,  WorldSector)
  PROPERTY_GET    (int,  GroundPercent)
  PROPERTY_GET    (int,  GlobePlateCount)
  PROPERTY_GET    (int,  WorldPlateCount)
  ;
public:
  void setGlobeRadius(int value);
  void setGlobeSector(int value);
  void setWorldSector(int value);
  void setGroundPercent(int value);
  void setGlobePlateCount(int value);
  void setWorldPlateCount(int value);

public:
  static int MinimumGlobeRadius() { return 30; }
  static int MaximumGlobeRadius() { return 300; }
  static int DefaultGlobeRadius() { return 100; }

  static int MinimumGlobeSector() { return 30; }
  static int MaximumGlobeSector() { return 360; }
  static int DefaultGlobeSector() { return 120; }

  static int MinimumWorldSector() { return 30; }
  static int MaximumWorldSector() { return 360; }
  static int DefaultWorldSector() { return 120; }

  static int MinimumGroundPercent() { return 10; }
  static int MaximumGroundPercent() { return 80; }
  static int DefaultGroundPercent() { return 30; }

  static int MinimumGlobePlateCount() { return 1; }
  static int MaximumGlobePlateCount() { return 9; }
  static int DefaultGlobePlateCount() { return 3; }

  static int MinimumWorldPlateCount() { return 1; }
  static int MaximumWorldPlateCount() { return 9; }
  static int DefaultWorldPlateCount() { return 3; }

public:
  MapParameters();
};
