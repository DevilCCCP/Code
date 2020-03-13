#include <QFileDialog>
#include <QFile>
#include <QSet>
#include <QItemSelection>

#include <Lib/Common/Icon.h>
#include <Lib/Log/Log.h>

#include "FormSourceStore.h"
#include "StorageScaner.h"
#include "ui_FormSourceStore.h"


const QString GetProgramName();

FormSourceStore::FormSourceStore(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormSourceStore)
  , mCellIcon(QIcon(":/Icons/Cluster.png"))
  , mStorageScaner(nullptr), mCellModel(new QStandardItemModel(this)), mObjectModel(new QStandardItemModel(this))
  , mAutoChange(false), mDeselectSkip(false)
{
  ui->setupUi(this);

  mCellModel->setHorizontalHeaderLabels(QStringList() << "Индекс" << "Объект" << "Время");
  ui->tableViewMain->setModel(mCellModel);

  mObjectModel->setHorizontalHeaderLabels(QStringList() << "Текущий" << "Новый");
  ui->tableViewObjects->setModel(mObjectModel);

  ui->toolButtonOpen->setDefaultAction(ui->actionOpenFile);
  ui->toolButtonScan->setDefaultAction(ui->actionScan);
  ui->actionScan->setEnabled(false);
  ui->widgetSettings->setEnabled(false);
  ui->progressBarScan->setVisible(false);
  ui->widgetUseHelper->setVisible(false);

  connect(mCellModel, &QStandardItemModel::itemChanged, this, &FormSourceStore::OnItemChanged);
  connect(mObjectModel, &QStandardItemModel::itemChanged, this, &FormSourceStore::OnObjectChanged);
  connect(ui->tableViewMain->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FormSourceStore::OnSelectionChanged);
}

FormSourceStore::~FormSourceStore()
{
  delete ui;

  if (mStorageScaner) {
    mStorageScaner->wait(2000);
  }
}

void FormSourceStore::closeEvent(QCloseEvent* event)
{
  if (mStorageScaner) {
    mStorageScaner->Stop();
  }

  QWidget::closeEvent(event);
}

int FormSourceStore::GetCellSize() const
{
  return ui->spinBoxCell->value();
}

int FormSourceStore::GetPageSize() const
{
  return ui->spinBoxPage->value();
}

int FormSourceStore::GetSize() const
{
  return ui->spinBoxSize->value();
}

QString FormSourceStore::Filename() const
{
  return ui->lineEditPath->text();
}

ContInfo FormSourceStore::GetContainerInfo() const
{
  return ContInfo(Filename(), GetCellSize(), GetPageSize(), GetSize());
}

int FormSourceStore::CellCount() const
{
  int count = 0;
  for (int i = 0; i < mCellModel->rowCount(); i++) {
    if (mCellModel->item(i, 0)->checkState() == Qt::Checked) {
      count++;
    }
  }
  return count;
}

void FormSourceStore::AddCells(int id, QVector<CellInfoEx>* cells)
{
  QMap<int, int> unitIdMap;
  for (int i = 0; i < mObjectModel->rowCount(); i++) {
    QStandardItem* item1 = mObjectModel->item(i, 0);
    QStandardItem* item2 = mObjectModel->item(i, 1);
    unitIdMap[item1->data().toInt()] = item2->data().toInt();
  }

  for (int i = 0; i < mCellModel->rowCount(); i++) {
    QStandardItem* item1 = mCellModel->item(i, 0);
    if (item1->checkState() == Qt::Checked) {
      cells->append(CellInfoEx());
      CellInfoEx& cellInfo = cells->last();
      cellInfo.SourceId     = id;
      cellInfo.Id           = item1->data(Qt::UserRole+1).toInt();
      cellInfo.UnitExportId = item1->data(Qt::UserRole+2).toInt();
      cellInfo.UnitImportId = unitIdMap[cellInfo.UnitExportId];
      cellInfo.StartTime    = item1->data(Qt::UserRole+3).toInt();
    }
  }
}

void FormSourceStore::OnPercentChanged(int perc)
{
  ui->progressBarScan->setValue(perc);
}

void FormSourceStore::OnScanEnded()
{
  mCellModel->removeRows(0, mCellModel->rowCount());
  QSet<int> unitIds;
  const CellInfoList& cellInfoList = mStorageScaner->GetCellInfoList();
  foreach (const CellInfo& cellInfo, cellInfoList) {
    unitIds.insert(cellInfo.UnitId);
    QStandardItem* item1 = new QStandardItem(mCellIcon, QString::number(cellInfo.Id));
    item1->setCheckable(true);
    item1->setCheckState(Qt::Checked);
    QStandardItem* item2 = new QStandardItem(QString::number(cellInfo.UnitId));
    QStandardItem* item3 = new QStandardItem(QDateTime::fromMSecsSinceEpoch(cellInfo.StartTime).toString());
    item1->setData(cellInfo.Id, Qt::UserRole+1);
    item1->setData(cellInfo.UnitId, Qt::UserRole+2);
    item1->setData(cellInfo.StartTime, Qt::UserRole+3);
    mCellModel->appendRow(QList<QStandardItem*>() << item1 << item2 << item3);
  }
  ui->tableViewMain->resizeColumnsToContents();

  mObjectModel->removeRows(0, mObjectModel->rowCount());
  ui->comboBoxObjects->clear();
  foreach (int unitId, unitIds) {
    ui->comboBoxObjects->addItem(QString::number(unitId));
    QStandardItem* item1 = new QStandardItem(QString::number(unitId));
    QStandardItem* item2 = new QStandardItem(QString::number(unitId));
    item1->setData(unitId, Qt::UserRole+1);
    item1->setEditable(false);
    item2->setData(unitId, Qt::UserRole+1);
    item2->setEditable(true);
    mObjectModel->appendRow(QList<QStandardItem*>() << item1 << item2);
  }

  mStorageScaner->deleteLater();
  mStorageScaner = nullptr;

  ui->actionScan->setEnabled(true);
  ui->progressBarScan->setVisible(false);
  ui->widgetUseHelper->setVisible(true);
}

void FormSourceStore::OnItemChanged(QStandardItem* item)
{
  if (mAutoChange) {
    return;
  }

  Qt::CheckState checkState = item->checkState();
  auto indexList = ui->tableViewMain->selectionModel()->selectedRows();
  mAutoChange = true;
  foreach (const QModelIndex& index, indexList) {
    mCellModel->itemFromIndex(index)->setCheckState(checkState);
  }
  mAutoChange = false;
  mDeselectSkip = true;
}

void FormSourceStore::OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_UNUSED(selected);

  if (mDeselectSkip) {
    mDeselectSkip = false;
    ui->tableViewMain->selectionModel()->select(deselected, QItemSelectionModel::Select);
  }
}

void FormSourceStore::OnObjectChanged(QStandardItem* item)
{
  bool ok;
  int id = item->text().toInt(&ok);
  if (ok) {
    item->setData(id);
  } else {
    item->setText(item->data().toString());
  }
}

void FormSourceStore::on_actionOpenFile_triggered()
{
  QString path = QFileDialog::getOpenFileName(this, GetProgramName()
                                              , QString(), QString::fromUtf8("Файл архива (*.bin)"));
  if (!path.isEmpty()) {
    ui->lineEditPath->setText(path);
  }
}

void FormSourceStore::on_actionScan_triggered()
{
  if (!mStorageScaner) {
    int cell = ui->spinBoxCell->value();
    int page = ui->spinBoxPage->value();
    int size = ui->spinBoxSize->value();
    QString path = ui->lineEditPath->text();
    mStorageScaner = new StorageScaner(ContInfo(path, cell, page, size), this);
    mStorageScaner->start();
    connect(mStorageScaner, &StorageScaner::PercentChanged, this, &FormSourceStore::OnPercentChanged);
    connect(mStorageScaner, &StorageScaner::finished, this, &FormSourceStore::OnScanEnded);

    ui->progressBarScan->setValue(0);
    ui->progressBarScan->setMaximum(size);
    ui->progressBarScan->setVisible(true);
  }

  ui->actionScan->setEnabled(false);
}

void FormSourceStore::on_lineEditPath_textChanged(const QString& path)
{
  if (mStorageScaner) {
    mStorageScaner->Stop();
  }

  QFile file(path);
  bool exists = file.exists();
  ui->widgetSettings->setEnabled(exists);
  ui->actionScan->setEnabled(exists);
  if (exists) {
    int cellSize = ui->spinBoxCell->value();
    qint64 sz = file.size();
    qint64 count = sz / cellSize;
    ui->spinBoxSize->setMaximum(count);
    ui->spinBoxSize->setValue(count);
  }
}

void FormSourceStore::on_labelAll_linkActivated(const QString&)
{
  for (int i = 0; i < mCellModel->rowCount(); i++) {
    mCellModel->item(i, 0)->setCheckState(Qt::Checked);
  }
}

void FormSourceStore::on_labelNone_linkActivated(const QString&)
{
  for (int i = 0; i < mCellModel->rowCount(); i++) {
    mCellModel->item(i, 0)->setCheckState(Qt::Unchecked);
  }
}

void FormSourceStore::on_labelObjectAdd_linkActivated(const QString&)
{
  int selUnitId = ui->comboBoxObjects->currentText().toInt();
  for (int i = 0; i < mCellModel->rowCount(); i++) {
    int unitId = mCellModel->index(i, 1).data().toInt();
    if (selUnitId == unitId) {
      mCellModel->item(i, 0)->setCheckState(Qt::Checked);
    }
  }
}

void FormSourceStore::on_labelObjectRemove_linkActivated(const QString&)
{
  int selUnitId = ui->comboBoxObjects->currentText().toInt();
  for (int i = 0; i < mCellModel->rowCount(); i++) {
    int unitId = mCellModel->index(i, 1).data().toInt();
    if (selUnitId == unitId) {
      mCellModel->item(i, 0)->setCheckState(Qt::Unchecked);
    }
  }
}
