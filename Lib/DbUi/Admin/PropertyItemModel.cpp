#include <Lib/Common/Format.h>
#include <Lib/Db/ObjectSettingsType.h>

#include "PropertyItemModel.h"
#include "Def.h"


QVariant PropertyItemModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::DisplayRole && index.column() > 0) {
    const ObjectSettingsType* typeItem = QStandardItemModel::data(index, Qt::UserRole + 1).value<const ObjectSettingsType*>();
    bool ok;
    if (typeItem->Type == "bool") {
      int yes = QStandardItemModel::data(index, Qt::UserRole + 2).toInt(&ok);
      if (ok) {
        if (yes) {
          return QVariant(typeItem->MaxValue);
        } else {
          return QVariant(typeItem->MinValue);
        }
      } else {
        return QVariant();
      }
    } else if (typeItem->Type == "size") {
      qint64 ms = QStandardItemModel::data(index, Qt::UserRole + 2).toLongLong(&ok);
      if (ok) {
#ifdef LANG_EN
        return FormatBytes(ms);
#else
        return FormatBytesRu(ms);
#endif
      }
    } else if (typeItem->Type == "time_period") {
      qint64 ms = QStandardItemModel::data(index, Qt::UserRole + 2).toLongLong(&ok);
      if (ok) {
#ifdef LANG_EN
        return FormatTime(ms);
#else
        return FormatTimeRu(ms);
#endif
      }
    }
    return QStandardItemModel::data(index, Qt::UserRole + 2);
  }
  return QStandardItemModel::data(index, role);
}

PropertyItemModel::PropertyItemModel(QObject* parent)
  : QStandardItemModel(parent)
{
}
