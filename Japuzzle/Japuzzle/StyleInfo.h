#pragma once

#include <QString>
#include <QVector>
#include <QPoint>


struct StyleInfo {
  QString         Name;
  QString         ViewName;
  uint            BackColor;
  uint            LineColor;
  uint            HighlightColor;
  uint            DigitColor1;
  uint            DigitColor2;
  uint            DigitColor3;
  uint            PreviewColorY;
  uint            PreviewColorN;
  uint            PreviewColorY1;
  uint            PreviewColorN1;
  uint            PreviewLine;
  QString         FontName;
  QVector<QPoint> CursorYesSpot;
  QVector<QPoint> CursorNoSpot;
  QPoint          CursorESpot;
};
