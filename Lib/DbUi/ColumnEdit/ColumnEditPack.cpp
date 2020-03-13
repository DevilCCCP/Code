#include <QTreeView>
#include <QStandardItemModel>
#include <QDir>

#include <Lib/Common/FilesPackage.h>

#include "ColumnEditPack.h"
#include "FormEditPack.h"


QWidget* ColumnEditPack::CreateControl(QWidget* parent)
{
  mCtrl = new FormEditPack(parent);
  return mCtrl;
}

bool ColumnEditPack::LoadValue(const QVariant& value)
{
  QByteArray data = value.toByteArray();
  mFilesPackage.reset(new FilesPackage());
  mFilesPackage->Load(data);
  mCtrl->SetPackage(mFilesPackage);
  return true;
}

bool ColumnEditPack::SaveValue(QVariant& value)
{
  QByteArray data;
  mFilesPackage->Save(data);
  value = QVariant(data);
  return true;
}


ColumnEditPack::ColumnEditPack()
{
}
