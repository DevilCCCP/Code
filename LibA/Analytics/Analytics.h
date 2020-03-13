#pragma once

#include <Lib/Log/Log.h>
#include <LibV/Va/AnalyticsA.h>

#include "MacroMotion.h"
#include "CarMotion.h"


AnalyticsAS CreateAnalytics(const QString& vaType)
{
  if (vaType == "vac") {
    return AnalyticsAS(new MacroMotion());
  } else if (vaType == "vaa") {
    return AnalyticsAS(new CarMotion());
  }

  Log.Warning(QString("Undefined va type '%1'").arg(vaType));
  return AnalyticsAS(new MacroMotion());
}
