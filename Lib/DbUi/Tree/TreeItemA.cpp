#include <QBrush>
#include <QIcon>

#include "TreeItemA.h"


QBrush TreeItemA::ForeBrush(int column) const
{
  Q_UNUSED(column);

  return QBrush();
}

QBrush TreeItemA::BackBrush(int column) const
{
  Q_UNUSED(column);

  return QBrush();
}

QIcon TreeItemA::Icon() const
{
  return QIcon();
}

void TreeItemA::AppendChild(const TreeItemAS& child)
{
  child->mParentItem = this;
  mChildItems.append(child);
}

void TreeItemA::RemoveChilren()
{
  mChildItems.clear();
}

TreeItemA* TreeItemA::Child(int row)
{
  return mChildItems.value(row).data();
}

int TreeItemA::ChildCount() const
{
  return mChildItems.size();
}

int TreeItemA::Row() const
{
  if (!mParentItem) {
    return 0;
  }
  for (int i = 0; i < mParentItem->mChildItems.size(); i++) {
    if (mParentItem->mChildItems.at(i).data() == this) {
      return i;
    }
  }
  return 0;
}

TreeItemA* TreeItemA::ParentItem()
{
  return mParentItem;
}


TreeItemA::TreeItemA(TreeItemA* _ParentItem)
  : mParentItem(_ParentItem)
{
}

TreeItemA::~TreeItemA()
{
}
