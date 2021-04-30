#pragma once

#include <QIcon>
#include <QMap>

#include <Lib/Db/Db.h>

#include "Tree/TreeModelA.h"


DefineClassS(ObjectModel);
DefineClassS(TreeSchema);
class QTreeView;

class ObjectModel: public TreeModelA
{
  const Db&              mDb;
  ObjectTableS           mObjectTable;
  ObjectTypeTableS       mObjectTypeTable;

  TreeSchemaS            mTreeSchema;

  QIcon                  mDefaultIcon;
  QIcon                  mDefaultIconGray;
  typedef QPair<QIcon, QIcon> PairIcon;
  QMap<int, PairIcon>    mIcons;

  int                    mCurrentId;
  int                    mParentId;
  int                    mNextId;
  int                    mCurrentFit;
  QModelIndex*           mCurrentIndex;
  QString                mFilter;

  Q_OBJECT

public:
  const ObjectTableS& GetObjectTable() { return mObjectTable; }
  const ObjectTypeTableS&  GetObjectTypeTable() { return mObjectTypeTable; }

public:
  int MinId();
  void UpdateSchema();
  void SetFilter(const QString& _Filter, QTreeView* tree);
  bool ReloadTree(int currentId, int parentId, int nextId, QModelIndex& currentIndex, QTreeView* tree);
  void LoadTree(QTreeView* tree);
  bool GetItem(const QModelIndex& index, ObjectItemS& item) const;
  bool GetItemId(const QModelIndex& index, int& id);

  const QIcon& GetIcon(int objType, int status);

private:
  const QIcon& GetIcon(const QString& abbr, int status = 0);
  void AddRootObject(const ObjectItemS& rootObj, TreeItemA* rootItem);
  void AddChildObjects(int rootId, TreeItemA* rootItem);
  void AutoSetCurrent(TreeItemA* item, int objId, int objParentId = 0);

public:
  ObjectModel(const Db& _Db, const TreeSchemaS& _TreeSchema, QObject* parent = 0);
};

