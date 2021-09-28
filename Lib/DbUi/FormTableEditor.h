#pragma once

#include <QWidget>

#include <Lib/Db/DbTable.h>

#include "TableEditSchema.h"


DefineClassS(TableModel);
DefineStructS(TableEditSchema);
class QSplitter;
class QTreeView;
class QToolButton;

namespace Ui {
class FormTableEditor;
}

class FormTableEditor: public QWidget
{
  Ui::FormTableEditor*         ui;

  TableEditSchemaS             mSchema;
  DbTableBS                    mTable;
  TableModel*                  mTableModel;

  DbItemBS                     mCurrentEditItem;
  QTimer*                      mWarningTimer;

  Q_OBJECT

public:
  explicit FormTableEditor(QWidget* parent = 0);
  ~FormTableEditor();

public:
  QSplitter* Splitter();
  QTreeView* Tree();
  const DbItemBS& CurrentEditItem() { return mCurrentEditItem; }

public:
  void InitTable(const TableEditSchemaS& _Schema, const DbTableBS& _Table);
  void Reload();

private:
  void CreateEdit();
  void SaveEdit();
  void UpdateActions();

  void ClearWarning();
  void Warning(const QString& text);

private:
  void OnTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void OnWarningTimeout();

signals:
  void SelectionChanged();
  void ItemActivated();

private slots:
  void on_actionReload_triggered();
  void on_pushButtonSave_clicked();
  void on_actionCreate_triggered();
  void on_actionEdit_triggered();
  void on_actionClone_triggered();
  void on_actionRemove_triggered();
  void on_treeViewTable_activated(const QModelIndex& index);
};
