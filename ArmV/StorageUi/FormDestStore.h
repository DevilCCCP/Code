#pragma once

#include <QWidget>
#include <QFile>
#include <QStandardItemModel>

#include <Lib/Include/Common.h>

#include "Info.h"


DefineClassS(StorageTransfer);
DefineClassS(DbSaver);

namespace Ui {
class FormDestStore;
}

class FormDestStore: public QWidget
{
  Ui::FormDestStore*   ui;
  QIcon                mCellIcon;
  QIcon                mDbIcon;

  QVector<ContInfo>    mSourceCont;
  QVector<CellInfoEx>  mSourceCells;
  QVector<CellInfoEx>  mFileCells;
  QVector<QString>     mSourceNames;
  QStandardItemModel*  mCellModel;
  StorageTransfer*     mStorageTransfer;
  DbSaver*             mDbSaver;

  bool                 mSizeManual;
  bool                 mSizeDbManual;

  Q_OBJECT

public:
  explicit FormDestStore(QWidget* parent = 0);
  ~FormDestStore();

protected:
  /*override */virtual void closeEvent(QCloseEvent* event) override;

public:
  void SetCellSize(int size);
  void SetPageSize(int size);
  void SetSize(int size);
  void SetSources(const QVector<ContInfo>& conts, const QVector<CellInfoEx>& cells, const QVector<QString>& names);

private:
  void LoadCells();

  bool DoWrite2();
  bool DoWriteFile();
  bool DoWriteDb();
  bool ValidateFile();
  bool ValidateDb();

  void PrepareTransfer();
  void PrepareDb(bool dest);
  void StartWork();

private:
  void OnPercentChanged(int perc);
  void OnTransferEnded();
  void OnPercentDbChanged(int perc);
  void OnCreateDbEnded();

private slots:
  void on_actionOpenFile_triggered();
  void on_pushButtonGo_clicked();
  void on_checkBoxFile_toggled(bool checked);
  void on_checkBoxDb_toggled(bool checked);
  void on_spinBoxSize_editingFinished();
  void on_spinBoxSizeDb_editingFinished();
  void on_pushButtonStop_clicked();
  void on_comboBoxDb_currentIndexChanged(int index);
};
