#include <QComboBox>
#include <QRegExp>

#include "ColumnEditSwitch.h"


QWidget* ColumnEditSwitch::CreateControl(QWidget* parent)
{
  mCtrl = new QComboBox(parent);
  QRegExp keyValueRegExp("(\\-?\\d+):\\s*(.*)");

  int index = 0;
  for (const QString& line: mSwitchText) {
    if (!keyValueRegExp.exactMatch(line)) {
      Q_ASSERT(0);
      continue;
    }

    int key = keyValueRegExp.cap(1).toInt();
    QString value = keyValueRegExp.cap(2);
    mCtrl->addItem(value, key);
    mValueIndexMap[key] = index;
    index++;
  }

  return mCtrl;
}

bool ColumnEditSwitch::LoadValue(const QVariant& value)
{
  auto itr = mValueIndexMap.find(value.toInt());
  if (itr != mValueIndexMap.end()) {
    mCtrl->setCurrentIndex(itr.value());
  }
  return true;
}

bool ColumnEditSwitch::SaveValue(QVariant& value)
{
  value = QVariant(mCtrl->currentData().toInt());
  return true;
}


ColumnEditSwitch::ColumnEditSwitch(const QStringList& _SwitchText)
  : mSwitchText(_SwitchText)
{
}
