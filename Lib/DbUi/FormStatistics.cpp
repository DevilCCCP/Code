#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QFileDialog>

#include <Lib/Common/Format.h>
#include <Lib/Common/CsvWriter.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectState.h>
#include <Lib/Db/ObjectStateHours.h>

#include "FormStatistics.h"
#include "ui_FormStatistics.h"
#include "StatisticsLoader.h"


const QStringList kHeaders = QStringList() << "Название" << "Описание" << "Тип";

FormStatistics::FormStatistics(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormStatistics), mDb(nullptr)
  , mStatisticsWatcher(new QFutureWatcher<void>(this)), mStatisticsLoader(new StatisticsLoader(this))
  , mResultsModel(new QStandardItemModel(this)), mFileDialog(new QFileDialog(this)), mDone(false)
{
  ui->setupUi(this);

  ui->formSectionInfo->SetWidget(ui->widgetInfo, "Информация");
  ui->formSectionLoad->SetWidget(ui->widgetLoad, "Загрузка статистики");
  ui->formSectionFilters->SetWidget(ui->widgetFilters, "Фильтр статистики");

  ui->toolButtonCsv->setDefaultAction(ui->actionCsv);
  ui->comboBoxGroupBy->addItem(QString("Дни"), QVariant(1));
  ui->comboBoxGroupBy->addItem(QString("Недели"), QVariant(7));
  ui->comboBoxGroupBy->addItem(QString("Месяцы"), QVariant(30));

  mResultsModel->setHorizontalHeaderLabels(kHeaders);
  ui->tableViewStatistics->setModel(mResultsModel);

  connect(mStatisticsWatcher, &QFutureWatcher<void>::finished, this, &FormStatistics::OnFinished);
  connect(mStatisticsLoader, &StatisticsLoader::CountCanged, this, &FormStatistics::OnProgress);

  mFileDialog->setDirectory(qApp->applicationDirPath());
  mFileDialog->setNameFilters(QStringList() << "Csv файлы (*.csv *.xls)" << "Все файлы (*.*)");
  mFileDialog->setAcceptMode(QFileDialog::AcceptSave);
  mFileDialog->setFileMode(QFileDialog::AnyFile);

  QDate from = QDate::currentDate();
  from.setDate(from.year(), from.month(), 1);
  QDate to = from.addMonths(1);
  ui->dateTimeEditFrom->setDate(from);
  ui->dateTimeEditTo->setDate(to);

  UpdateState(false);
}

FormStatistics::~FormStatistics()
{
  delete ui;
}


void FormStatistics::Init(Db* _Db)
{
  mDb = _Db;

  Reload();
}

void FormStatistics::UpdateState(bool running)
{
  ui->widgetStart->setVisible(!running);
  ui->widgetRunning->setVisible(running);
  ui->pushButtonFilter->setEnabled(mDone);
}

void FormStatistics::Reload()
{
  if (!mObjectTable) {
    mObjectTable.reset(new ObjectTable(*mDb));
  }
  if (!mObjectTypeTable) {
    mObjectTypeTable.reset(new ObjectTypeTable(*mDb));
  }
  if (!mObjectStateTable) {
    mObjectStateTable.reset(new ObjectState(*mDb));
  }
  if (!mObjectStateHoursTable) {
    mObjectStateHoursTable.reset(new ObjectStateHoursTable(*mDb));
  }

  qint64 count = 0;
  mObjectTable->LoadCount(count);
  ui->spinBoxObjects->setMaximum(count);
  ui->spinBoxObjects->setValue(count);

  count = 0;
  mObjectStateTable->LoadCount(count);
  ui->spinBoxEvents->setMaximum(count);
  ui->spinBoxEvents->setValue(count);

  count = 0;
  mObjectStateTable->LoadLogCount(count);
  ui->spinBoxLogs->setMaximum(count);
  ui->spinBoxLogs->setValue(count);

  count = 0;
  bool hasTable = false;
  if (mObjectStateHoursTable->TestTable(hasTable)) {
    mObjectStateHoursTable->GetCount(count);
  }
  ui->spinBoxStats->setMaximum(count);
  ui->spinBoxStats->setValue(count);
  ui->checkBoxUseStats->setEnabled(hasTable);
  ui->checkBoxUseStats->setChecked(hasTable);
}

void FormStatistics::LoadStatistics()
{
  if (!mDone) {
    return;
  }
  QString filterName  = ui->lineEditFilterName->text();
  QString filterDescr = ui->lineEditFilterDescr->text();
  QString filterType  = ui->lineEditFilterType->text();

  mObjectTypeTable->Reload();
  mObjectTable->Reload();
  mResultsModel->clear();

  const QMap<int, StatisticsMap>& resultsMap = mStatisticsLoader->ResultsMap();
  QMap<QDate, int> daysIndex;
  mResultsHeaders = kHeaders;
  for (auto itr = resultsMap.begin(); itr != resultsMap.end(); itr++) {
    const StatisticsMap& statisticsMap = itr.value();
    for (auto itr = statisticsMap.begin(); itr != statisticsMap.end(); itr++) {
      const QDate& date = itr.key();
      daysIndex[date] = 0;
    }
  }

  int index = mResultsHeaders.size();
  for (auto itr = daysIndex.begin(); itr != daysIndex.end(); itr++) {
    const QDate& date = itr.key();
    mResultsHeaders.append(date.toString());
    itr.value() = index;
    index++;
  }

  mResultsModel->setHorizontalHeaderLabels(mResultsHeaders);

  for (auto itr = resultsMap.begin(); itr != resultsMap.end(); itr++) {
    int objectId = itr.key();
    TableItemS item = mObjectTable->GetItem(objectId);
    if (!item) {
      continue;
    }
    const ObjectItem* objItem = static_cast<const ObjectItem*>(item.data());
    if (!objItem->Name.contains(filterName, Qt::CaseInsensitive) || !objItem->Descr.contains(filterDescr, Qt::CaseInsensitive)) {
      continue;
    }
    TableItemS itemType = mObjectTypeTable->GetItem(objItem->Type);
    if (!itemType) {
      continue;
    }
    const ObjectTypeItem* objTypeItem = static_cast<const ObjectTypeItem*>(itemType.data());
    if (!objTypeItem->Descr.contains(filterType, Qt::CaseInsensitive)) {
      continue;
    }

    QList<QStandardItem*> logItems;
    logItems << new QStandardItem(QIcon(QString(":/ObjTree/%1").arg(objTypeItem->Name)), objItem->Name);
    logItems << new QStandardItem(objItem->Descr);
    logItems << new QStandardItem(objTypeItem->Descr);
    mResultsModel->appendRow(logItems);
    int rowIndex = mResultsModel->rowCount() - 1;

    const StatisticsMap& statisticsMap = itr.value();
    for (auto itr = statisticsMap.begin(); itr != statisticsMap.end(); itr++) {
      const QDate&       date = itr.key();
      const Statistics& stats = itr.value();
      qint64 total = stats.Green + stats.Red;

      auto itr_ = daysIndex.find(date);
      if (itr_ != daysIndex.end()) {
        int columnIndex = itr_.value();
        QModelIndex index = mResultsModel->index(rowIndex, columnIndex);
        if (total > stats.Gray) {
          int green = 255 * stats.Green / total;
          int red = 255 - green;
          mResultsModel->setData(index, QColor(red, green, 127), Qt::BackgroundColorRole);
          mResultsModel->setData(index, QString("%1%").arg(100.0 * stats.Green / total, 0, 'f', 2));
        } else {
          mResultsModel->setData(index, QString("Недоступен"));
        }
      } else {
        Log.Warning(QString("Statistics fail at %1").arg(date.toString()));
      }
    }
  }
}

bool FormStatistics::ExportFile(QFile* file)
{
  CsvWriter writer(file);
  writer.WriteLine(mResultsHeaders);
  for (int i = 0; i < mResultsModel->rowCount(); i++) {
    for (int j = 0; j < mResultsModel->columnCount(); j++) {
      if (!writer.WriteValue(mResultsModel->index(i, j).data().toString().toUtf8())) {
        return false;
      }
    }
    if (!writer.WriteEndLine()) {
      return false;
    }
  }
  return true;
}

void FormStatistics::OnProgress(qint64 count, qint64 countMax)
{
  ui->progressBarLoaded->setMaximum(countMax);
  ui->progressBarLoaded->setValue(count);
}

void FormStatistics::OnFinished()
{
  mDone = true;
  UpdateState(false);
  LoadStatistics();
}

void FormStatistics::on_pushButtonStart_clicked()
{
  mDone = false;
  if (mStatisticsWatcher->isRunning()) {
    mStatisticsLoader->Stop();
    mStatisticsWatcher->waitForFinished();
  }

  int days = ui->comboBoxGroupBy->currentData().toInt();
  QDateTime from = ui->dateTimeEditFrom->dateTime();
  QDateTime to = ui->dateTimeEditTo->dateTime();
  bool isFast = ui->checkBoxUseStats->isChecked();
  QFuture<void> future = QtConcurrent::run(mStatisticsLoader, isFast? &StatisticsLoader::LoadFast: &StatisticsLoader::Load, days, from, to);
  mStatisticsWatcher->setFuture(future);

  ui->progressBarLoaded->setValue(0);
  ui->progressBarLoaded->setMaximum(isFast? ui->spinBoxStats->value(): ui->spinBoxLogs->value());
  UpdateState(true);
}

void FormStatistics::on_pushButtonStop_clicked()
{
  mStatisticsLoader->Stop();
}

void FormStatistics::on_actionCsv_triggered()
{
  if (mFileDialog->exec() && !mFileDialog->selectedFiles().isEmpty()) {
    QString filename = mFileDialog->selectedFiles().first();
    QFile file(filename);
    if (!file.open(QFile::WriteOnly)) {
      QMessageBox::warning(this, this->windowTitle(), QString("Ошибка открытия файла (%1)").arg(file.errorString()));
      return;
    }
    if (!ExportFile(&file)) {
      QMessageBox::warning(this, this->windowTitle(), QString("Ошибка записи в файл (%1)").arg(file.errorString()));
      return;
    }
  }
}

void FormStatistics::on_pushButtonFilter_clicked()
{
  LoadStatistics();
}
