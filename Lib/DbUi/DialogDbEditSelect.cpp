#include <QPushButton>

#include "DialogDbEditSelect.h"
#include "ui_DialogDbEditSelect.h"
#include "Tree/TreeModelB.h"


DialogDbEditSelect::DialogDbEditSelect(QWidget* parent)
  : QDialog(parent), ui(new Ui::DialogDbEditSelect)
  , mTreeModel(new TreeModelB(this))
{
  ui->setupUi(this);

  ui->treeViewMain->setModel(mTreeModel);
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

  connect(mTreeModel, &QAbstractItemModel::dataChanged, this, &DialogDbEditSelect::OnItemDataChanged);
}

DialogDbEditSelect::~DialogDbEditSelect()
{
  delete ui;
}


void DialogDbEditSelect::Init(const QVector<DbItemBS>& itemsList, DbTreeSchema* schema)
{
  mTreeModel->SetHeaders(schema->Table->Headers());

  QVector<TreeItemBS> modelItemList;
  modelItemList.reserve(itemsList.size());
  foreach (const DbItemBS& item, itemsList) {
    TreeItemBS modelItem(new TreeItemB(item, schema));
    modelItem->SetCheckable(true);
    modelItemList.append(modelItem);
  }
  mTreeModel->SetChildren(modelItemList);
  for (int i = 0; i < mTreeModel->columnCount(); i++) {
    ui->treeViewMain->resizeColumnToContents(i);
  }
  mSingleCheck = false;
}

void DialogDbEditSelect::SetSingleItem()
{
  mSingleCheck = true;
  mHasCheck = false;
}

void DialogDbEditSelect::GetResult(QVector<DbItemBS>& itemsList)
{
  itemsList.clear();
  for (int i = 0; i < mTreeModel->rowCount(); i++) {
    TreeItemB* item = mTreeModel->InvisibleRootItem()->Child(i);
    if (item->IsChecked()) {
      itemsList.append(item->Item());
    }
  }
}

void DialogDbEditSelect::OnItemDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
  Q_UNUSED(topLeft);
  Q_UNUSED(bottomRight);
  Q_UNUSED(roles);

  bool hasChecked = false;
  for (int i = 0; i < mTreeModel->rowCount(); i++) {
    TreeItemB* item = mTreeModel->InvisibleRootItem()->Child(i);
    if (item->IsChecked()) {
      hasChecked = true;
    }
  }

  if (mSingleCheck) {
    if (mHasCheck != hasChecked) {
      mTreeModel->BeginUpdate();
      for (int i = 0; i < mTreeModel->rowCount(); i++) {
        TreeItemB* item = mTreeModel->InvisibleRootItem()->Child(i);
        item->SetEnabled(hasChecked? item->IsChecked(): true);
      }

      mHasCheck = hasChecked;
      mTreeModel->Done();
    }
  }

  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(hasChecked);
}
