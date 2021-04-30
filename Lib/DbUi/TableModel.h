#pragma once

#include <QAbstractTableModel>
#include <QList>

#include <Lib/Include/Common.h>
#include <Lib/Db/DbTable.h>


class TableModel: public QAbstractTableModel
{
  PROPERTY_GET(DbTableBS,       Table)
  PROPERTY_GET(QList<DbItemBS>, Items)

  mutable QIcon                 mTableIcon;

protected:
  QList<DbItemBS>& Items() { return mItems; }
  const QList<DbItemBS>& Items() const { return mItems; }

public:
  /*override */virtual int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
  /*override */virtual int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
  /*override */virtual QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
  /*override */virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
  /*override */virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;

protected:
  /*new */virtual QString Text(int row, int column) const;
  /*new */virtual bool SetText(int row, int column, const QString& text);
  /*new */virtual QBrush ForeBrush(int row, int column) const;
  /*new */virtual QBrush BackBrush(int row, int column) const;

public:
  void SetList(const QList<DbItemBS>& _Items);
  void UpdateList(const QList<DbItemBS>& _Items);
  void AddItem(const DbItemBS& item);
  void UpdateItem(const DbItemBS& item);

  int GetCount() const;
  bool GetItem(const QModelIndex& index, DbItemBS& item) const;
  bool GetItem(int index, DbItemBS& item) const;

  const QIcon& GetIcon() const;

public:
  TableModel(const DbTableBS& _Table, QObject* parent = 0);
};
