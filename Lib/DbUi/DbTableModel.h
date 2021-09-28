#pragma once

#include <QAbstractTableModel>
#include <QList>
#include <QPainter>

#include <Lib/Db/TableB.h>
#include <Lib/Common/Icon.h>


template <typename DbItemT>
class DbTableModel: public QAbstractTableModel
{
  typedef QSharedPointer<DbItemT> DbItemTS;

  PROPERTY_GET_SET(QStringList, Headers)
  PROPERTY_GET_SET(QIcon,       Icon)
  PROPERTY_GET(QList<DbItemTS>, Items)

protected:
  QList<DbItemTS>& Items() { return mItems; }
  const QList<DbItemTS>& Items() const { return mItems; }
  const QIcon& Icon() const { return mIcon; }

public:
  /*override */virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
  {
    Q_UNUSED(parent);

    return mItems.size();
  }

  /*override */virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override
  {
    Q_UNUSED(parent);

    return mHeaders.size();
  }

  /*override */virtual QVariant data(const QModelIndex& index, int role) const override
  {
    if (index.isValid()) {
      if (index.row() >= 0 && index.row() < Items().size()) {
        int section = index.column();
        int row     = index.row();
        switch (role) {
        case Qt::DisplayRole   :
        case Qt::EditRole      : return Text(row, section);
        case Qt::ForegroundRole: return ForeBrush(row, section);
        case Qt::BackgroundRole: return BackBrush(row, section);
        case Qt::DecorationRole: if (section == 0) { return Icon(); } break;
        case Qt::UserRole + 1  : return mItems.at(row)->Id;
        }
      }
    }
    return QVariant();
  }

  /*override */virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override
  {
    if (index.isValid()) {
      if (index.row() >= 0 && index.row() < Items().size()) {
        int section = index.column();
        int row     = index.row();
        switch (role) {
        case Qt::EditRole      : return SetText(row, section, value.toString());
        }
      }
    }
    return false;
  }

  /*override */virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override
  {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal
        && section >= 0 && section < mHeaders.size()) {
      return mHeaders.at(section);
    }
    return QVariant();
  }

protected:
  /*new */virtual QString Text(int row, int column) const
  {
    return mItems.at(row)->Text(column);
  }

  /*new */virtual bool SetText(int row, int column, const QString& text)
  {
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(text);

    return false;
  }

  /*new */virtual QBrush ForeBrush(int row, int column) const
  {
    Q_UNUSED(row);
    Q_UNUSED(column);

    return QBrush();
  }

  /*new */virtual QBrush BackBrush(int row, int column) const
  {
    Q_UNUSED(row);
    Q_UNUSED(column);

    return QBrush();
  }

public:
  void SetList(const QList<DbItemTS>& _Items)
  {
    beginResetModel();
    mItems = _Items;
    endResetModel();
  }

  void UpdateList(const QList<DbItemTS>& _Items)
  {
    if (mItems.size() != _Items.size()) {
      return SetList(_Items);
    }
    int size = mItems.size();

    for (int i = 0; i < size; i++) {
      DbItemTS&       it1 = mItems[i];
      const DbItemTS& it2 = _Items.at(i);
      if (!it1->Equals(*it2)) {
        it1 = it2;
        dataChanged(index(i, 0), index(i, mHeaders.size() - 1));
      }
    }
  }

  int GetCount() const { return mItems.size(); }

  bool GetItem(const QModelIndex& index, DbItemTS& item) const
  {
    if (index.isValid()) {
      if (index.row() >= 0 && index.row() < mItems.size()) {
        item = mItems.at(index.row());
        return true;
      }
    }
    return false;
  }

  bool GetItem(int index, DbItemTS& item) const
  {
    if (index >= 0 && index < mItems.size()) {
      item = mItems.at(index);
      return true;
    }
    return false;
  }

public:
  DbTableModel(const QStringList& _Headers, const QString& _IconName, QObject *parent = 0)
    : QAbstractTableModel(parent)
    , mHeaders(_Headers), mIcon(SmallIcon(_IconName))
  { }

  DbTableModel(const QString& _IconName, QObject *parent = 0)
    : QAbstractTableModel(parent)
    , mIcon(SmallIcon(_IconName))
  { }

  DbTableModel(QObject *parent = 0)
    : QAbstractTableModel(parent)
  {
  }
};
