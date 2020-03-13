#include <Lib/Db/Event.h>

#include "EventModel.h"


static int kColumsCount = 2;

int EventModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);

  return Items().size();
}

int EventModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);

  return kColumsCount;
}

QVariant EventModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid()) {
    if (index.row() >= 0 && index.row() < Items().size()) {
      int section = index.column();
      const TableItemS& eventitem = Items().at(index.row());
      const Event* evnt = static_cast<const Event*>(eventitem.data());
      if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (section) {
        case 0: return evnt->ObjectId;
        case 1: return evnt->EventTypeId;
        }
      } else if (role == Qt::DecorationRole && section == 0) {
        return Icon();
      }
    }
  }
  return QVariant();
}

QVariant EventModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    switch (section) {
    case 0: return mHeaders.size() > 0? mHeaders[0]: "Object";
    case 1: return mHeaders.size() > 1? mHeaders[1]: "Type";
    }
  }
  return QVariant();
}


EventModel::EventModel(const ObjectTableS& _ObjectTable, const EventTypeTableS& _EventTypeTable, const QStringList& _Headers, QObject* parent)
  : TableModel("", parent)
  , mObjectTable(_ObjectTable), mEventTypeTable(_EventTypeTable), mHeaders(_Headers)
{
}

