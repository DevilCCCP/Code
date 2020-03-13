#include <QListView>
#include <QStandardItemModel>

#include "ColumnEditKeyList.h"
#include "FormKeyList.h"


QWidget* ColumnEditKeyList::CreateControl(QWidget* parent)
{
  mCtrl = new FormKeyList(mModel, parent);
  return mCtrl;
}

bool ColumnEditKeyList::LoadValue(const QVariant& value)
{
  QStringList valueText = value.toStringList();
  mCtrl->SetList(valueText);
  return true;
}

bool ColumnEditKeyList::SaveValue(QVariant& value)
{
  QStringList valueText;
  mCtrl->GetList(valueText);
  value = QVariant(valueText);
  return true;
}


ColumnEditKeyList::ColumnEditKeyList(QAbstractTableModel* _Model)
  : mModel(_Model)
{
}
