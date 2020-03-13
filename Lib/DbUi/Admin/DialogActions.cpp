#include <QStandardItemModel>
#include <QStandardItem>

#include <Lib/Db/ObjectType.h>

#include "DialogActions.h"
#include "ui_DialogActions.h"
#include "ActionIcons.h"


DialogActions::DialogActions(QWidget* parent)
  : QDialog(parent), ui(new Ui::DialogActions)
  , mItemModel(new QStandardItemModel(this))
{
  ui->setupUi(this);

  ui->treeViewMain->setModel(mItemModel);
  mItemModel->setHorizontalHeaderLabels(QStringList() << "Название объекта");
}

DialogActions::~DialogActions()
{
  delete ui;
}


void DialogActions::Init(const ActionIcons* actionIcons, const ToolActionList& actions)
{
  for (int i = 0; i < actions.size(); i++) {
    const ToolAction& action = actions.at(i);
    QStandardItem* item = new QStandardItem(actionIcons->GetIcon(action), action.getSlaveItem()->Name);
    item->setData(i);
    item->setCheckable(true);
    item->setCheckState(Qt::Checked);
    mItemModel->appendRow(item);
  }
}

QList<int> DialogActions::SelectedActions()
{
  QList<int> selected;
  for (int i = 0; i < mItemModel->rowCount(); i++) {
    if (mItemModel->item(i, 0)->checkState() == Qt::Checked) {
      selected.append(mItemModel->item(i, 0)->data().toInt());
    }
  }
  return selected;
}

void DialogActions::on_labelAll_linkActivated(const QString&)
{
  for (int i = 0; i < mItemModel->rowCount(); i++) {
    mItemModel->item(i, 0)->setCheckState(Qt::Checked);
  }
}

void DialogActions::on_labelNone_linkActivated(const QString&)
{
  for (int i = 0; i < mItemModel->rowCount(); i++) {
    mItemModel->item(i, 0)->setCheckState(Qt::Unchecked);
  }
}
