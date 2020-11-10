#include "MapParameters.h"


void MapParameters::setGlobeRadius(int value)
{
  mGlobeRadius = qBound(MinimumGlobeRadius(), value, MaximumGlobeRadius());
}

void MapParameters::setGlobeSector(int value)
{
  mGlobeSector = qBound(MinimumGlobeSector(), value, MaximumGlobeSector());
}

void MapParameters::setWorldSector(int value)
{
  mWorldSector = qBound(MinimumWorldSector(), value, MaximumWorldSector());
}

void MapParameters::setGroundPercent(int value)
{
  mGroundPercent = qBound(MinimumGroundPercent(), value, MaximumGroundPercent());
}

void MapParameters::setGlobePlateCount(int value)
{
  mGlobePlateCount = qBound(MinimumGlobePlateCount(), value, MaximumGlobePlateCount());
}

void MapParameters::setWorldPlateCount(int value)
{
  mWorldPlateCount = qBound(MinimumWorldPlateCount(), value, MaximumWorldPlateCount());
}


MapParameters::MapParameters()
{
  mGlobeRadius     = DefaultGlobeRadius();
  mGlobeSector     = DefaultGlobeSector();
  mWorldSector     = DefaultWorldSector();
  mGroundPercent   = DefaultGroundPercent();
  mGlobePlateCount = DefaultGlobePlateCount();
  mWorldPlateCount = DefaultWorldPlateCount();
}
