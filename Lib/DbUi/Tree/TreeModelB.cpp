#include <QBrush>
#include <QIcon>

#include <Lib/Common/Icon.h>

#include "TreeModelB.h"
#include "TreeItemB.h"
#include "TreeItemStandard.h"


Qt::ItemFlags TreeModelB::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flag;
  flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  if (index.isValid()) {
    TreeItemB* item = ItemFromIndex(index);
    if (!item->IsEnabled()) {
      flag = Qt::NoItemFlags;
    }
    if (item->IsCheckable()) {
      flag |= Qt::ItemIsUserCheckable;
    }
  }
  return flag;
}

QVariant TreeModelB::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  if (TreeItemB* item = static_cast<TreeItemB*>(index.internalPointer())) {
    int section = index.column();
    switch (role) {
    case Qt::DisplayRole   :
    case Qt::EditRole      : return item->Text(section);
    case Qt::ForegroundRole: return item->ForeBrush(section);
    case Qt::BackgroundRole: return item->BackBrush(section);

    case Qt::DecorationRole:
      if (section == 0) {
        return item->Item()->Id? item->Schema()->Icon: IconFromImage(item->Schema()->Table->Icon(), QString(":/Icons/Warn.png"));
      }
      break;

    case Qt::CheckStateRole:
      if (section == 0 && item->IsCheckable()){
        return item->CheckState();
      }
      break;
    }
  }

  return QVariant();
}

bool TreeModelB::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (index.isValid()) {
    switch (role) {
    case Qt::CheckStateRole:
      if (IsCheckable(index)) {
        TreeItemB* item = ItemFromIndex(index);
        item->SetCheckState((Qt::CheckState)value.toInt());
        QVector<int> roles;
        roles.append(Qt::CheckStateRole);
        emit dataChanged(index, index, roles);
        return true;
      }
      break;
    }
  }
  return false;
}

QVariant TreeModelB::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal
      && section >= 0 && section < mHeaders.size()) {
    return mHeaders.at(section);
  }

  return QVariant();
}

QModelIndex TreeModelB::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  TreeItemB* parentItem = ParentItem(parent);
  TreeItemB* childItem = parentItem->Child(row);

  if (childItem) {
    return createIndex(row, column, childItem);
  }

  return QModelIndex();
}

QModelIndex TreeModelB::parent(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return QModelIndex();
  }

  TreeItemB* childItem = ItemFromIndex(index);
  TreeItemB* parentItem = childItem->ParentItem();

  if (parentItem == mRootItem) {
    return QModelIndex();
  }

  return createIndex(parentItem->Row(), 0, parentItem);
}

int TreeModelB::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid() && parent.column() > 0) {
    return 0;
  }

  return ParentItem(parent)->ChildCount();
}

int TreeModelB::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);

  return mHeaders.size();
}

void TreeModelB::Clear()
{
  beginResetModel();
  mRootItem->RemoveChilren();
  endResetModel();
}

void TreeModelB::SetEnabled(TreeItemB* item, bool enabled)
{
  item->SetEnabled(enabled);
  UpdateItem(item);
}

void TreeModelB::SetChildren(const QVector<TreeItemBS>& children)
{
  beginResetModel();
  mRootItem->SetChildren(children);
  endResetModel();
}

void TreeModelB::UpdateItem(TreeItemB* item)
{
  QModelIndex index = IndexFromItem(item);
  emit dataChanged(index, index, QVector<int>());
}

void TreeModelB::UpdateItem(TreeItemB* item, QList<int> roles)
{
  QModelIndex index = IndexFromItem(item);
  emit dataChanged(index, index, roles.toVector());
}

void TreeModelB::AppendChild(const TreeItemBS& child)
{
  int row = mRootItem->ChildCount();
  beginInsertRows(QModelIndex(), row, row);
  mRootItem->AppendChild(child);
  endInsertRows();
}

void TreeModelB::AppendChild(TreeItemB* parent, const TreeItemBS& child)
{
  int row = parent->ChildCount();
  beginInsertRows(IndexFromItem(parent), row, row);
  parent->AppendChild(child);
  endInsertRows();
}

void TreeModelB::RemoveChild(TreeItemB* parent, const TreeItemBS& child)
{
  RemoveChild(parent, child.data());
}

void TreeModelB::RemoveChild(TreeItemB* parent, const TreeItemB* child)
{
  for (int i = 0; i < parent->ChildCount(); i++) {
    if (parent->Child(i) == child) {
      RemoveChild(parent, i);
      return;
    }
  }
}

void TreeModelB::RemoveChild(TreeItemB* parent, int row)
{
  beginRemoveRows(IndexFromItem(parent), row, row);
  parent->RemoveChild(row);
  endRemoveRows();
}

void TreeModelB::RemoveChildren(TreeItemB* parent)
{
  if (int count = parent->ChildCount()) {
    beginRemoveRows(IndexFromItem(parent), 0, count - 1);
    parent->RemoveChilren();
    endRemoveRows();
  }
}

void TreeModelB::RemoveItem(TreeItemB* item)
{
  int row = item->Row();
  TreeItemB* parentItem = item->ParentItem();
  beginRemoveRows(IndexFromItem(parentItem), row, row);
  parentItem->RemoveChild(row);
  endRemoveRows();
}

void TreeModelB::BeginUpdate()
{
  beginResetModel();
}

void TreeModelB::Done()
{
  endResetModel();
}

TreeItemB* TreeModelB::InvisibleRootItem() const
{
  return mRootItem.data();
}

QModelIndex TreeModelB::IndexFromItem(TreeItemB* item) const
{
  if (!item || item == mRootItem) {
    return QModelIndex();
  }

  return createIndex(item->Row(), 0, item);
}

TreeItemB* TreeModelB::ItemFromIndex(const QModelIndex& index) const
{
  return static_cast<TreeItemB*>(index.internalPointer());
}

void TreeModelB::RowChanged(const QModelIndex& index)
{
  if (index.isValid()) {
    dataChanged(createIndex(index.row(), 0, index.internalPointer()), createIndex(index.row(), mHeaders.size() - 1, index.internalPointer()));
  }
}

TreeItemB* TreeModelB::ParentItem(const QModelIndex& parent) const
{
  return (parent.isValid())? static_cast<TreeItemB*>(parent.internalPointer()): mRootItem.data();
}

bool TreeModelB::IsCheckable(const QModelIndex& index) const
{
  TreeItemB* item = ItemFromIndex(index);
  return item->IsCheckable();
}


TreeModelB::TreeModelB(QObject* parent)
  : QAbstractItemModel(parent)
  , mHeaders(QStringList() << "0"), mRootItem(new TreeItemB(DbItemBS(), nullptr))
{
}

TreeModelB::~TreeModelB()
{
}
