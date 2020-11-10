#include <QLineEdit>
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

#include "FormTable.h"
#include "Filters/FormTableTime.h"
#include "ui_FormTable.h"


const int kLimitDefault = 1000;

void FormTable::CondCtrl::Init(QWidget* parent, QVBoxLayout* insertLayout, int insertIndex)
{
  HBoxLayout = new QHBoxLayout();
  ComboBoxType = new QComboBox(parent);
  EditWidget = nullptr;
  ToolButtonRemove = new QToolButton(parent);

  HBoxLayout->insertWidget(0, ToolButtonRemove);
  HBoxLayout->insertWidget(1, ComboBoxType);
  Completer = nullptr;

  insertLayout->insertLayout(insertIndex, HBoxLayout);
}

void FormTable::CondCtrl::ReplaceEdit(QWidget* edit)
{
  RemoveEdit();
  EditWidget = edit;
  HBoxLayout->insertWidget(2, edit);
}

void FormTable::CondCtrl::RemoveEdit()
{
  if (EditWidget) {
    EditWidget->deleteLater();
    EditWidget = nullptr;
  }
}

void FormTable::CondCtrl::Release()
{
  HBoxLayout->deleteLater();
  ComboBoxType->deleteLater();
  if (EditWidget) {
    EditWidget->deleteLater();
  }
  if (ToolButtonRemove) {
    ToolButtonRemove->deleteLater();
  }
  if (Completer) {
    Completer->deleteLater();
  }
}


FormTable::FormTable(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::FormTable)
  , mProxyModel(nullptr)
  , mHasCurrent(false), mHasSelection(false)
{
  Q_INIT_RESOURCE(DbUi);

  ui->setupUi(this);

  ui->widgetCount->setVisible(false);

  mConditions.append(CondCtrl());
  CondCtrl* mainCondCtrl = &mConditions.first();
  mainCondCtrl->HBoxLayout = ui->horizontalLayout;
  mainCondCtrl->ComboBoxType = ui->comboBoxTypeMain;
  mainCondCtrl->EditWidget = nullptr;
  mainCondCtrl->ToolButtonRemove = nullptr;

  ui->actionLimit->setChecked(true);
  ui->actionAddFilter->setIcon(IconFromImage(":/Icons/Filter.png", ":/Icons/Add.png"));
  ui->actionRemoveFilter->setIcon(IconFromImage(":/Icons/Filter.png", ":/Icons/Remove.png"));
  ui->actionExport->setIcon(IconFromImage(":/Icons/Csv.png", ":/Icons/Link.png"));
  ui->actionImport->setIcon(IconFromImage(":/Icons/Csv.png", ":/Icons/Unlink.png"));
  ui->toolButtonAddFilter->setDefaultAction(ui->actionAddFilter);
  ui->toolButtonLimits->setDefaultAction(ui->actionLimit);
  ui->toolButtonRefresh->setDefaultAction(ui->actionReload);
  ui->spinBoxLimit->setValue(kLimitDefault);

  ui->widgetCount->setVisible(false);
}

FormTable::~FormTable()
{
  delete ui;
}


void FormTable::Init(const FormTableAdapterAS& _TableAdapter, bool load)
{
  mTableAdapter = _TableAdapter;

  mProxyModel = new QSortFilterProxyModel(this);
  mProxyModel->setSourceModel(mTableAdapter->Model());
  mProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  ui->treeViewTable->setModel(mProxyModel);
  ui->treeViewTable->header()->setSortIndicator(-1, Qt::AscendingOrder);

  if (auto sm = ui->treeViewTable->selectionModel()) {
    connect(sm, &QItemSelectionModel::currentChanged, this, &FormTable::OnCurrentChanged);
    connect(sm, &QItemSelectionModel::selectionChanged, this, &FormTable::OnSelectionChanged);
  }
  connect(ui->treeViewTable, &QTreeView::activated, this, &FormTable::OnActivated);

  InitFilterOne(mConditions.first(), true);
  ChangeCondition(&mConditions.first(), 0);
  InitMenu();
  if (load) {
    Reload();
  }

  OnCurrentChanged(QModelIndex(), QModelIndex());
  OnSelectionChanged(QItemSelection(), QItemSelection());
}

void FormTable::SetEnableClone(bool enabled)
{
  ui->toolButtonClone->setVisible(enabled);
  ui->actionClone->setVisible(enabled);
}

void FormTable::SetReadOnly()
{
  ui->toolButtonCreate->setVisible(false);
  ui->toolButtonEdit->setVisible(false);
  ui->toolButtonClone->setVisible(false);
  ui->toolButtonImport->setVisible(false);
  ui->toolButtonRemove->setVisible(false);

  ui->actionNew->setVisible(false);
  ui->actionEdit->setVisible(false);
  ui->actionClone->setVisible(false);
  ui->actionImport->setVisible(false);
  ui->actionRemove->setVisible(false);
}

void FormTable::SetViewMode()
{
  ui->widgetControlsMain->setVisible(false);
}

void FormTable::SetLimit(int limit)
{
  ui->spinBoxLimit->setValue(limit);
}

void FormTable::SetSingleSelection(bool single)
{
  ui->treeViewTable->setSelectionMode(single? QTreeView::SingleSelection: QTreeView::ExtendedSelection);
}

int FormTable::CurrentItem()
{
  QModelIndex index = ui->treeViewTable->selectionModel()->currentIndex();
  if (index.isValid()) {
    return mProxyModel->mapToSource(index).row();
  }
  return -1;
}

void FormTable::AddToControls(QWidget* widget)
{
  ui->verticalLayoutControls->addWidget(widget);
}

void FormTable::AddAction(QAction* action)
{
  ui->treeViewTable->addAction(action);
}

void FormTable::Reload()
{
  if (!mTableAdapter) {
    return;
  }

  ApplyFilters();

  mLoadedCount = mTableAdapter->LoadQuery(mWhere);
  ui->spinBoxTotal->setValue(mLoadedCount);

  UpdateCurrent(false);
  UpdateSelection(false);
}

void FormTable::ClearStaticConditions()
{
  mStaticConditions.clear();
}

void FormTable::AddStaticCondition(const QString& condition)
{
  mStaticConditions << condition;
}

void FormTable::ClearSeek()
{
  mMainCondIsCustom = false;

  auto itr = mConditions.begin();
  for (itr++; itr != mConditions.end(); ) {
    CondCtrl* cond = &*itr;
    cond->Release();
    itr = mConditions.erase(itr);
  }
  ui->comboBoxTypeMain->setCurrentIndex(0);
}

void FormTable::AddSeek(const QString& column, const QString& text)
{
  CondCtrl* cond;
  if (mMainCondIsCustom || mConditions.isEmpty()) {
    AddNewFilter();
  }
  mMainCondIsCustom = true;
  cond = &mConditions.last();
  TableSchema* schema = mTableAdapter->GetTableSchema();

  int index = 0;
  foreach (const TableSchema::TableFilter& filter, schema->Filters) {
    if (filter.Name == column) {
      cond->ComboBoxType->setCurrentIndex(index);
      if (QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(cond->EditWidget)) {
        lineEdit->setText(text);
      }
      break;
    }
    index++;
  }
}

void FormTable::GetSelected(QList<int>& indexList)
{
  indexList.clear();

  auto indexes = ui->treeViewTable->selectionModel()->selectedRows();
  foreach (const QModelIndex& index, indexes) {
    indexList.append(mProxyModel->mapToSource(index).row());
  }
}

void FormTable::InitFilterOne(const CondCtrl& condCtrl, bool main)
{
  condCtrl.ComboBoxType->setSizePolicy(ui->comboBoxTypeMain->sizePolicy());
  condCtrl.ComboBoxType->setMinimumWidth(ui->comboBoxTypeMain->minimumWidth());

  if (condCtrl.ToolButtonRemove) {
    condCtrl.ToolButtonRemove->setAutoRaise(true);
    condCtrl.ToolButtonRemove->setIcon(ui->actionRemoveFilter->icon());
    condCtrl.ToolButtonRemove->setToolTip(ui->actionRemoveFilter->toolTip());
    condCtrl.ToolButtonRemove->setText(ui->actionRemoveFilter->text());

    connect(condCtrl.ToolButtonRemove, &QToolButton::clicked, this, &FormTable::OnButtonRemove_clicked);
  }

  TableSchema* schema = mTableAdapter->GetTableSchema();

  int index = 0;
  foreach (const TableSchema::TableFilter& filter, schema->Filters) {
    if (!main || filter.Main) {
      condCtrl.ComboBoxType->addItem(filter.Icon, filter.Name, index);
    }
    index++;
  }

  connect(condCtrl.ComboBoxType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged)
          , this, &FormTable::OnComboBoxTypeCurrentIndexChanged);
}

void FormTable::AddNewFilter()
{
  int insertIndex = mConditions.size();
  mConditions.append(CondCtrl());
  CondCtrl& condCtrl = mConditions.last();
  condCtrl.Init(this, ui->verticalLayoutMain, insertIndex);

  InitFilterOne(condCtrl, false);
  ChangeCondition(&condCtrl, 0);
}

void FormTable::InitMenu()
{
  InitAction(ui->toolButtonCreate, ui->actionNew, tr("Create new %1"));
  InitAction(ui->toolButtonEdit, ui->actionEdit, tr("Edit current %1"));
  InitAction(ui->toolButtonClone, ui->actionClone, tr("Clone current %1"));
  InitAction(ui->toolButtonExport, ui->actionExport, tr("Export currently loaded %1 items to .csv file"));
  InitAction(ui->toolButtonImport, ui->actionImport, tr("Import %1 items from .csv file"));
  if (mTableAdapter->CanBackup()) {
    InitAction(ui->toolButtonBackup, ui->actionBackup, tr("Backup currently loaded %1 items to .csv file"));
    InitAction(ui->toolButtonRestore, ui->actionRestore, tr("Restore %1 items from .csv file"));
  } else {
    ui->toolButtonBackup->setVisible(false);
    ui->toolButtonRestore->setVisible(false);
  }
  InitAction(ui->toolButtonRemove, ui->actionRemove, tr("Remove selected %1 items"));

  ui->treeViewTable->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void FormTable::InitAction(QToolButton* button, QAction* action, const QString& format)
{
  button->setDefaultAction(action);
  QString text = QString(format).arg(mTableAdapter->GetTableSchema()->Name.toLower());
  action->setText(text);
  action->setToolTip(text);
  ui->treeViewTable->addAction(action);
}

void FormTable::InitFilter(TableSchema::TableFilter::EEqualType type, FormTable::CondCtrl* selCond)
{
  switch (type) {
  case TableSchema::TableFilter::eLike:
  case TableSchema::TableFilter::eLikeInside:
  case TableSchema::TableFilter::eByteArray:
  case TableSchema::TableFilter::eEqGreater:
  case TableSchema::TableFilter::eEqual:
  case TableSchema::TableFilter::eEqualKey:
  case TableSchema::TableFilter::eEqText:
    if (!dynamic_cast<QLineEdit*>(selCond->EditWidget)) {
      QLineEdit* lineEdit = new QLineEdit(this);
//      connect(lineEdit, &QLineEdit::textEdited, this, &FormTable::OnLineEditTextEdited);
//      connect(lineEdit, &QLineEdit::textChanged, this, &FormTable::OnLineEditTextChanged);
      connect(lineEdit, &QLineEdit::returnPressed, this, &FormTable::OnLineEditReturnPressed);

      selCond->ReplaceEdit(lineEdit);
    }
    if (QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(selCond->EditWidget)) {
      lineEdit->setText("");
      switch (type) {
      case TableSchema::TableFilter::eLike:
        lineEdit->setPlaceholderText(tr("Text filter (use wildcards: '%' AND '_')"));
        lineEdit->setValidator(nullptr);
        lineEdit->setEnabled(true);
        break;
      case TableSchema::TableFilter::eLikeInside:
        lineEdit->setPlaceholderText(tr("Text filter (use wildcards: '%' AND '_')"));
        lineEdit->setValidator(nullptr);
        lineEdit->setEnabled(true);
        break;
      case TableSchema::TableFilter::eByteArray:
        lineEdit->setPlaceholderText(tr("Binary data as HEX text (example: 'aaCCdd')"));
        lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9A-Fa-f]*"), lineEdit));
        lineEdit->setEnabled(true);
        break;
      case TableSchema::TableFilter::eEqGreater:
        lineEdit->setPlaceholderText(tr("Integer value (minimum id)"));
        lineEdit->setValidator(new QIntValidator(lineEdit));
        lineEdit->setEnabled(true);
        break;
      case TableSchema::TableFilter::eEqual:
        lineEdit->setPlaceholderText(tr("Integer value"));
        lineEdit->setValidator(new QIntValidator(lineEdit));
        lineEdit->setEnabled(true);
        break;
      case TableSchema::TableFilter::eEqualKey:
        lineEdit->setPlaceholderText(tr("Select from list"));
        lineEdit->setValidator(nullptr);
        lineEdit->setEnabled(true);
        break;
      case TableSchema::TableFilter::eEqText:
        lineEdit->setPlaceholderText(tr("Text value"));
        lineEdit->setValidator(nullptr);
        lineEdit->setEnabled(true);
        break;
      case TableSchema::TableFilter::eTimeRange:
      case TableSchema::TableFilter::eDateRange:
        break;
      }
    }
    break;

  case TableSchema::TableFilter::eTimeRange:
    if (!dynamic_cast<FormTableTime*>(selCond->EditWidget)) {
      FormTableTime* formTableTime = new FormTableTime(true, this);

      selCond->ReplaceEdit(formTableTime);
    }
    if (FormTableTime* formTableTime = dynamic_cast<FormTableTime*>(selCond->EditWidget)) {
      formTableTime->Clear();
    }
    break;

  case TableSchema::TableFilter::eDateRange:
    if (!dynamic_cast<FormTableTime*>(selCond->EditWidget)) {
      FormTableTime* formTableDate = new FormTableTime(false, this);

      selCond->ReplaceEdit(formTableDate);
    }
    if (FormTableTime* formTableTime = dynamic_cast<FormTableTime*>(selCond->EditWidget)) {
      formTableTime->Clear();
    }
    break;
  }
}

void FormTable::ChangeCondition(FormTable::CondCtrl* selCond, int index)
{
  if (index >= 0) {
    const TableSchema::TableFilter& filter = mTableAdapter->GetTableSchema()->Filters.at(index);
    InitFilter(filter.EqualType, selCond);

    if (selCond->Completer) {
      selCond->Completer->deleteLater();
      selCond->Completer = nullptr;
    }
  }

  Reload();
}

void FormTable::UpdateCurrent(bool hasCurrent)
{
  mHasCurrent = hasCurrent;

  ui->actionEdit->setEnabled(mHasCurrent);
  ui->actionClone->setEnabled(mHasCurrent);

  emit OnChangeCurrent(mHasCurrent);
}

void FormTable::UpdateSelection(bool hasSelection)
{
  mHasSelection = hasSelection;

  ui->actionRemove->setEnabled(mHasSelection);

  emit OnChangeSelection(mHasSelection);
}

void FormTable::ApplyFilters()
{
  mWhere.clear();
  QMap<int, QStringList> filtersMap;
  foreach (const CondCtrl& cond, mConditions) {
    int index = cond.ComboBoxType->currentIndex();
    if (index < 0) {
      continue;
    }
    int ind = cond.ComboBoxType->itemData(index).toInt();
    const TableSchema::TableFilter& filter = mTableAdapter->GetTableSchema()->Filters.at(ind);
    QString whereOne;
    if (QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(cond.EditWidget)) {
      QString text = lineEdit->text();
      if (text.isEmpty()) {
        continue;
      }

      switch (filter.EqualType) {
      case TableSchema::TableFilter::eLike:
        whereOne = QString("%1 LIKE %2").arg(filter.Column).arg(ToSql(text + '%'));
        break;
      case TableSchema::TableFilter::eLikeInside:
        whereOne = QString("%1 LIKE %2").arg(filter.Column).arg(ToSql(QString('%') + text + '%'));
        break;
      case TableSchema::TableFilter::eByteArray:
        whereOne = QString("%1 = decode('%2', 'hex')").arg(filter.Column).arg(text.toLatin1().constData());
        break;
      case TableSchema::TableFilter::eEqGreater:
        whereOne = QString("%1 >= %2").arg(filter.Column).arg(text.toInt());
        break;
      case TableSchema::TableFilter::eEqual:
      case TableSchema::TableFilter::eEqualKey:
        whereOne = QString("%1 = %2").arg(filter.Column).arg(text);
        break;
      case TableSchema::TableFilter::eEqText:
        whereOne = QString("%1 = %2").arg(filter.Column).arg(ToSql(text));
        break;
      case TableSchema::TableFilter::eTimeRange:
      case TableSchema::TableFilter::eDateRange:
        break;
      }
    } else if (FormTableTime* formTableTime = dynamic_cast<FormTableTime*>(cond.EditWidget)) {
      if (!formTableTime->GetWhere(filter.Column, whereOne)) {
        continue;
      }
    }

    filtersMap[ind].append(whereOne);
  }

  QStringList whereAnd = QStringList() << mStaticConditions;
  for (auto itr = filtersMap.begin(); itr != filtersMap.end(); itr++) {
    const QStringList& whereOr = itr.value();
    QString orText = whereOr.join(" OR ");
    if (whereOr.size() > 1) {
      orText = QString("(") + orText + ")";
    }
    whereAnd.append(orText);
  }
  mQuery = mWhere = (!whereAnd.isEmpty())? QString("WHERE ") + whereAnd.join(" AND "): QString();

  mWhere.append(" ORDER BY _id");
  if (ui->actionLimit->isChecked()) {
    mWhere.append(QString(" LIMIT %1").arg(ui->spinBoxLimit->value()));
  }
}

void FormTable::Db2Csv(bool backup)
{
  QString filename = QFileDialog::getSaveFileName(this, QString(), QString(), tr("Csv format (*.csv)"));
  if (filename.isEmpty()) {
    return;
  }

  QFile file(filename);
  bool ok = file.open(QFile::WriteOnly);
  if (ok) {
    CsvWriter writer(&file);
    ok = backup? mTableAdapter->Backup(&writer): mTableAdapter->ExportAll(&writer);
  }

  if (!ok) {
    QString actionText = QString(tr("%1 %2 items"))
        .arg(backup? tr("Backup"): tr("Export")).arg(mTableAdapter->GetTableSchema()->Name.toLower());
    QString errorText = (file.error() != QFileDevice::NoError)
        ? QString(tr("Write file fail (%1)")).arg(file.errorString())
        : QString(tr("Write file fail"));

    QMessageBox::warning(this, actionText, errorText);
  }
}

void FormTable::Csv2Db(bool backup)
{
  QString filename = QFileDialog::getOpenFileName(this, QString(), QString(), tr("Csv format (*.csv);;Text files (*.txt)"));
  if (filename.isEmpty()) {
    return;
  }

  QFile file(filename);
  bool ok = file.open(QFile::ReadOnly);
  QString info;
  if (ok) {
    CsvReader reader(&file);
    ok = backup? mTableAdapter->Restore(&reader): mTableAdapter->ImportAll(&reader, &info);
  }

  QString actionText = QString(tr("%1 %2 items"))
      .arg(backup? tr("Restore"): tr("Export")).arg(mTableAdapter->GetTableSchema()->Name.toLower());

  if (!ok) {
    if (file.error() != QFileDevice::NoError) {
      QMessageBox::warning(this, actionText, QString(tr("%2 file fail (%1)")).arg(file.errorString()).arg(backup? tr("Write"): tr("Read")));
    } else if (!info.isEmpty()) {
      QMessageBox::warning(this, actionText, info);
    } else {
      QMessageBox::warning(this, actionText, backup? QString(tr("Restore fail")): QString(tr("Backup fail")));
    }
  } else if (!info.isEmpty()) {
    QMessageBox::information(this, actionText, info);
  }
  Reload();
}

void FormTable::OnActivated(const QModelIndex& index)
{
  if (index.isValid()) {
    emit OnEditItem(mProxyModel->mapToSource(index).row());
  }
}

void FormTable::OnCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
  Q_UNUSED(previous);
  Q_UNUSED(current);

  UpdateCurrent(current.isValid());
}

void FormTable::OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_UNUSED(selected);
  Q_UNUSED(deselected);

  bool hasSelection = ui->treeViewTable->selectionModel()->hasSelection();

  UpdateSelection(hasSelection);
}

void FormTable::OnComboBoxTypeCurrentIndexChanged(int index)
{
  int ind = -1;
  const QObject* combo = sender();
  CondCtrl* selCond = nullptr;
  for (auto itr = mConditions.begin(); itr != mConditions.end(); itr++) {
    const CondCtrl& cond = *itr;
    if (combo == cond.ComboBoxType) {
      ind = cond.ComboBoxType->itemData(index).toInt();
      selCond = &*itr;
    }
  }

  ChangeCondition(selCond, ind);
}

void FormTable::OnLineEditTextEdited(const QString& text)
{
  Q_UNUSED(text);
  //mListModel->setRowCount(0);
  //int ind = -1;
  //const QObject* edit = sender();
  //if (edit == ui->lineEditSeekMain) {
  //  ind = ui->comboBoxTypeMain->itemData(ui->comboBoxTypeMain->currentIndex()).toInt();
  //  mMainItemId = 0;
  //} else {
  //  for (auto itr = mConditions.begin(); itr != mConditions.end(); itr++) {
  //    CondCtrl* cond = &*itr;
  //    if (edit == cond->LineEditText) {
  //      ind = cond->ComboBoxType->itemData(cond->ComboBoxType->currentIndex()).toInt();
  //      cond->ItemId = 0;
  //      break;
  //    }
  //  }
  //}

  //if (ind >= 0) {
  //  const TableSchema::TableFilter& condition = mCurrentFind->Conditions.at(ind);
  //  if (condition.CondTable) {
  //    QString conditions = QString("WHERE name LIKE %1 ORDER BY name LIMIT 200").arg(ToSql(text + '%'));

  //    QList<TableItemBS> items;
  //    if (condition.CondTable->GetItems(conditions, items)) {
  //      condition.CondModel->SetList(items);
  //    }
  //  }
  //}
}

void FormTable::OnLineEditTextChanged(const QString& text)
{
  Q_UNUSED(text);

//  if (QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(sender())) {
//    if (!lineEdit->completer()) {
//      Reload();
//    } else if (text == "") {
//      emit OnZeroComplete();
//    }
//  }
}

//void FormTable::OnCompleterActivated(const QModelIndex& index)
//{
//  QObject* compl = sender();
//  for (auto itr = mConditions.begin(); itr != mConditions.end(); itr++) {
//    CondCtrl* cond = &*itr;
//    if (compl == cond->Completer) {
//      ItemBModel* model = static_cast<ItemBModel*>(cond->Completer->model());
//      TableItemBS item;
//      if (model->GetItem(index, item)) {
//        cond->ItemId = item->Id;
//      } else {
//        cond->ItemId = 0;
//      }
//      break;
//    }
//  }
//
//  if (mMainCompleter == compl) {
//    mMainItemId = 0;
//    int ind = ui->comboBoxTypeMain->itemData(ui->comboBoxTypeMain->currentIndex()).toInt();
//    if (ind >= 0) {
//      TableSchema::TableFilter* condition = &mCurrentFind->Conditions[ind];
//      if (condition->CondModel) {
//        TableItemBS item;
//        if (condition->CondModel->GetItem(index, item)) {
//          mMainItemId = item->Id;
//        }
//      }
//    }
//  }
//
//  Reload();
//}

void FormTable::OnLineEditReturnPressed()
{
  //mListModel->setRowCount(0);
  //int ind = -1;
  //const QObject* edit = sender();
  //CondCtrl* curCond = nullptr;
  //if (edit == ui->lineEditSeekMain) {
  //  ind = ui->comboBoxTypeMain->itemData(ui->comboBoxTypeMain->currentIndex()).toInt();
  //} else {
  //  for (auto itr = mConditions.begin(); itr != mConditions.end(); itr++) {
  //    CondCtrl* cond = &*itr;
  //    if (edit == cond->LineEditText) {
  //      ind = cond->ComboBoxType->itemData(cond->ComboBoxType->currentIndex()).toInt();
  //      curCond = cond;
  //      break;
  //    }
  //  }
  //}

  //if (ind >= 0) {
  //  const TableSchema::TableFilter& condition = mCurrentFind->Conditions.at(ind);
  //  if (condition.CondTable) {
  //    QString text = static_cast<const QLineEdit*>(edit)->text();
  //    QString conditions = QString("WHERE name = %1 ORDER BY name LIMIT 2").arg(ToSql(text));

  //    qint64 id = 0;
  //    QList<TableItemBS> items;
  //    condition.CondTable->GetItems(conditions, items);
  //    //condition.CondModel->SetList(QList<TableItemBS>());
  //    if (items.size() == 1) {
  //      id = items.first()->Id;
  //    }
  //    if (curCond) {
  //      curCond->ItemId = id;
  //    } else {
  //      mMainItemId = id;
  //    }
  //  }
  //}

  Reload();
}

void FormTable::OnButtonRemove_clicked()
{
  QObject* button = sender();
  for (auto itr = mConditions.begin(); itr != mConditions.end(); itr++) {
    CondCtrl* cond = &*itr;
    if (button == cond->ToolButtonRemove) {
      cond->Release();
      mConditions.erase(itr);
      break;
    }
  }

  Reload();
}

void FormTable::on_actionNew_triggered()
{
  emit OnNewItem();
}

void FormTable::on_actionAddFilter_triggered()
{
  AddNewFilter();
}

void FormTable::on_spinBoxLimit_valueChanged(int value)
{
  Q_UNUSED(value);

  if (ui->actionLimit->isChecked()) {
    Reload();
  }
}

void FormTable::on_actionLimit_triggered(bool checked)
{
  Q_UNUSED(checked);

  Reload();
}

void FormTable::on_actionEdit_triggered()
{
  QModelIndex index = ui->treeViewTable->currentIndex();
  if (index.isValid()) {
    emit OnEditItem(mProxyModel->mapToSource(index).row());
  } else {
    QString name = mTableAdapter->GetTableSchema()->Name.toLower();
    QString actionText = QString(tr("Edit %1")).arg(name);
    QMessageBox::warning(parentWidget(), actionText, QString(tr("Set current %1 to edit it")).arg(name));
  }
}

void FormTable::on_actionRemove_triggered()
{
  QModelIndexList list = ui->treeViewTable->selectionModel()->selectedRows();
  if (list.isEmpty()) {
    QMessageBox::warning(parentWidget(), QString(tr("Remove %1")).arg(mTableAdapter->GetTableSchema()->Name.toLower())
                         , QString(tr("Select %1 items to remove")).arg(mTableAdapter->GetTableSchema()->Name.toLower()));
    return;
  }

  QString actionText = QString(tr("Remove %1")).arg(mTableAdapter->GetTableSchema()->Name.toLower());
  if (list.size() > 0) {
    QMessageBox mb(parentWidget());
    mb.setWindowTitle(actionText);
    mb.setText(QString(tr("Removed %1 items")).arg(list.size()));
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
      QMessageBox::warning(parentWidget(), actionText, tr("Remove fail"));
      break;
    }
  }

  Reload();

  emit OnRemoveItems();
}

void FormTable::on_actionClone_triggered()
{
  QModelIndex index = ui->treeViewTable->currentIndex();
  QString actionText = QString(tr("Clone %1")).arg(mTableAdapter->GetTableSchema()->Name.toLower());
  if (!index.isValid()) {
    QMessageBox::warning(parentWidget(), actionText
                         , QString(tr("Set current %1 to clone it")).arg(mTableAdapter->GetTableSchema()->Name.toLower()));
    return;
  }

  if (!mTableAdapter->Clone(mProxyModel->mapToSource(index).row())) {
    QMessageBox::warning(parentWidget(), actionText, tr("Clone fail"));
    return;
  }
  Reload();
}

void FormTable::on_actionExport_triggered()
{
  Db2Csv(false);
}

void FormTable::on_actionImport_triggered()
{
  Csv2Db(false);
}

void FormTable::on_actionReload_triggered()
{
  Reload();
}


void FormTable::on_actionBackup_triggered()
{
  Db2Csv(true);
}

void FormTable::on_actionRestore_triggered()
{
  Csv2Db(true);
}
