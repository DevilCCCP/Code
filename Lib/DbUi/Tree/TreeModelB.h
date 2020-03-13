#pragma once

#include <QAbstractItemModel>
#include <QStringList>
#include <QModelIndex>

#include <Lib/Db/DbTable.h>

#include "DbTreeSchema.h"
#include "TreeItemB.h"


class TreeModelB: public QAbstractItemModel
{
  PROPERTY_GET_SET(QStringList, Headers)

  DbTreeSchemaS mSchema;
  TreeItemBS    mRootItem;

public:
  void SetHeaders(const QStringList& _Headers) { mHeaders = _Headers; }

public:
  /*override */virtual Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
  /*override */virtual QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
  /*override */virtual bool setData(const QModelIndex& index, const QVariant& value, int role) Q_DECL_OVERRIDE;
  /*override */virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
  /*override */virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
  /*override */virtual QModelIndex parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
  /*override */virtual int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
  /*override */virtual int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

public:
  void Clear();
  void SetEnabled(TreeItemB* item, bool enabled);
  void SetChildren(const QVector<TreeItemBS>& children);
  void UpdateItem(TreeItemB* item);
  void UpdateItem(TreeItemB* item, QList<int> roles);
  void AppendChild(const TreeItemBS& child);
  void AppendChild(TreeItemB* parent, const TreeItemBS& child);
  void RemoveChild(TreeItemB* parent, const TreeItemBS& child);
  void RemoveChild(TreeItemB* parent, const TreeItemB* child);
  void RemoveChild(TreeItemB* parent, int row);
  void RemoveChildren(TreeItemB* parent);
  void RemoveItem(TreeItemB* item);
  void BeginUpdate();
  void Done();

  TreeItemB* InvisibleRootItem() const;
  QModelIndex IndexFromItem(TreeItemB* item) const;
  TreeItemB* ItemFromIndex(const QModelIndex& index) const;

  void RowChanged(const QModelIndex& index);

private:
  TreeItemB* ParentItem(const QModelIndex& parent) const;
  bool IsCheckable(const QModelIndex& index) const;

public:
  explicit TreeModelB(QObject* parent = 0);
  /*override */virtual ~TreeModelB();
};
