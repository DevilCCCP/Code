#include <QDoubleSpinBox>

#include "ColumnEditReal.h"


QWidget* ColumnEditReal::CreateControl(QWidget* parent)
{
  mCtrl = new QDoubleSpinBox(parent);
  mCtrl->setMinimum(mMinValue);
  mCtrl->setMaximum(mMaxValue);
  mCtrl->setValue(mDefaultValue);
  return mCtrl;
}

bool ColumnEditReal::LoadValue(const QVariant& value)
{
  mCtrl->setValue(value.toDouble());
  return true;
}

bool ColumnEditReal::SaveValue(QVariant& value)
{
  value = QVariant(mCtrl->value());
  return true;
}


ColumnEditReal::ColumnEditReal(qreal _MinValue, qreal _DefaultValue, qreal _MaxValue)
  : mMinValue(_MinValue), mDefaultValue(_DefaultValue), mMaxValue(_MaxValue)
{
}
