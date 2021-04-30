#include <QTreeView>
#include <QMessageBox>

#include <Lib/Db/ObjectType.h>
#include <Lib/Common/Icon.h>
#include <Lib/Log/Log.h>

#include "ObjectModel.h"
#include "Tree/TreeSchema.h"
#include "Tree/TreeItemObject.h"
#include "Tree/TreeItemStandard.h"


int ObjectModel::MinId()
{
  return mTreeSchema->GetMinId();
}

void ObjectModel::UpdateSchema()
{
  mTreeSchema->Reload(mObjectTypeTable);
}

void ObjectModel::SetFilter(const QString& _Filter, QTreeView* tree)
{
  mFilter = _Filter;

  LoadTree(tree);
}

bool ObjectModel::ReloadTree(int currentId, int parentId, int nextId, QModelIndex& currentIndex, QTreeView* tree)
{
  mCurrentId = currentId;
  mParentId = parentId;
  mNextId = nextId;
  mCurrentFit = 0;
  mCurrentIndex = &currentIndex;

  mObjectTable->ReloadConnections();
  mObjectTable->Reload();
  if (!mTreeSchema->IsInit()) {
    return false;
  }

  mIcons.clear();
  LoadTree(tree);
  return true;
}

void ObjectModel::LoadTree(QTreeView* tree)
{
  Clear();

  QVector<QList<ObjectItemS> > rootItems(mTreeSchema->GetSize());
  auto objectItems = mObjectTable->GetItems();
  for (auto itr = objectItems.begin(); itr != objectItems.end(); itr++) {
    const TableItemS& item = itr.value();
    const ObjectItemS& obj = item.staticCast<ObjectItem>();

    if (!mFilter.isEmpty()) {
      if (!obj->Name.contains(mFilter) && !obj->Guid.contains(mFilter)) {
        continue;
      }
    }
    int objType = (!mObjectTable->IsDefault(obj->Id))? obj->Type: 0;
    int index = -1;
    while (mTreeSchema->GetIndex(objType, index)) {
      rootItems[index].append(obj);
    }
  }

  QList<TreeItemA*> expendItems;
  const QList<TreeRootItem>& schema = mTreeSchema->GetSchema();
  for (int i = 0; i < schema.size(); i++) {
    const TreeRootItem& item = schema.at(i);
    TreeItemAS rootItem(new TreeItemStandard(GetIcon(item.getType()), item.getName()));
    AppendChild(rootItem);
    if (item.getExpanded()) {
      expendItems.append(rootItem.data());
    }

    const QList<ObjectItemS>& list = rootItems.at(i);
    for (auto itr = list.begin(); itr != list.end(); itr++) {
      ObjectItemS obj = *itr;
      if (!item.getUnlinkedOnly() || !mObjectTable->HasParent(obj->Id)) {
        AddRootObject(obj, rootItem.data());
      }
    }
  }

  Done();

  foreach (TreeItemA* expendItem, expendItems) {
    tree->expand(IndexFromItem(expendItem));
  }
}

bool ObjectModel::GetItem(const QModelIndex& index, ObjectItemS& item) const
{
  if (index.isValid()) {
    TreeItemA* treeItem = static_cast<TreeItemA*>(index.internalPointer());
    TreeItemObject* treeObjectItem = dynamic_cast<TreeItemObject*>(treeItem);
    if (treeObjectItem) {
      item = treeObjectItem->getItem();
      return true;
    }
  }
  return false;
}

bool ObjectModel::GetItemId(const QModelIndex& index, int& id)
{
  if (index.isValid()) {
    TreeItemA* treeItem = static_cast<TreeItemA*>(index.internalPointer());
    TreeItemObject* treeObjectItem = dynamic_cast<TreeItemObject*>(treeItem);
    if (treeObjectItem) {
      id = treeObjectItem->getItem()->Id;
      return true;
    }
  }
  return false;
}

const QIcon& ObjectModel::GetIcon(int objType, int status)
{
  auto itr = mIcons.find(objType);
  if (itr == mIcons.end()) {
    QIcon icon;
    if (objType) {
      const TableItemS& item = mObjectTypeTable->GetItem(objType);
      const ObjectTypeItem* typeItem = static_cast<const ObjectTypeItem*>(item.data());
      if (typeItem) {
        icon = QIcon(QString(":/ObjTree/") + typeItem->Name);
      }
      if (icon.availableSizes().isEmpty()) {
        icon = mDefaultIcon;
      }
    } else {
      icon = QIcon(QString(":/ObjTree/tmp"));
    }

    QIcon icong = GrayIcon(icon);
    itr = mIcons.insert(objType, qMakePair(icon, icong));
  }

  const QIcon& icon = (status >= 0)? itr.value().first: itr.value().second;
  if (!icon.isNull()) {
    return icon;
  }

  return (status >= 0)? mDefaultIcon: mDefaultIconGray;
}

const QIcon& ObjectModel::GetIcon(const QString& abbr, int status)
{
  if (abbr == "tmp") {
    return GetIcon(0, status);
  }

  if (const NamedItem* item = mObjectTypeTable->GetItemByName(abbr)) {
    return GetIcon(item->Id, status);
  }
  return (status >= 0)? mDefaultIcon: mDefaultIconGray;
}

void ObjectModel::AddRootObject(const ObjectItemS& rootObj, TreeItemA* rootItem)
{
  TreeItemAS item(new TreeItemObject(this, rootObj));
  rootItem->AppendChild(item);
  AddChildObjects(rootObj->Id, item.data());

  AutoSetCurrent(item.data(), rootObj->Id, 0);
}

void ObjectModel::AddChildObjects(int rootId, TreeItemA* rootItem)
{
  const QMap<int, int>& slaveConnection = mObjectTable->SlaveConnection();
  QList<int> childs;
  for (auto itr = slaveConnection.find(rootId); itr != slaveConnection.end() && itr.key() == rootId; itr++) {
    int slaveId = itr.value();
    childs.append(slaveId);
  }

  qSort(childs);
  for (auto itr = childs.begin(); itr != childs.end(); itr++) {
    int slaveId = *itr;
    TableItemS item = mObjectTable->GetItem(slaveId);

    if (ObjectItemS obj = item.staticCast<ObjectItem>()) {
      TreeItemAS childItem(new TreeItemObject(this, obj));
      rootItem->AppendChild(childItem);
      AddChildObjects(obj->Id, childItem.data());

      AutoSetCurrent(childItem.data(), obj->Id, rootId);
    }
  }
}

void ObjectModel::AutoSetCurrent(TreeItemA* item, int objId, int objParentId)
{
  if (mCurrentFit < 200) {
    if (objId == mCurrentId) {
      if (objParentId == mParentId) {
        *mCurrentIndex = IndexFromItem(item);
        mCurrentFit = 200;
      } else if (mCurrentFit < 50) {
        *mCurrentIndex = IndexFromItem(item);
        mCurrentFit = 50;
      }
    }
    if (mCurrentFit < 75) {
      if (objId == mNextId) {
        *mCurrentIndex = IndexFromItem(item);
        mCurrentFit = 75;
      }
      if (mCurrentFit < 50) {
        if (objId == mParentId) {
          *mCurrentIndex = IndexFromItem(item);
          mCurrentFit = 50;
        }
      }
    }
  }
}


ObjectModel::ObjectModel(const Db& _Db, const TreeSchemaS& _TreeSchema, QObject* parent)
  : TreeModelA(parent)
  , mDb(_Db), mObjectTable(new ObjectTable(_Db)), mObjectTypeTable(new ObjectTypeTable(_Db))
  , mTreeSchema(_TreeSchema)
  , mCurrentId(0), mParentId(0), mCurrentFit(0)
{
  Q_INIT_RESOURCE(DbUi);

  mDefaultIcon = QIcon(":/ObjTree/default");
  mDefaultIconGray = GrayIcon(mDefaultIcon);
}
