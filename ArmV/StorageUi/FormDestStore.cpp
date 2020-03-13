#include <QFileDialog>
#include <QFile>
#include <QSet>
#include <QMessageBox>
#include <QDirIterator>
#include <QRegExp>

#include <Lib/Common/Icon.h>
#include <Lib/Common/Var.h>
#include <LibV/Storage/DbIndex.h>
#include <Lib/Log/Log.h>

#include "FormDestStore.h"
#include "ui_FormDestStore.h"
#include "StorageTransfer.h"
#include "DbSaver.h"


const QString GetProgramName();

FormDestStore::FormDestStore(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormDestStore)
  , mCellIcon(QIcon(":/Icons/Cluster.png")), mDbIcon(QIcon(":/Icons/Db.png"))
  , mCellModel(new QStandardItemModel(this)), mStorageTransfer(nullptr)
  , mSizeManual(false), mSizeDbManual(false)
{
  ui->setupUi(this);

  mCellModel->setHorizontalHeaderLabels(QStringList() << "Источник" << "Индекс" << "Объект" << "Время");
  ui->tableViewMain->setModel(mCellModel);

  ui->toolButtonOpen->setDefaultAction(ui->actionOpenFile);

  ui->progressBarFile->setVisible(false);
  ui->progressBarDb->setVisible(false);

  ui->pushButtonStop->setVisible(false);
  ui->labelSuccess->setVisible(false);
  ui->labelErrorText->setVisible(false);
  ui->labelSuccessDb->setVisible(false);
  ui->labelErrorTextDb->setVisible(false);

  QDirIterator varDirItr(GetVarPath(), QStringList() << "storage_*.ini", QDir::Files);
  while (varDirItr.hasNext()) {
    varDirItr.next();
    QString filename = varDirItr.fileName();
    QRegExp r("^(storage_([0-9]+)).ini$");
    if (r.indexIn(filename) >= 0) {
      QString filename = r.cap(1);
      QString id       = r.cap(2);
      ui->comboBoxDb->addItem(mDbIcon, id, filename);
    }
  }
}

FormDestStore::~FormDestStore()
{
  delete ui;

  if (mStorageTransfer) {
    mStorageTransfer->wait(2000);
  }
}


void FormDestStore::closeEvent(QCloseEvent* event)
{
  if (mStorageTransfer) {
    mStorageTransfer->Stop();
  }

  QWidget::closeEvent(event);
}

void FormDestStore::SetCellSize(int size)
{
  ui->spinBoxCell->setValue(size);
  ui->spinBoxCellDb->setValue(size);
}

void FormDestStore::SetPageSize(int size)
{
  ui->spinBoxPage->setValue(size);
  ui->spinBoxPageDb->setValue(size);
}

void FormDestStore::SetSize(int size)
{
  if (!mSizeManual) {
    ui->spinBoxSize->setValue(size);
  }
  if (!mSizeDbManual) {
    ui->spinBoxSizeDb->setValue(size);
  }
}

bool CellLess(const CellInfoEx& a, const CellInfoEx& b)
{
  return a.StartTime < b.StartTime;
}

void FormDestStore::SetSources(const QVector<ContInfo>& conts, const QVector<CellInfoEx>& cells, const QVector<QString>& names)
{
  mSourceCont  = conts;
  mSourceCells = cells;
  mSourceNames = names;

  std::sort(mSourceCells.begin(), mSourceCells.end(), CellLess);

  LoadCells();
}

void FormDestStore::LoadCells()
{
  mCellModel->removeRows(0, mCellModel->rowCount());
  if (ui->spinBoxSize->value() < mSourceCells.size()) {
    int over = mSourceCells.size() - ui->spinBoxSize->value();
    mFileCells = mSourceCells.mid(over, ui->spinBoxSize->value());
  } else {
    mFileCells = mSourceCells;
  }
  for (int i = 0; i < mFileCells.size(); i++) {
    const CellInfoEx& cellInfo = mFileCells.at(i);
    QStandardItem* item1 = new QStandardItem(mSourceNames.at(cellInfo.SourceId));
    QStandardItem* item2 = new QStandardItem(mCellIcon, QString::number(cellInfo.Id));
    QStandardItem* item3 = new QStandardItem(QString::number(cellInfo.UnitImportId));
    QStandardItem* item4 = new QStandardItem(QDateTime::fromMSecsSinceEpoch(cellInfo.StartTime).toString());
    mCellModel->appendRow(QList<QStandardItem*>() << item1 << item2 << item3 << item4);
  }
}

bool FormDestStore::DoWrite2()
{
  if (!ValidateFile() || !ValidateDb()) {
    return false;
  }
  if (ui->spinBoxCell->value() != ui->spinBoxCellDb->value()) {
    QMessageBox::warning(this, GetProgramName(), "Кластеры у файла и у БД должны совпадать");
    return false;
  }
  if (ui->spinBoxPage->value() != ui->spinBoxPageDb->value()) {
    QMessageBox::warning(this, GetProgramName(), "Страницы у файла и у БД должны совпадать");
    return false;
  }
  if (ui->spinBoxSize->value() != ui->spinBoxSizeDb->value()) {
    QMessageBox::warning(this, GetProgramName(), "Размеры у файла и у БД должны совпадать");
    return false;
  }

  PrepareTransfer();
  PrepareDb(true);
  StartWork();
  mStorageTransfer->start();

  return false;
}

bool FormDestStore::DoWriteFile()
{
  if (!ValidateFile()) {
    return false;
  }

  PrepareTransfer();
  StartWork();
  mStorageTransfer->start();

  return false;
}

bool FormDestStore::DoWriteDb()
{
  if (!ValidateDb()) {
    return false;
  }
  if (mSourceCont.size() != 1) {
    QMessageBox::warning(this, GetProgramName(), "Должен быть только один источник для записи в БД");
    return false;
  }
  if (mSourceCont.first().CellSize != ui->spinBoxCellDb->value()) {
    QMessageBox::warning(this, GetProgramName(), "Кластеры у источника и у БД должны совпадать");
    return false;
  }
  if (mSourceCont.first().CellPageSize != ui->spinBoxPageDb->value()) {
    QMessageBox::warning(this, GetProgramName(), "Страницы у источника и у БД должны совпадать");
    return false;
  }
  if (mSourceCont.first().Capacity != ui->spinBoxSizeDb->value()) {
    QMessageBox::warning(this, GetProgramName(), "Размеры у источника и у БД должны совпадать");
    return false;
  }

  PrepareDb(false);
  StartWork();
  mDbSaver->start();

  return false;
}

bool FormDestStore::ValidateFile()
{
  if (ui->lineEditPath->text().isEmpty()) {
    QMessageBox::warning(this, GetProgramName(), "Необходимо указать конечный файл архива");
    ui->lineEditPath->setFocus();
    return false;
  }
  return !mStorageTransfer;
}

bool FormDestStore::ValidateDb()
{
  return true;
}

void FormDestStore::PrepareTransfer()
{
  ContInfo destContInfo(ui->lineEditPath->text(), ui->spinBoxCell->value(), ui->spinBoxPage->value(), ui->spinBoxSize->value());
  mStorageTransfer = new StorageTransfer(mSourceCont, destContInfo, mFileCells, this);
  connect(mStorageTransfer, &StorageTransfer::PercentChanged, this, &FormDestStore::OnPercentChanged);
  connect(mStorageTransfer, &StorageTransfer::finished, this, &FormDestStore::OnTransferEnded);

  ui->progressBarFile->setValue(0);
  ui->progressBarFile->setMaximum(mFileCells.size());
  ui->progressBarFile->setVisible(true);
}

void FormDestStore::PrepareDb(bool dest)
{
  if (dest) {
    ContInfo destContInfo(ui->lineEditPath->text(), ui->spinBoxCell->value(), ui->spinBoxPage->value(), ui->spinBoxSize->value());
    mDbSaver = new DbSaver(destContInfo, ui->comboBoxDb->currentData().toString(), this);
  } else {
    mDbSaver = new DbSaver(mSourceCont.first(), ui->comboBoxDb->currentData().toString(), this);
  }
  connect(mDbSaver, &DbSaver::PercentChanged, this, &FormDestStore::OnPercentDbChanged);
  connect(mDbSaver, &DbSaver::finished, this, &FormDestStore::OnCreateDbEnded);

  ui->progressBarDb->setValue(0);
  ui->progressBarDb->setMaximum(mFileCells.size());
  ui->progressBarDb->setVisible(true);
}

void FormDestStore::StartWork()
{
  ui->pushButtonGo->setEnabled(false);
  ui->pushButtonStop->setVisible(true);
  ui->labelSuccess->setVisible(false);
  ui->labelErrorText->setVisible(false);
}

void FormDestStore::OnPercentChanged(int perc)
{
  ui->progressBarFile->setValue(perc);
}

void FormDestStore::OnTransferEnded()
{
  bool success = false;
  if (mStorageTransfer) {
    if (mStorageTransfer->Success()) {
      ui->labelSuccess->setVisible(true);
      success = true;
    } else {
      ui->labelErrorText->setText(mStorageTransfer->ErrorString());
      ui->labelErrorText->setVisible(true);
    }
    mStorageTransfer->deleteLater();
    mStorageTransfer = nullptr;
  }
  ui->progressBarFile->setVisible(false);

  if (mDbSaver && success) {
    mDbSaver->start();
  } else {
    ui->pushButtonGo->setEnabled(true);
    ui->pushButtonStop->setVisible(false);
  }
}

void FormDestStore::OnPercentDbChanged(int perc)
{
  ui->progressBarDb->setValue(perc);
}

void FormDestStore::OnCreateDbEnded()
{
  if (mDbSaver) {
    if (mDbSaver->Success()) {
      ui->labelSuccessDb->setVisible(true);
    } else {
      ui->labelErrorTextDb->setText(mDbSaver->ErrorString());
      ui->labelErrorTextDb->setVisible(true);
    }
    mDbSaver->deleteLater();
    mDbSaver = nullptr;
  }

  ui->progressBarDb->setVisible(false);
  ui->pushButtonGo->setEnabled(true);
  ui->pushButtonStop->setVisible(false);
}

void FormDestStore::on_actionOpenFile_triggered()
{
  QString path = QFileDialog::getSaveFileName(this, GetProgramName()
                                              , QString(), QString::fromUtf8("Файл архива (*.bin)"));
  if (!path.isEmpty()) {
    ui->lineEditPath->setText(path);
  }
}

void FormDestStore::on_pushButtonGo_clicked()
{
  bool writeFile = ui->checkBoxFile->isChecked();
  bool writeDb = ui->checkBoxDb->isChecked();
  if (writeFile && writeDb) {
    DoWrite2();
  } else if (writeFile) {
    DoWriteFile();
  } else if (writeDb) {
    DoWriteDb();
  } else {
    QMessageBox::warning(this, GetProgramName(), "Необходимо выбрать цель для выполнения");
  }
}

void FormDestStore::on_checkBoxFile_toggled(bool checked)
{
  ui->groupBoxFile->setEnabled(checked);
}

void FormDestStore::on_checkBoxDb_toggled(bool checked)
{
  ui->groupBoxDb->setEnabled(checked);
}

void FormDestStore::on_spinBoxSize_editingFinished()
{
  mSizeManual = true;

  LoadCells();
}

void FormDestStore::on_spinBoxSizeDb_editingFinished()
{
  mSizeDbManual = true;
}

void FormDestStore::on_pushButtonStop_clicked()
{
  if (mStorageTransfer) {
    mStorageTransfer->Stop();
  }
  ui->pushButtonStop->setVisible(false);
}


void FormDestStore::on_comboBoxDb_currentIndexChanged(int index)
{
  ui->spinBoxSizeDb->setValue(0);
  QString filename = ui->comboBoxDb->itemData(index).toString();
  Db db;
  if (db.OpenFromFile(filename) && db.Connect()) {
    DbIndex dbIndex(db);
    dbIndex.ConnectUnit(0);
    int size;
    if (dbIndex.GetLastCell(size)) {
      ui->spinBoxSizeDb->setValue(size);
      mSizeDbManual = false;
    }
  }
}
