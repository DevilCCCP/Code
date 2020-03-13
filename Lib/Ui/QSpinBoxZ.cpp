#include "QSpinBoxZ.h"


QString QSpinBoxZ::textFromValue(int value) const
{
  return QString("%1").arg(value, mZeroLength, displayIntegerBase(), QChar('0'));
}


QSpinBoxZ::QSpinBoxZ(QWidget* parent)
  : QSpinBox(parent)
  , mZeroLength(8)
{
}
