#pragma once

#include <QAbstractItemModel>
#include <QStringList>
#include <QModelIndex>

#include <Lib/Include/Common.h>


DefineClassS(TreeItemA);

class TreeModelA: public QAbstractItemModel
{
  PROPERTY_GET_SET(QStringList, Headers)

  TreeItemAS mRootItem;

public:
  void SetHeaders(const QStringList& _Headers) { mHeaders = _Headers; }

public:
  /*override */virtual QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
  /*override */virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
  /*override */virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
  /*override */virtual QModelIndex parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
  /*override */virtual int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
  /*override */virtual int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

public:
  void Clear();
  void AppendChild(const TreeItemAS& child);
  void Done();

  TreeItemA* InvisibleRootItem() const;
  QModelIndex IndexFromItem(TreeItemA* item) const;
  TreeItemA* ItemFromIndex(const QModelIndex& index) const;

  void RowChanged(const QModelIndex& index);

private:
  TreeItemA* ParentItem(const QModelIndex& parent) const;

public:
  explicit TreeModelA(QObject* parent = 0);
  /*override */virtual ~TreeModelA();
};
