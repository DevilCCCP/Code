#pragma once

#include <QVector>
#include <QPointF>
#include <QColor>


struct EarthPlate {
  QColor           Color;
  QVector<QPointF> Border;
};
typedef QVector<EarthPlate> EarthLevel;
typedef QVector<EarthLevel> EarthLandscape;
