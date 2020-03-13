#include <QSpinBox>

#include "ColumnEditInt.h"


QWidget* ColumnEditInt::CreateControl(QWidget* parent)
{
  mCtrl = new QSpinBox(parent);
  mCtrl->setMinimum(mMinValue);
  mCtrl->setMaximum(mMaxValue);
  mCtrl->setValue(mDefaultValue);
  return mCtrl;
}

bool ColumnEditInt::LoadValue(const QVariant& value)
{
  mCtrl->setValue(value.toInt());
  return true;
}

bool ColumnEditInt::SaveValue(QVariant& value)
{
  value = QVariant(mCtrl->value());
  return true;
}


ColumnEditInt::ColumnEditInt(qint64 _MinValue, qint64 _DefaultValue, qint64 _MaxValue)
  : mMinValue(_MinValue), mDefaultValue(_DefaultValue), mMaxValue(_MaxValue)
{
}
