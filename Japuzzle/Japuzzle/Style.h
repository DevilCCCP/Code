#pragma once

#include <QString>
#include <QVector>
#include <QColor>
#include <QImage>
#include <QCursor>

#include <Lib/Include/Common.h>

#include "StyleInfo.h"


DefineStructS(StyleInfo);
typedef QVector<QImage> ImageVector;
typedef QVector<QCursor> CursorVector;

class Style
{
  PROPERTY_GET(QColor,      BackColor)
  PROPERTY_GET(QColor,      LineColor)
  PROPERTY_GET(QColor,      HighlightColor)
  PROPERTY_GET(QColor,      PreviewLineColor)

  PROPERTY_GET(QColor,      DigitWhiteColor)
  PROPERTY_GET(QColor,      DigitBlackColor)
  PROPERTY_GET(QColor,      DigitRedColor)

  PROPERTY_GET(QColor,      YesColor)
  PROPERTY_GET(QColor,      NoColor)
  PROPERTY_GET(QColor,      Yes1Color)
  PROPERTY_GET(QColor,      No1Color)

  PROPERTY_GET(QImage,      Background)

  PROPERTY_GET(ImageVector, Yes)
  PROPERTY_GET(ImageVector, No)
  PROPERTY_GET(QImage,      Prop)
  PROPERTY_GET(QImage,      Spot)
  PROPERTY_GET(QImage,      Null)

  PROPERTY_GET(QCursor,     CursorArrow)
  PROPERTY_GET(CursorVector,CursorYes)
  PROPERTY_GET(CursorVector,CursorNo)
  PROPERTY_GET(QCursor,     CursorErase)

  PROPERTY_GET(QString,     Font)
  ;
public:
  bool Load(const StyleInfo& info);

private:
  static void MakeColorFromInt(uint colorHex, QColor& color);

public:
  Style();
};
