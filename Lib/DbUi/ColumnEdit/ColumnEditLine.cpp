#include <QLineEdit>

#include "ColumnEditLine.h"


QWidget* ColumnEditLine::CreateControl(QWidget* parent)
{
  return mCtrl = new QLineEdit(parent);
}

bool ColumnEditLine::LoadValue(const QVariant& value)
{
  mCtrl->setText(value.toString());
  return true;
}

bool ColumnEditLine::SaveValue(QVariant& value)
{
  value = QVariant(mCtrl->text());
  return true;
}


ColumnEditLine::ColumnEditLine()
{
}
