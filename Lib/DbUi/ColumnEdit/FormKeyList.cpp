#include <QAbstractTableModel>
#include <QStandardItemModel>

#include "FormKeyList.h"
#include "ui_FormKeyList.h"


FormKeyList::FormKeyList(QAbstractTableModel* _Model, QWidget* parent)
  : QWidget(parent), ui(new Ui::FormKeyList)
  , mModel(_Model), mListModel(new QStandardItemModel(this))
{
  ui->setupUi(this);

  ui->toolButtonAdd->setDefaultAction(ui->actionAdd);
  ui->toolButtonRemove->setDefaultAction(ui->actionRemove);

  ui->actionAdd->setEnabled(false);
  ui->actionRemove->setEnabled(false);

  ui->comboBoxNew->setModel(mModel);
  ui->treeViewList->setModel(mListModel);
  connect(ui->comboBoxNew, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged)
          , this, &FormKeyList::OnComboNewIndexChanged);
  if (auto sm = ui->treeViewList->selectionModel()) {
    connect(sm, &QItemSelectionModel::selectionChanged, this, &FormKeyList::OnListSelectionChanged);
  }
}

FormKeyList::~FormKeyList()
{
  delete ui;
}


void FormKeyList::SetList(const QStringList& list)
{
  mListModel->removeRows(0, mListModel->rowCount());
  QMap<qint64, int> idMap;
  for (int i = 0; i < mModel->rowCount(); i++) {
    qint64 id = mModel->data(mModel->index(i, 0), Qt::UserRole + 1).toLongLong();
    idMap[id] = i;
  }
  for (int i = 0; i < list.size(); i++) {
    AddItem(idMap[list.at(i).toLongLong()]);
  }
}

void FormKeyList::GetList(QStringList& list)
{
  for (int i = 0; i < mListModel->rowCount(); i++) {
    list.append(QString::number(mListModel->data(mListModel->index(i, 0), Qt::UserRole + 1).toLongLong()));
  }
}

void FormKeyList::AddItem(int ind)
{
  auto index = mModel->index(ind, 0);
  QVariant icon = mModel->data(index, Qt::DecorationRole);
  QVariant name = mModel->data(index);
  QStandardItem* item = new QStandardItem(icon.isValid()? icon.value<QIcon>(): QIcon(), name.isValid()? name.toString(): "<invalid unit>");
  item->setData(mModel->data(index, Qt::UserRole + 1));
  mListModel->appendRow(item);
}

void FormKeyList::OnComboNewIndexChanged(int index)
{
  ui->actionAdd->setEnabled(index >= 0);
}

void FormKeyList::OnListSelectionChanged()
{
  ui->actionRemove->setEnabled(ui->treeViewList->selectionModel()->hasSelection());
}

void FormKeyList::on_actionAdd_triggered()
{
  int ind = ui->comboBoxNew->currentIndex();
  if (ind >= 0) {
    AddItem(ind);
  }
}

void FormKeyList::on_actionRemove_triggered()
{
  QModelIndexList selIndexes = ui->treeViewList->selectionModel()->selectedRows();
  QVector<int> rows;
  rows.reserve(selIndexes.size());
  foreach (const QModelIndex& index, selIndexes) {
    rows.append(index.row());
  }
  qSort(rows);
  for (int i = rows.size() - 1; i >= 0; i--) {
    mListModel->removeRow(rows.at(i));
  }
}
