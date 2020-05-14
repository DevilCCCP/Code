#pragma once

#include <QVector>

#include <Lib/Db/DbTable.h>


DefineClassS(TreeItemB);
DefineClassS(DbTreeSchema);

class TreeItemB
{
  DbItemBS            mItem;
  qint64              mMultiLinkId;
  DbTreeSchema*       mDbTreeSchema;
  QVector<TreeItemBS> mChildItems;
  TreeItemB*          mParentItem;

  bool                mEnabled;
  bool                mCheckable;
  Qt::CheckState      mCheckState;

public:
  void SetMultiLinkId(const qint64& _MultiLinkId) { mMultiLinkId = _MultiLinkId; }
  qint64 MultiLinkId() const { return mMultiLinkId; }
  const DbItemBS& Item() const { return mItem; }
  void SetItem(const DbItemBS& item) { mItem = item; }
  DbTreeSchema* Schema() const { return mDbTreeSchema; }
  void SetEnabled(bool _Enabled) { mEnabled = _Enabled; }
  bool IsEnabled() { return mEnabled; }
  void SetCheckable(bool _Checkable) { mCheckable = _Checkable; }
  bool IsCheckable() const { return mCheckable; }
  void SetCheckState(Qt::CheckState _CheckState) { mCheckState = _CheckState; }
  Qt::CheckState CheckState() const { return mCheckState; }
  bool IsChecked() const { return mCheckState != Qt::Unchecked; }

public:
  /*new */virtual QString Text(int column) const;
  /*new */virtual bool SetText(int column, const QString& text) const;
  /*new */virtual QBrush ForeBrush(int column) const;
  /*new */virtual QBrush BackBrush(int column) const;
  /*new */virtual QIcon Icon() const;

public:
  void SetChildren(const QVector<TreeItemBS>& children);
  void AppendChild(const TreeItemBS& child);
  void AppendChildren(const QVector<TreeItemBS>& children);
  void RemoveChilren();
  void RemoveChild(TreeItemB* child);
  void RemoveChild(int row);
  TreeItemB* Child(int row);
  bool HasChildren() const;
  int ChildCount() const;
  int Row() const;
  TreeItemB* ParentItem();

public:
  explicit TreeItemB(const DbItemBS& _Item, DbTreeSchema* _DbTreeSchema, TreeItemB* _ParentItem = 0);
  /*new */virtual ~TreeItemB();
};
