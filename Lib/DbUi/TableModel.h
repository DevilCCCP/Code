#pragma once

#include <QAbstractTableModel>
#include <QIcon>
#include <QList>

#include <Lib/Db/Db.h>
#include <Lib/Common/Icon.h>


template <typename TableItemTS>
class TableTModel: public QAbstractTableModel
{
  QList<TableItemTS> mItems;
  QIcon              mIcon;

protected:
  const QList<TableItemTS>& Items() const { return mItems; }
  QList<TableItemTS>&       Items()       { return mItems; }
  const QIcon&              Icon()  const { return mIcon; }

public:
  void SetList(const QList<TableItemTS>& _Items)
  {
    beginResetModel();
    mItems = _Items;
    endResetModel();
  }

  bool GetItem(const QModelIndex& index, TableItemTS& item)
  {
    if (index.isValid()) {
      if (index.row() >= 0 && index.row() < mItems.size()) {
        item = mItems.at(index.row());
        return true;
      }
    }
    return false;
  }

public:
  TableTModel(const QString& _IconName, QObject *parent = 0)
    : QAbstractTableModel(parent)
    , mIcon(SmallIcon(_IconName))
  { }
};

typedef TableTModel<TableItemBS> TableBModel;
typedef TableTModel<TableItemS>  TableModel;

typedef QSharedPointer<TableBModel> TableBModelS;
typedef QSharedPointer<TableModel>  TableModelS;
