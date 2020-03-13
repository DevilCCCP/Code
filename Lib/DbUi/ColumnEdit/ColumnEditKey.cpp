#include <QComboBox>

#include "ColumnEditKey.h"


QWidget* ColumnEditKey::CreateControl(QWidget* parent)
{
  mCtrl = new QComboBox(parent);
  mCtrl->setModel(mModel);
  mCtrl->setEditable(true);
  return mCtrl;
}

bool ColumnEditKey::LoadValue(const QVariant& value)
{
  qint64 id = value.toLongLong();
  mCtrl->setCurrentIndex(-1);
  if (id) {
    for (int i = 0; i < mModel->rowCount(); i++) {
      if (id == mModel->index(i, 0).data(Qt::UserRole + 1).toLongLong()) {
        mCtrl->setCurrentIndex(i);
        break;
      }
    }
  }
  return true;
}

bool ColumnEditKey::SaveValue(QVariant& value)
{
  int index = mCtrl->currentIndex();
  qint64 id = mModel->index(index, 0).data(Qt::UserRole + 1).toLongLong();
  value = QVariant(id);
  return true;
}


ColumnEditKey::ColumnEditKey(QAbstractTableModel* _Model)
  : mModel(_Model)
{
}
