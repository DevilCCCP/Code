#include <QTimer>
#include <QFormLayout>

#include <Lib/Common/Icon.h>
#include <Lib/Ui/DialogName.h>
#include <Lib/DbUi/TableModel.h>

#include "FormTableEditor.h"
#include "ui_FormTableEditor.h"


const int kWarningClearTimeoutMs = 15000;
const QString kWarningMsg("<html><head/><body><p><span style=\" color:#aa0000;\">%1</span></p></body></html>");

FormTableEditor::FormTableEditor(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormTableEditor)
  , mTableModel(nullptr)
  , mWarningTimer(new QTimer(this))
{
  Q_INIT_RESOURCE(DbUi);

  ui->setupUi(this);

  ui->toolButtonCreate->setDefaultAction(ui->actionCreate);
  ui->toolButtonEdit->setDefaultAction(ui->actionEdit);
  ui->toolButtonClone->setDefaultAction(ui->actionClone);
  ui->toolButtonRemove->setDefaultAction(ui->actionRemove);
  ui->toolButtonReload->setDefaultAction(ui->actionReload);

  ui->treeViewTable->addAction(ui->actionCreate);
  ui->treeViewTable->addAction(ui->actionEdit);
  ui->treeViewTable->addAction(ui->actionClone);
  ui->treeViewTable->addAction(ui->actionRemove);
  ui->treeViewTable->addAction(ui->actionReload);
  ui->treeViewTable->setContextMenuPolicy(Qt::ActionsContextMenu);

  ui->widgetWarning->setVisible(false);
  mWarningTimer->setSingleShot(true);
  mWarningTimer->setInterval(kWarningClearTimeoutMs);

  connect(mWarningTimer, &QTimer::timeout, this, &FormTableEditor::OnWarningTimeout);
}

FormTableEditor::~FormTableEditor()
{
  delete ui;
}


QSplitter* FormTableEditor::Splitter()
{
  return ui->splitter;
}

QTreeView* FormTableEditor::Tree()
{
  return ui->treeViewTable;
}

void FormTableEditor::InitTable(const TableEditSchemaS& _Schema, const DbTableBS& _Table)
{
  mSchema     = _Schema;
  mTable      = _Table;

  mTableModel = new TableModel(mTable, this);
  ui->treeViewTable->setModel(mTableModel);

  UpdateActions();
  connect(ui->treeViewTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FormTableEditor::OnTreeSelectionChanged);
}

void FormTableEditor::Reload()
{
  QList<DbItemBS> items;
  if (!mTable->Select("ORDER BY _id", items)) {
    Warning(tr("Load failed"));
    return;
  }

  mTableModel->UpdateList(items);
  for (int i = 0; i < ui->treeViewTable->model()->columnCount(); i++) {
    ui->treeViewTable->resizeColumnToContents(i);
  }
}

void FormTableEditor::CreateEdit()
{
  for (QObject* obj: ui->widgetEdit->children()) {
    if (obj != ui->formLayoutEdit) {
      obj->deleteLater();
    }
  }

  ui->widgetEdit->setEnabled(mCurrentEditItem);
  ui->widgetEditControls->setEnabled(mCurrentEditItem);

  if (mCurrentEditItem) {
    for (int i = 0; i < mSchema->Columns.size(); i++) {
      const TableEditSchema::ColumnSchema& columnSchema = mSchema->Columns.at(i);
      ui->formLayoutEdit->addRow(columnSchema.Lable, columnSchema.ColumnEdit->CreateControl(ui->widgetEdit));
      QVariant v = mCurrentEditItem->Data(i);
      columnSchema.ColumnEdit->LoadValue(v);
    }
    if (!mSchema->Compact) {
      QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      sizePolicy.setVerticalStretch(1);
      ui->widgetEdit->setSizePolicy(sizePolicy);
    }
  }

  emit ItemActivated();
}

void FormTableEditor::SaveEdit()
{
  for (int i = 0; i < mSchema->Columns.size(); i++) {
    const TableEditSchema::ColumnSchema& columnSchema = mSchema->Columns.at(i);
    QVariant v;
    columnSchema.ColumnEdit->SaveValue(v);
    mCurrentEditItem->SetData(i, v);
  }
  if (!mTable->Update(mCurrentEditItem)) {
    Warning(QString(tr("Update %1 fail")).arg(mSchema->Name));
  }
  mTableModel->UpdateItem(mCurrentEditItem);
}

void FormTableEditor::UpdateActions()
{
  int selCount = ui->treeViewTable->selectionModel()->selectedRows().size();

  ui->actionEdit->setEnabled(selCount == 1);
  ui->actionClone->setEnabled(selCount == 1);
  ui->actionRemove->setEnabled(selCount > 0);
}

void FormTableEditor::ClearWarning()
{
  ui->widgetWarning->setVisible(false);
  mWarningTimer->stop();
}

void FormTableEditor::Warning(const QString& text)
{
  ui->labelWarning->setText(kWarningMsg.arg(text));
  ui->widgetWarning->setVisible(true);
  mWarningTimer->start();
}

void FormTableEditor::OnTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_UNUSED(selected);
  Q_UNUSED(deselected);

  if (mCurrentEditItem) {
    mCurrentEditItem.clear();
    CreateEdit();
  }

  UpdateActions();

  emit SelectionChanged();
}

void FormTableEditor::OnWarningTimeout()
{
  ui->widgetWarning->setVisible(false);
}

void FormTableEditor::on_actionReload_triggered()
{
  ClearWarning();

  Reload();
}

void FormTableEditor::on_pushButtonSave_clicked()
{
  ClearWarning();

  SaveEdit();
}

void FormTableEditor::on_actionCreate_triggered()
{
  DbItemBS item;
  if (!mTable->Create(item)) {
    Warning(tr("Create item failed"));
    return;
  }

  mTableModel->AddItem(item);
  mCurrentEditItem = item;
  CreateEdit();
}

void FormTableEditor::on_actionEdit_triggered()
{
  QModelIndexList indexList = ui->treeViewTable->selectionModel()->selectedRows();
  if (indexList.size() != 1) {
    Warning(tr("Select one item to edit"));
    return;
  }
  QModelIndex index = indexList.first();
  mTableModel->GetItem(index, mCurrentEditItem);

  CreateEdit();
}

void FormTableEditor::on_actionClone_triggered()
{
  QModelIndexList indexList = ui->treeViewTable->selectionModel()->selectedRows();
  if (indexList.size() != 1) {
    Warning(tr("Select one item to clone"));
    return;
  }
  QModelIndex index = indexList.first();
  mTableModel->GetItem(index, mCurrentEditItem);

  mCurrentEditItem->Id = 0;
  if (!mTable->Insert(mCurrentEditItem)) {
    Warning(tr("Create item failed"));
    mCurrentEditItem.clear();
  }

  CreateEdit();
}

void FormTableEditor::on_actionRemove_triggered()
{
  QModelIndexList indexList = ui->treeViewTable->selectionModel()->selectedRows();
  if (indexList.isEmpty()) {
    Warning(tr("Select item(s) to remove"));
    return;
  }

  QList<DbItemBS> newList = mTableModel->getItems();
  for (const QModelIndex& index: indexList) {
    DbItemBS item;
    if (!mTableModel->GetItem(index, item) || !item) {
      continue;
    }
    if (!mTable->Delete(item->Id)) {
      Warning(tr("Remove item failed"));
      return;
    }
    newList.removeOne(item);
  }

  mTableModel->UpdateList(newList);

  if (mCurrentEditItem) {
    mCurrentEditItem.clear();
    CreateEdit();
  }
}

void FormTableEditor::on_treeViewTable_activated(const QModelIndex& index)
{
  mTableModel->GetItem(index, mCurrentEditItem);

  CreateEdit();
}
