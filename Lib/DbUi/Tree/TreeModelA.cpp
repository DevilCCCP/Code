#include <QBrush>
#include <QIcon>

#include "TreeModelA.h"
#include "TreeItemA.h"
#include "TreeItemStandard.h"


QVariant TreeModelA::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  if (TreeItemA* item = static_cast<TreeItemA*>(index.internalPointer())) {
    int section = index.column();
    switch (role) {
    case Qt::DisplayRole   :
    case Qt::EditRole      : return item->Text(section);
    case Qt::ForegroundRole: return item->ForeBrush(section);
    case Qt::BackgroundRole: return item->BackBrush(section);
    case Qt::DecorationRole:
      if (section == 0) {
        return item->Icon();
      }
      break;
    }
  }

  return QVariant();
}

QVariant TreeModelA::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal
      && section >= 0 && section < mHeaders.size()) {
    return mHeaders.at(section);
  }

  return QVariant();
}

QModelIndex TreeModelA::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  TreeItemA* parentItem = ParentItem(parent);
  TreeItemA* childItem = parentItem->Child(row);

  if (childItem) {
    return createIndex(row, column, childItem);
  }

  return QModelIndex();
}

QModelIndex TreeModelA::parent(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return QModelIndex();
  }

  TreeItemA* childItem = ItemFromIndex(index);
  TreeItemA* parentItem = childItem->ParentItem();

  if (parentItem == mRootItem) {
    return QModelIndex();
  }

  return createIndex(parentItem->Row(), 0, parentItem);
}

int TreeModelA::rowCount(const QModelIndex& parent) const
{
  if (parent.column() > 0) {
    return 0;
  }

  return ParentItem(parent)->ChildCount();
}

int TreeModelA::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);

  return mHeaders.size();
}

void TreeModelA::Clear()
{
  mRootItem->RemoveChilren();
  beginResetModel();
}

void TreeModelA::AppendChild(const TreeItemAS& child)
{
  mRootItem->AppendChild(child);
}

void TreeModelA::Done()
{
  endResetModel();
}

TreeItemA* TreeModelA::InvisibleRootItem() const
{
  return mRootItem.data();
}

QModelIndex TreeModelA::IndexFromItem(TreeItemA* item) const
{
  if (!item) {
    return QModelIndex();
  }

  return createIndex(item->Row(), 0, item);
}

TreeItemA* TreeModelA::ItemFromIndex(const QModelIndex& index) const
{
  return static_cast<TreeItemA*>(index.internalPointer());
}

void TreeModelA::RowChanged(const QModelIndex& index)
{
  if (index.isValid()) {
    dataChanged(createIndex(index.row(), 0, index.internalPointer()), createIndex(index.row(), mHeaders.size() - 1, index.internalPointer()));
  }
}

TreeItemA* TreeModelA::ParentItem(const QModelIndex& parent) const
{
  return (parent.isValid())? static_cast<TreeItemA*>(parent.internalPointer()): mRootItem.data();
}


TreeModelA::TreeModelA(QObject* parent)
  : QAbstractItemModel(parent)
  , mHeaders(QStringList() << "0"), mRootItem(new TreeItemStandard())
{
}

TreeModelA::~TreeModelA()
{
}
