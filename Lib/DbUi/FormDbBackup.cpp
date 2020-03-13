#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QMutexLocker>

#include <Lib/Log/Log.h>

#include "FormDbBackup.h"
#include "DbBackupThread.h"
#include "ui_FormDbBackup.h"


const QString kHtmlHead = "<html><head><style type=\"text/css\">"
                          " p, li { white-space: pre-wrap; margin: 0px 0px 0px 0px; }</style></head>"
                          "<body>";
const QString kHtmlTail = "</body>";
const QString kFormatInfo = "<p><span style=\"color: darkgreen;\">%1</span></p>";
const QString kFormatError = "<p><span style=\"color: darkred;\">%1</span></p>";

FormDbBackup::FormDbBackup(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormDbBackup)
  , mFileDialog(new QFileDialog(this)), mModel(new QStandardItemModel(this)), mTableIcon(":/Icons/Table.png")
  , mManual(false)
{
  Q_INIT_RESOURCE(DbUi);

  ui->setupUi(this);

  ui->treeViewTables->setModel(mModel);

  ui->widgetResults->setVisible(false);
  ui->pushButtonStop->setVisible(false);

  ui->toolButtonTree->setDefaultAction(ui->actionExpand);
  ui->toolButtonCollapse->setDefaultAction(ui->actionCollapse);

  connect(ui->treeViewTables->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FormDbBackup::OnSelectionChanged);
  connect(mModel, &QStandardItemModel::itemChanged, this, &FormDbBackup::OnItemChanged);
  connect(this, &FormDbBackup::Update, this, &FormDbBackup::OnUpdate, Qt::QueuedConnection);

  mFileDialog->setDirectory(qApp->applicationDirPath());
  mFileDialog->setNameFilters(QStringList() << "Backup files (*.bak)" << "All files (*.*)");
}

FormDbBackup::~FormDbBackup()
{
  delete ui;

  if (mDbBackupThread) {
    mDbBackupThread->wait();
    mDbBackupThread->deleteLater();
  }
}


void FormDbBackup::closeEvent(QCloseEvent* event)
{
  mCancel = true;

  QWidget::closeEvent(event);
}

void FormDbBackup::AddTable(const QString& tableInfo, const QIcon& tableIcon, const QStringList& tableRef)
{
  QStringList tableNames = tableInfo.split('|');
  QString tableNameV = tableNames.at(0);
  QString tableName = tableNames.size() > 1? tableNames.at(1): tableNames.at(0);
  mTableNames[tableNameV] = tableName;

  QList<QStandardItem*> roots;
  for (auto itr = mModelUseMap.find(tableNameV); itr != mModelUseMap.end() && itr.key() == tableNameV; itr++) {
    QStandardItem* root = itr.value();
    root->setIcon(!tableIcon.isNull()? tableIcon: mTableIcon);
    roots.append(root);
  }

  QStandardItem* root = new QStandardItem(!tableIcon.isNull()? tableIcon: mTableIcon, tableNameV);
  root->setCheckable(true);
  root->setCheckState(Qt::Checked);
  mModelUseMap.insertMulti(tableNameV, root);
  if (roots.isEmpty()) {
    mModel->appendRow(root);
  }
  roots.append(root);

  if (!tableRef.isEmpty()) {
    foreach (QStandardItem* root, roots) {
      QList<QStandardItem*> rows;
      foreach (const QString& subTable, tableRef) {
        QStandardItem* item = CloneItemTree(subTable);
        mModelUseMap.insertMulti(subTable, item);
        rows << item;
      }
      root->appendRows(rows);
    }
  }

  Prepare();

  UpdateAllChecked();
}

void FormDbBackup::Prepare()
{
  mTableOrder.clear();

  QSet<QString> used;
  while (used.size() < mTableNames.size()) {
    int szStart = mTableOrder.size();
    for (auto itr = mModelUseMap.begin(); itr != mModelUseMap.end(); itr++) {
      const QString& tableName = itr.key();
      QStandardItem* item = itr.value();
      if (used.contains(tableName)) {
        continue;
      }
      bool childsUsed = true;
      for (int i = 0; i < item->rowCount(); i++) {
        if (QStandardItem* child = item->child(i)) {
          if (!used.contains(child->text())) {
            childsUsed = false;
            break;
          }
        }
      }
      if (childsUsed) {
        mTableOrder.append(mTableNames[tableName].split(';'));
        used.insert(tableName);
      }
    }
    if (mTableOrder.size() == szStart) {
      break;
    }
  }
  ui->treeViewTables->expandAll();
}

QStandardItem* FormDbBackup::CloneItemTree(const QString& table)
{
  QStandardItem* item = new QStandardItem(mTableIcon, table);
  item->setCheckable(true);
  item->setCheckState(Qt::Checked);

  auto itr = mModelUseMap.find(table);
  if (itr == mModelUseMap.end()) {
    return item;
  }

  QStandardItem* root = itr.value();
  item->setIcon(root->icon());
  if (root->hasChildren()) {
    QList<QStandardItem*> rows;
    for (int i = 0; i < root->rowCount(); i++) {
      if (QStandardItem* child = root->child(i)) {
        rows << CloneItemTree(child->text());
      }
    }
    item->appendRows(rows);
  }
  return item;
}

void FormDbBackup::UpdateLog()
{
  ui->textEditLog->setHtml(kHtmlHead + mLogText + kHtmlTail);
  QTextCursor cursor = ui->textEditLog->textCursor();
  cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
  ui->textEditLog->setTextCursor(cursor);
}

void FormDbBackup::UpdateSelectedChecked()
{
  bool hasChecked = false;
  bool hasUnchecked = false;
  QModelIndexList indexes = ui->treeViewTables->selectionModel()->selectedIndexes();
  foreach (const QModelIndex index, indexes) {
    QStandardItem* item = mModel->itemFromIndex(index);
    switch (item->checkState()) {
    case Qt::Unchecked: hasUnchecked = true; break;
    case Qt::Checked: hasChecked = true; break;
    default: break;
    }
  }

  if (hasChecked && hasUnchecked) {
    ui->checkBoxSelected->setCheckState(Qt::PartiallyChecked);
  } else if (hasChecked) {
    ui->checkBoxSelected->setCheckState(Qt::Checked);
  } else {
    ui->checkBoxSelected->setCheckState(Qt::Unchecked);
  }
}

void FormDbBackup::UpdateAllChecked()
{
  bool hasChecked = false;
  bool hasUnchecked = false;
  foreach (QStandardItem* item, mModelUseMap.values()) {
    switch (item->checkState()) {
    case Qt::Unchecked: hasUnchecked = true; break;
    case Qt::Checked: hasChecked = true; break;
    default: break;
    }
  }

  if (hasChecked && hasUnchecked) {
    ui->checkBoxAll->setCheckState(Qt::PartiallyChecked);
  } else if (hasChecked) {
    ui->checkBoxAll->setCheckState(Qt::Checked);
  } else {
    ui->checkBoxAll->setCheckState(Qt::Unchecked);
  }
}

void FormDbBackup::CheckTree(QStandardItem* root, bool checked)
{
  if (!root->hasChildren()) {
    return;
  }

  for (int i = 0; i < root->rowCount(); i++) {
    if (QStandardItem* child = root->child(i)) {
      child->setCheckState(checked? Qt::Checked: Qt::Unchecked);
      CheckTree(child, checked);
    }
  }
}

void FormDbBackup::CheckSelected(bool checked)
{
  mManual = true;
  QModelIndexList indexes = ui->treeViewTables->selectionModel()->selectedIndexes();
  foreach (const QModelIndex index, indexes) {
    QStandardItem* item = mModel->itemFromIndex(index);
    item->setCheckState(checked? Qt::Checked: Qt::Unchecked);
  }
  mManual = false;

  UpdateAllChecked();
}

void FormDbBackup::CheckAll(bool checked)
{
  mManual = true;
  foreach (QStandardItem* item, mModelUseMap.values()) {
    item->setCheckState(checked? Qt::Checked: Qt::Unchecked);
  }
  mManual = false;

  UpdateSelectedChecked();
}

bool FormDbBackup::OnQueryContinue()
{
  QMutexLocker lock(&mPrivate);
  return !mCancel;
}

void FormDbBackup::OnPercent(int perc)
{
  {
    QMutexLocker lock(&mPrivate);
    mBackupPercent = perc;
  }
  emit Update();
}

void FormDbBackup::OnTable(int index)
{
  {
    QMutexLocker lock(&mPrivate);
    mBackupIndex = index;
  }
  emit Update();
}

void FormDbBackup::OnError(const QString& text)
{
  {
    QMutexLocker lock(&mPrivate);
    mLogText.append(kFormatError.arg(text));
  }

  emit Update();
}

void FormDbBackup::OnInfo(const QString& text)
{
  {
    QMutexLocker lock(&mPrivate);
    mLogText.append(kFormatInfo.arg(text));
  }

  emit Update();
}

void FormDbBackup::Done()
{
  {
    QMutexLocker lock(&mPrivate);
    mDone = true;
  }

  emit Update();
}

void FormDbBackup::OnItemChanged(QStandardItem* item)
{
  if (mManual) {
    return;
  }

  mManual = true;
  CheckTree(item, item->checkState() == Qt::Checked);
  mManual = false;

  UpdateAllChecked();
}

void FormDbBackup::OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_UNUSED(selected);
  Q_UNUSED(deselected);

  UpdateSelectedChecked();
}

void FormDbBackup::OnUpdate()
{
  QMutexLocker lock(&mPrivate);
  ui->progressBarPercent->setValue(mBackupPercent);
  ui->progressBarTotal->setValue(mBackupIndex);
  UpdateLog();
  if (mDone) {
    ui->pushButtonStop->setVisible(false);
    ui->pushButtonStart->setVisible(true);
    ui->widgetProgress->setVisible(false);
    ui->widgetDone->setVisible(true);
  }
}

void FormDbBackup::on_pushButtonSelectFile_clicked()
{
  if (ui->radioButtonBackup->isChecked()) {
    mFileDialog->setAcceptMode(QFileDialog::AcceptSave);
    mFileDialog->setFileMode(QFileDialog::AnyFile);
  } else {
    mFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    mFileDialog->setFileMode(QFileDialog::ExistingFile);
  }
  if (mFileDialog->exec() && !mFileDialog->selectedFiles().isEmpty()) {
    ui->lineEditFile->setText(mFileDialog->selectedFiles().first());
  }
}

void FormDbBackup::on_pushButtonStart_clicked()
{
  bool backup = ui->radioButtonBackup->isChecked();
  if (ui->lineEditFile->text().isEmpty()) {
    QMessageBox::warning(this, this->parentWidget()->windowTitle()
                         , backup? "Select file to backup DB to"
                                 : "Select file to restore DB from");
    return;
  }

  if (!mDbBackupThread) {
    mDbBackupThread.reset(new DbBackupThread(this, mTableOrder));
  }

  QSet<QString> used;
  for (auto itr = mModelUseMap.begin(); itr != mModelUseMap.end(); itr++) {
    const QString& tableName = itr.key();
    QStandardItem* item = itr.value();
    if (item->checkState() == Qt::Checked) {
      used.unite(mTableNames[tableName].split(';').toSet());
    }
  }

  mBackupIndex = 0;
  mBackupPercent = 0;
  mLogText = QString(kFormatInfo).arg(backup? "Starting backup": "Starting restore");
  mLogText.append(QString(kFormatInfo).arg(QString("Tables: ") + QStringList(used.toList()).join("; ")));
  mLogText.append(QString(kFormatInfo).arg(QString("Order: ") + mTableOrder.join("; ")));
  ui->widgetResults->setVisible(true);
  ui->pushButtonStart->setVisible(false);
  ui->pushButtonStop->setVisible(true);
  ui->progressBarTotal->setMaximum(used.size());
  UpdateLog();
  mCancel = false;
  mDone = false;
  ui->widgetProgress->setVisible(true);
  ui->widgetDone->setVisible(false);
  mDbBackupThread->Start(ui->lineEditFile->text(), used, backup);
}

void FormDbBackup::on_pushButtonStop_clicked()
{
  mCancel = true;
}

void FormDbBackup::on_actionExpand_triggered()
{
  ui->treeViewTables->expandAll();
}

void FormDbBackup::on_actionCollapse_triggered()
{
  ui->treeViewTables->collapseAll();
}

void FormDbBackup::on_checkBoxSelected_toggled(bool checked)
{
  CheckSelected(checked);
}

void FormDbBackup::on_checkBoxAll_toggled(bool checked)
{
  CheckAll(checked);
}
