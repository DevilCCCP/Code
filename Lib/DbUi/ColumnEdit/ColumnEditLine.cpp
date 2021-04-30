#include <QLineEdit>

#include "ColumnEditLine.h"


QWidget* ColumnEditLine::CreateControl(QWidget* parent)
{
  mCtrl = new QLineEdit(parent);
  mCtrl->setReadOnly(mReadOnly);
  return mCtrl;
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


ColumnEditLine::ColumnEditLine(bool _ReadOnly)
  : mCtrl(nullptr), mReadOnly(_ReadOnly)
{
}
