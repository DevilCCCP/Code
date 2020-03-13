#include <QPlainTextEdit>

#include "ColumnEditText.h"


QWidget* ColumnEditText::CreateControl(QWidget* parent)
{
  mCtrl = new QPlainTextEdit(parent);
  if (mHeightLimit > 0) {
    mCtrl->setMaximumHeight(mHeightLimit);
  }
  if (mVerticalStretch > 0) {
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setVerticalStretch(mVerticalStretch);
    mCtrl->setSizePolicy(sizePolicy);
  }
  return mCtrl;
}

bool ColumnEditText::LoadValue(const QVariant& value)
{
  mCtrl->setPlainText(value.toString());
  return true;
}

bool ColumnEditText::SaveValue(QVariant& value)
{
  value = QVariant(mCtrl->toPlainText());
  return true;
}


ColumnEditText::ColumnEditText(int _HeightLimit, int _VerticalStretch)
  : mHeightLimit(_HeightLimit), mVerticalStretch(_VerticalStretch)
{
}
