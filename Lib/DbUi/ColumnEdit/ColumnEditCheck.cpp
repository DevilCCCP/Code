#include <QCheckBox>

#include "ColumnEditCheck.h"


QWidget* ColumnEditCheck::CreateControl(QWidget* parent)
{
  return mCtrl = new QCheckBox(mCheckText, parent);
}

bool ColumnEditCheck::LoadValue(const QVariant& value)
{
  mCtrl->setChecked(value.toBool());
  return true;
}

bool ColumnEditCheck::SaveValue(QVariant& value)
{
  value = QVariant(mCtrl->isChecked()? "1": "0");
  return true;
}


ColumnEditCheck::ColumnEditCheck(const QString& _CheckText)
  : mCheckText(_CheckText)
{
}
