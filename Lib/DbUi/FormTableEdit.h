#pragma once

#include <QWidget>
#include <QList>

#include <Lib/Db/DbTable.h>

#include "FormTableEditAdapter.h"
#include "DbTableModel.h"


namespace Ui {
class FormTableEdit;
}

DefineClassS(FormTableEditAdapterA);
DefineClassS(CsvWriter);
DefineClassS(CsvReader);

class QVBoxLayout;
class QHBoxLayout;
class QComboBox;
class QLineEdit;
class QToolButton;
class QStandardItemModel;

class FormTableEdit: public QWidget
{
  Ui::FormTableEdit*     ui;

  QSortFilterProxyModel* mProxyModel;
  FormTableEditAdapterAS mTableAdapter;
  bool                   mIsNew;

  Q_OBJECT

public:
  explicit FormTableEdit(QWidget* parent = 0);
  ~FormTableEdit();

public:
  void Init(const FormTableEditAdapterAS& _TableAdapter);
  void SetEnableClone(bool enabled);

  void AddToControls(QWidget* widget);
  void AddAction(QAction* action);
  void Reload();

private:
  void InitMenu();
  void InitAction(QToolButton* button, QAction* action, const QString& format);

  void UpdateCurrent(bool hasCurrent);
  void UpdateSelection(bool hasSelection);

  void Db2Csv();
  void Csv2Db();

  int  CurrentItem();
  void ChangeSelection(bool hasSelection);
  void ChangeCurrent(bool hasCurrent);
  void NewItem();
  void EditItem(int index);

private:
  void OnActivated(const QModelIndex& index);
  void OnCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
  void OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

signals:
  void Edit(bool isNew);

private slots:
  void on_actionNew_triggered();
  void on_actionEdit_triggered();
  void on_actionRemove_triggered();
  void on_actionClone_triggered();
  void on_actionExport_triggered();
  void on_actionImport_triggered();
  void on_actionReload_triggered();
  void on_pushButtonSave_clicked();
};

