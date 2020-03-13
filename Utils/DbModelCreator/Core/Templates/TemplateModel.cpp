#include "ClassTModel.h"


QString ClassTModel::Text(int row, int column) const
{
  const ClassTS& item = Items().at(row);
  switch (column) {
SWITCH_CASES
  }
  return QString();
}

ClassTModel::ClassTModel(QObject* parent)
  : DbTableModel(QStringList()COLUMN_LIST, ":/Icons/ClassT.png", parent)
{
}

