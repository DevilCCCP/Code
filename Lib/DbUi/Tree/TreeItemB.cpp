#include <QBrush>
#include <QIcon>

#include "TreeItemB.h"
#include "DbTreeSchema.h"


QString TreeItemB::Text(int column) const
{
  return mItem->Text(column);
}

bool TreeItemB::SetText(int column, const QString& text) const
{
  return mItem->SetText(column, text);
}

QBrush TreeItemB::ForeBrush(int column) const
{
  Q_UNUSED(column);

  return QBrush();
}

QBrush TreeItemB::BackBrush(int column) const
{
  Q_UNUSED(column);

  return QBrush();
}

QIcon TreeItemB::Icon() const
{
  return mDbTreeSchema->Icon;
}

void TreeItemB::SetChildren(const QVector<TreeItemBS>& children)
{
  mChildItems = children;
  foreach (const TreeItemBS& item, mChildItems) {
    item->mParentItem = this;
  }
}

void TreeItemB::AppendChild(const TreeItemBS& child)
{
  child->mParentItem = this;
  mChildItems.append(child);
}

void TreeItemB::AppendChildren(const QVector<TreeItemBS>& children)
{
  foreach (const TreeItemBS& child, children) {
    child->mParentItem = this;
    mChildItems.append(child);
  }
}

void TreeItemB::RemoveChilren()
{
  mChildItems.clear();
}

void TreeItemB::RemoveChild(TreeItemB* child)
{
  for (auto itr = mChildItems.begin(); itr != mChildItems.end(); itr++) {
    if (itr->data() == child) {
      mChildItems.erase(itr);
      return;
    }
  }
}

void TreeItemB::RemoveChild(int row)
{
  mChildItems.removeAt(row);
}

TreeItemB* TreeItemB::Child(int row)
{
  return mChildItems.value(row).data();
}

bool TreeItemB::HasChildren() const
{
  return !mChildItems.isEmpty();
}

int TreeItemB::ChildCount() const
{
  return mChildItems.size();
}

int TreeItemB::Row() const
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

TreeItemB* TreeItemB::ParentItem()
{
  return mParentItem;
}


TreeItemB::TreeItemB(const DbItemBS& _Item, DbTreeSchema* _DbTreeSchema, TreeItemB* _ParentItem)
  : mItem(_Item), mMultiLinkId(0), mDbTreeSchema(_DbTreeSchema), mParentItem(_ParentItem)
  , mEnabled(true), mCheckable(false), mCheckState(Qt::Unchecked)
{
}

TreeItemB::~TreeItemB()
{
}
