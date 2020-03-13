#include <QBrush>
#include <QColor>

#include "MulInfoModel.h"


const QStringList kHeaders = QStringList() << "Операция" << "Попыток" << "Успешно" << "Провалено" << "Серия" << "Пройден";

int MulInfoModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);

  return mData.size();
}

int MulInfoModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);

  return kHeaders.size();
}

QVariant MulInfoModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid()) {
    if (index.row() >= 0 && index.row() < mData.size()) {
      int section = index.column();
      const MulInfo& mulInfo = mData.at(index.row());
      if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (section) {
        case 0: return QString("%1x%2").arg(1 + index.row() / 9).arg(1 + index.row() % 9);
        case 1: return mulInfo.Try;
        case 2: return mulInfo.Ok;
        case 3: return mulInfo.Fail;
        case 4: return mulInfo.Last;
        case 5: return mulInfo.Pass? "Да": "Нет";
        }
      } else if (role == Qt::ForegroundRole) {
        switch (section) {
        case 0:
          if (mulInfo.Pass) {
            return QBrush(Qt::darkGreen);
          } else if (mulInfo.Fail > 0) {
            return QBrush(Qt::darkRed);
          }
          break;
        case 2:
        case 4:
          if (mulInfo.Ok > 0) {
            return QBrush(Qt::darkGreen);
          }
          break;
        case 3:
          if (mulInfo.Fail > 0) {
            return QBrush(Qt::darkRed);
          }
          break;
        }
      }
    }
  }
  return QVariant();
}

QVariant MulInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    return kHeaders.value(section);
  }
  return QVariant();
}

MulInfoModel::MulInfoModel(const QVector<MulInfo>& _Data, QObject* parent)
  : QAbstractTableModel(parent)
  , mData(_Data)
{
}
