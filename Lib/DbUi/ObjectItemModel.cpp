#include <QBrush>
#include <QColor>

#include "ObjectItemModel.h"


const QStringList kHeaders = QStringList() << "Название" << "Описание" << "Версия";

int ObjectItemModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);

  return mItems.size();
}

int ObjectItemModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);

  return kHeaders.size();
}

QVariant ObjectItemModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid()) {
    if (index.row() >= 0 && index.row() < mItems.size()) {
      int section = index.column();
      const ObjectItemS& item = mItems.at(index.row());
      if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (section) {
        case 0: return item->Name;
        case 1: return item->Descr;
        case 2: return item->Version;
        }
      }
    }
  }
  return QVariant();
}

QVariant ObjectItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    return kHeaders.value(section);
  }
  return QVariant();
}

void ObjectItemModel::SetList(const QList<ObjectItemS>& _Items)
{
  beginResetModel();
  mItems = _Items;
  endResetModel();
}

void ObjectItemModel::UpdateList(const QList<ObjectItemS>& _Items)
{
  if (mItems.size() != _Items.size()) {
    return SetList(_Items);
  }
  int size = mItems.size();

  for (int i = 0; i < size; i++) {
    ObjectItemS&       it1 = mItems[i];
    const ObjectItemS& it2 = _Items.at(i);
    if (!it1->Equals(*it2)) {
      it1 = it2;
      dataChanged(index(i, 0), index(i, mHeaders.size() - 1));
    }
  }
}

bool ObjectItemModel::GetItem(const QModelIndex& index, ObjectItemS& item) const
{
  if (index.isValid()) {
    if (index.row() >= 0 && index.row() < mItems.size()) {
      item = mItems.at(index.row());
      return true;
    }
  }
  return false;
}

ObjectItemModel::ObjectItemModel(QObject* parent)
  : QAbstractTableModel(parent)
{
}
