#include <QPainter>

#include <Lib/Common/Icon.h>

#include "TableModel.h"


int TableModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);

  return mItems.size();
}

int TableModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);

  return mTable->Headers().size();
}

QVariant TableModel::data(const QModelIndex& index, int role) const
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
      case Qt::DecorationRole: if (section == 0) { return GetIcon(); } break;
      case Qt::UserRole + 1  : return mItems.at(row)->Id;
      }
    }
  }
  return QVariant();
}

bool TableModel::setData(const QModelIndex& index, const QVariant& value, int role)
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

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal
      && section >= 0 && section < mTable->Headers().size()) {
    return mTable->Headers().at(section);
  }
  return QVariant();
}

QString TableModel::Text(int row, int column) const
{
  return mItems.at(row)->Text(column);
}

bool TableModel::SetText(int row, int column, const QString& text)
{
  Q_UNUSED(row);
  Q_UNUSED(column);
  Q_UNUSED(text);

  return false;
}

QBrush TableModel::ForeBrush(int row, int column) const
{
  Q_UNUSED(row);
  Q_UNUSED(column);

  return QBrush();
}

QBrush TableModel::BackBrush(int row, int column) const
{
  Q_UNUSED(row);
  Q_UNUSED(column);

  return QBrush();
}

void TableModel::SetList(const QList<DbItemBS>& _Items)
{
  beginResetModel();
  mItems = _Items;
  endResetModel();
}

void TableModel::UpdateList(const QList<DbItemBS>& _Items)
{
  if (mItems.size() != _Items.size()) {
    return SetList(_Items);
  }
  int size = mItems.size();

  for (int i = 0; i < size; i++) {
    DbItemBS&       it1 = mItems[i];
    const DbItemBS& it2 = _Items.at(i);
    if (!it1->Equals(*it2)) {
      it1 = it2;
      dataChanged(index(i, 0), index(i, mTable->Headers().size() - 1));
    }
  }
}

void TableModel::AddItem(const DbItemBS& item)
{
  beginResetModel();
  mItems.append(item);
  endResetModel();
}

void TableModel::UpdateItem(const DbItemBS& item)
{
  int size = mItems.size();

  for (int i = 0; i < size; i++) {
    if (mItems.at(i)->Id == item->Id) {
      dataChanged(index(i, 0), index(i, mTable->Headers().size() - 1));
      break;
    }
  }
}

int TableModel::GetCount() const
{
  return mItems.size();
}

bool TableModel::GetItem(const QModelIndex& index, DbItemBS& item) const
{
  if (index.isValid()) {
    if (index.row() >= 0 && index.row() < mItems.size()) {
      item = mItems.at(index.row());
      return true;
    }
  }
  return false;
}

bool TableModel::GetItem(int index, DbItemBS& item) const
{
  if (index >= 0 && index < mItems.size()) {
    item = mItems.at(index);
    return true;
  }
  return false;
}

const QIcon& TableModel::GetIcon() const
{
  if (mTableIcon.isNull()) {
    mTableIcon = QIcon(mTable->Icon());
  }

  return mTableIcon;
}

TableModel::TableModel(const DbTableBS& _Table, QObject* parent)
  : QAbstractTableModel(parent)
  , mTable(_Table)
{
}
