#include <QCompleter>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QFileDialog>

#include <Lib/Common/Icon.h>
#include <Lib/Common/StringUtils.h>
#include <Lib/Common/CsvWriter.h>
#include <Lib/Common/CsvReader.h>
#include <Lib/Log/Log.h>

#include "FormTableEdit.h"
#include "ui_FormTableEdit.h"


FormTableEdit::FormTableEdit(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::FormTableEdit)
  , mProxyModel(nullptr)
{
  Q_INIT_RESOURCE(DbUi);

  ui->setupUi(this);

  ui->actionExport->setIcon(IconFromImage(":/Icons/Csv.png", ":/Icons/Link.png"));
  ui->actionImport->setIcon(IconFromImage(":/Icons/Csv.png", ":/Icons/Unlink.png"));
  ui->widgetEdit->setEnabled(false);
}

FormTableEdit::~FormTableEdit()
{
  delete ui;
}


void FormTableEdit::Init(const FormTableEditAdapterAS& _TableAdapter)
{
  mTableAdapter = _TableAdapter;
  mTableAdapter->CreateEditControls(ui->widgetEditCtrl);

  mProxyModel = new QSortFilterProxyModel(this);
  mProxyModel->setSourceModel(mTableAdapter->Model());
  mProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  ui->treeViewTable->setModel(mProxyModel);
  ui->treeViewTable->header()->setSortIndicator(-1, Qt::AscendingOrder);

  if (auto sm = ui->treeViewTable->selectionModel()) {
    connect(sm, &QItemSelectionModel::currentChanged, this, &FormTableEdit::OnCurrentChanged);
    connect(sm, &QItemSelectionModel::selectionChanged, this, &FormTableEdit::OnSelectionChanged);
  }
  connect(ui->treeViewTable, &QTreeView::activated, this, &FormTableEdit::OnActivated);

  InitMenu();
}

void FormTableEdit::SetEnableClone(bool enabled)
{
  ui->toolButtonClone->setVisible(enabled);
  ui->actionClone->setVisible(enabled);
}

void FormTableEdit::AddToControls(QWidget* widget)
{
  ui->verticalLayoutControls->addWidget(widget);
}

void FormTableEdit::AddAction(QAction* action)
{
  ui->treeViewTable->addAction(action);
}

void FormTableEdit::Reload()
{
  if (!mTableAdapter) {
    return;
  }

  mTableAdapter->Load();

  UpdateCurrent(false);
  UpdateSelection(false);
}

void FormTableEdit::InitMenu()
{
  InitAction(ui->toolButtonCreate, ui->actionNew, "Create new %1");
  InitAction(ui->toolButtonEdit, ui->actionEdit, "Edit current %1");
  InitAction(ui->toolButtonClone, ui->actionClone, "Clone current %1");
  InitAction(ui->toolButtonExport, ui->actionExport, "Export currently loaded %1s to .csv file");
  InitAction(ui->toolButtonImport, ui->actionImport, "Import %1s from .csv file");
  InitAction(ui->toolButtonRemove, ui->actionRemove, "Remove selected %1s");
  QAction* sep = new QAction(this);
  sep->setSeparator(true);
  ui->treeViewTable->addAction(sep);
  ui->treeViewTable->addAction(ui->actionReload);

  ui->treeViewTable->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void FormTableEdit::InitAction(QToolButton* button, QAction* action, const QString& format)
{
  button->setDefaultAction(action);
  QString text = QString(format).arg(mTableAdapter->GetTableSchema()->Name.toLower());
  action->setText(text);
  action->setToolTip(text);
  ui->treeViewTable->addAction(action);
}

void FormTableEdit::UpdateCurrent(bool hasCurrent)
{
  ui->actionEdit->setEnabled(hasCurrent);
  ui->actionClone->setEnabled(hasCurrent);

  ChangeCurrent(hasCurrent);
}

void FormTableEdit::UpdateSelection(bool hasSelection)
{
  ui->actionRemove->setEnabled(hasSelection);

  ChangeSelection(hasSelection);
}

void FormTableEdit::Db2Csv()
{
  QString filename = QFileDialog::getSaveFileName(this, QString(), QString(), "Csv format (*.csv)");
  if (filename.isEmpty()) {
    return;
  }

  QFile file(filename);
  bool ok = file.open(QFile::WriteOnly);
  if (ok) {
    CsvWriter writer(&file);
    ok = mTableAdapter->ExportAll(&writer);
  }

  if (!ok) {
    QString actionText = QString("Export");
    if (file.error() != QFileDevice::NoError) {
      QMessageBox::warning(this, QString("%1 %2s").arg(actionText).arg(mTableAdapter->GetTableSchema()->Name.toLower())
                           , QString("Write file fail (%1)").arg(file.errorString()));
    } else {
      QMessageBox::warning(this, QString("%1 %2s").arg(actionText).arg(mTableAdapter->GetTableSchema()->Name.toLower())
                           , QString("Write file fail"));
    }
  }
}

void FormTableEdit::Csv2Db()
{
  QString filename = QFileDialog::getOpenFileName(this, QString(), QString(), "Csv format (*.csv);;Text files (*.txt)");
  if (filename.isEmpty()) {
    return;
  }

  QFile file(filename);
  bool ok = file.open(QFile::ReadOnly);
  QString info;
  if (ok) {
    CsvReader reader(&file);
    ok = mTableAdapter->ImportAll(&reader, &info);
  }

  QString actionText = QString("Export");
  if (!ok) {
    if (file.error() != QFileDevice::NoError) {
      QMessageBox::warning(this, QString("%1 %2s").arg(actionText).arg(mTableAdapter->GetTableSchema()->Name.toLower())
                           , QString("%2 file fail (%1)").arg(file.errorString()).arg("Read"));
    } else if (!info.isEmpty()) {
      QMessageBox::warning(this, QString("%1 %2s").arg(actionText).arg(mTableAdapter->GetTableSchema()->Name.toLower()), info);
    } else {
      QMessageBox::warning(this, QString("%1 %2s").arg(actionText).arg(mTableAdapter->GetTableSchema()->Name.toLower())
                           , QString("Backup fail"));
    }
  } else if (!info.isEmpty()) {
    QMessageBox::information(this, QString("%1 %2s").arg(actionText).arg(mTableAdapter->GetTableSchema()->Name.toLower()), info);
  }
  Reload();
}

int FormTableEdit::CurrentItem()
{
  QModelIndex index = ui->treeViewTable->selectionModel()->currentIndex();
  if (index.isValid()) {
    return mProxyModel->mapToSource(index).row();
  }
  return -1;
}

void FormTableEdit::ChangeSelection(bool hasSelection)
{
  Q_UNUSED(hasSelection);
}

void FormTableEdit::ChangeCurrent(bool hasCurrent)
{
  Q_UNUSED(hasCurrent);
}

void FormTableEdit::NewItem()
{
  bool loaded = mTableAdapter->NewEditItem();
  ui->widgetEdit->setEnabled(loaded);
  ui->pushButtonSave->setText("Create");

  mIsNew = true;
  emit Edit(mIsNew);
}

void FormTableEdit::EditItem(int index)
{
  bool loaded = mTableAdapter->LoadEditItem(index);
  ui->widgetEdit->setEnabled(loaded);
  ui->pushButtonSave->setText("Update");

  mIsNew = false;
  emit Edit(mIsNew);
}

void FormTableEdit::OnActivated(const QModelIndex& index)
{
  if (index.isValid()) {
    EditItem(mProxyModel->mapToSource(index).row());
  }
}

void FormTableEdit::OnCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
  Q_UNUSED(previous);
  Q_UNUSED(current);

  UpdateCurrent(current.isValid());
}

void FormTableEdit::OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_UNUSED(selected);
  Q_UNUSED(deselected);

  bool hasSelection = ui->treeViewTable->selectionModel()->hasSelection();

  UpdateSelection(hasSelection);
}

void FormTableEdit::on_actionNew_triggered()
{
  NewItem();
}

void FormTableEdit::on_actionEdit_triggered()
{
  QModelIndex index = ui->treeViewTable->currentIndex();
  if (index.isValid()) {
    EditItem(mProxyModel->mapToSource(index).row());
  } else {
    QString name = mTableAdapter->GetTableSchema()->Name.toLower();
    QMessageBox::warning(parentWidget(), "Edit " + name, "Set current " + name + " to edit it");
  }
}

void FormTableEdit::on_actionRemove_triggered()
{
  QModelIndexList list = ui->treeViewTable->selectionModel()->selectedRows();
  if (list.isEmpty()) {
    QMessageBox::warning(parentWidget(), "Remove " + mTableAdapter->GetTableSchema()->Name.toLower()
                         , "Select " + mTableAdapter->GetTableSchema()->Name.toLower() + "(s) to remove");
    return;
  }

  if (list.size() > 0) {
    QMessageBox mb(parentWidget());
    mb.setWindowTitle("Remove " + mTableAdapter->GetTableSchema()->Name.toLower());
    mb.setText(QString("Remove %1 ").arg(list.size()) + mTableAdapter->GetTableSchema()->Name.toLower() + "(s)");
    mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    mb.setDefaultButton(QMessageBox::No);
    auto res = mb.exec();
    if (res != QMessageBox::Yes) {
      return;
    }
  }

  auto indexes = ui->treeViewTable->selectionModel()->selectedRows();
  foreach (const QModelIndex& index, indexes) {
    if (!mTableAdapter->Delete(mProxyModel->mapToSource(index).row())) {
      QMessageBox::warning(parentWidget(), "Remove " + mTableAdapter->GetTableSchema()->Name.toLower(), "Remove fail");
      break;
    }
  }
  Reload();
}

void FormTableEdit::on_actionClone_triggered()
{
  QModelIndex index = ui->treeViewTable->currentIndex();
  if (!index.isValid()) {
    QMessageBox::warning(parentWidget(), "Clone " + mTableAdapter->GetTableSchema()->Name.toLower()
                         , "Set current " + mTableAdapter->GetTableSchema()->Name.toLower() + " to clone it");
    return;
  }

  if (!mTableAdapter->Clone(mProxyModel->mapToSource(index).row())) {
    QMessageBox::warning(parentWidget(), "Clone " + mTableAdapter->GetTableSchema()->Name.toLower(), "Clone fail");
    return;
  }
  Reload();
}

void FormTableEdit::on_actionExport_triggered()
{
  Db2Csv();
}

void FormTableEdit::on_actionImport_triggered()
{
  Csv2Db();
}

void FormTableEdit::on_actionReload_triggered()
{
  Reload();
}

void FormTableEdit::on_pushButtonSave_clicked()
{
  if (!mTableAdapter->SaveEditItem()) {
    return;
  }

  if (!mIsNew) {
    ui->widgetEdit->setEnabled(false);
  }
  Reload();
}
