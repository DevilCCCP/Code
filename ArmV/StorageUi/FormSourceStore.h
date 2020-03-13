#pragma once

#include <QWidget>
#include <QStandardItemModel>

#include <Lib/Include/Common.h>

#include "Info.h"


namespace Ui {
class FormSourceStore;
}

DefineClassS(StorageScaner);
class QItemSelection;

class FormSourceStore: public QWidget
{
  Ui::FormSourceStore* ui;
  QIcon                mCellIcon;

  StorageScaner*       mStorageScaner;
  QStandardItemModel*  mCellModel;
  QStandardItemModel*  mObjectModel;

  bool                 mAutoChange;
  bool                 mDeselectSkip;

  Q_OBJECT

public:
  explicit FormSourceStore(QWidget* parent = 0);
  ~FormSourceStore();

protected:
  /*override */virtual void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;

public:
  int GetCellSize() const;
  int GetPageSize() const;
  int GetSize() const;
  QString Filename() const;
  ContInfo GetContainerInfo() const;

  int CellCount() const;
  void AddCells(int id, QVector<CellInfoEx>* cells);

private:
  void OnPercentChanged(int perc);
  void OnScanEnded();

  void OnItemChanged(QStandardItem* item);
  void OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  void OnObjectChanged(QStandardItem* item);

private slots:
  void on_actionOpenFile_triggered();
  void on_actionScan_triggered();
  void on_lineEditPath_textChanged(const QString& path);
  void on_labelAll_linkActivated(const QString&);
  void on_labelNone_linkActivated(const QString&);
  void on_labelObjectAdd_linkActivated(const QString&);
  void on_labelObjectRemove_linkActivated(const QString&);
};
