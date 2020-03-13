#pragma once

#include <QVector>

#include <Lib/Include/Common.h>


DefineClassS(TreeItemA);

class TreeItemA
{
  QVector<TreeItemAS> mChildItems;
  TreeItemA*          mParentItem;

public:
  /*new */virtual QString Text(int column) const = 0;
  /*new */virtual QBrush ForeBrush(int column) const;
  /*new */virtual QBrush BackBrush(int column) const;
  /*new */virtual QIcon Icon() const;

public:
  void AppendChild(const TreeItemAS& child);
  void RemoveChilren();
  TreeItemA* Child(int row);
  int ChildCount() const;
  int Row() const;
  TreeItemA* ParentItem();

public:
  explicit TreeItemA(TreeItemA* _ParentItem = 0);
  /*new */virtual ~TreeItemA();
};
