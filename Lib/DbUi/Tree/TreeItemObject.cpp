#include "TreeItemObject.h"
#include "ObjectModel.h"


QString TreeItemObject::Text(int section) const
{
  switch (section) {
  case 0: return mItem->Version.isEmpty()? mItem->Name: QString("%1 (%2)").arg(mItem->Name).arg(mItem->Version);
  }
  return QString();
}

QIcon TreeItemObject::Icon() const
{
  return mCore->GetIcon(mItem->Type, mItem->Status);
}


TreeItemObject::TreeItemObject(ObjectModel* _Core, const ObjectItemS& _Item, TreeItemA* parent)
  : TreeItemA(parent)
  , mCore(_Core), mItem(_Item)
{
}

