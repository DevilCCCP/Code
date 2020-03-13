#pragma once

#include <QWidget>
#include <QFutureWatcher>

#include <Lib/Db/Db.h>


namespace Ui {
class FormStatistics;
}

class StatisticsLoader;
class QStandardItemModel;
class QFileDialog;

class FormStatistics: public QWidget
{
  Ui::FormStatistics*    ui;
  Db*                    mDb;
  ObjectTableS           mObjectTable;
  ObjectTypeTableS       mObjectTypeTable;
  ObjectStateS           mObjectStateTable;
  ObjectStateHoursTableS mObjectStateHoursTable;

  QFutureWatcher<void>*  mStatisticsWatcher;
  StatisticsLoader*      mStatisticsLoader;
  QStandardItemModel*    mResultsModel;
  QStringList            mResultsHeaders;
  QFileDialog*           mFileDialog;
  bool                   mDone;

  Q_OBJECT

public:
  explicit FormStatistics(QWidget* parent = 0);
  ~FormStatistics();

public:
  void Init(Db* _Db);

private:
  void UpdateState(bool running);
  void Reload();
  void LoadStatistics();
  bool ExportFile(QFile* file);

private:
  void OnProgress(qint64 count, qint64 countMax);
  void OnFinished();

private slots:
  void on_pushButtonStart_clicked();
  void on_pushButtonStop_clicked();
  void on_actionCsv_triggered();
  void on_pushButtonFilter_clicked();
};
