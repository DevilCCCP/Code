#pragma once

#include <QWidget>
#include <QList>

#include <Lib/Db/DbTable.h>

#include "FormTableAdapter.h"
#include "DbTableModel.h"


namespace Ui {
class FormTable;
}

DefineClassS(FormTableAdapterA);
DefineClassS(CsvWriter);
DefineClassS(CsvReader);

class QVBoxLayout;
class QHBoxLayout;
class QComboBox;
class QToolButton;
class QStandardItemModel;

class FormTable: public QWidget
{
  Ui::FormTable*       ui;

  struct CondCtrl {
    QHBoxLayout*   HBoxLayout;
    QComboBox*     ComboBoxType;
    QWidget*       EditWidget;
    QToolButton*   ToolButtonRemove;

    QCompleter*    Completer;
    qint64         ItemId;
    bool           IsMain;

    void Init(QWidget* parent, QVBoxLayout* insertLayout, int insertIndex);
    void ReplaceEdit(QWidget* edit);
    void RemoveEdit();
    void Release();
  };

  QSortFilterProxyModel* mProxyModel;
  FormTableAdapterAS   mTableAdapter;

  QList<CondCtrl>      mConditions;
  QStringList          mStaticConditions;
  QString              mWhere;
  QString              mQuery;
  bool                 mMainCondIsCustom;

  Q_OBJECT

public:
  explicit FormTable(QWidget *parent = 0);
  ~FormTable();

public:
  void Init(const FormTableAdapterAS& _TableAdapter, bool load = true);
  void SetEnableClone(bool enabled);
  void SetReadOnly();
  void SetViewMode();
  void SetLimit(int limit);
  int CurrentItem();

  void AddToControls(QWidget* widget);
  void AddAction(QAction* action);
  void Reload();

  void ClearStaticConditions();
  void AddStaticCondition(const QString& condition);

  void ClearSeek();
  void AddSeek(const QString& column, const QString& text);

  const QString& GetQuery() const { return mQuery; }

private:
  void InitFilterOne(const CondCtrl& condCtrl, bool main);
  void AddNewFilter();
  void InitMenu();
  void InitAction(QToolButton* button, QAction* action, const QString& format);
  void InitFilter(TableSchema::TableFilter::EEqualType type, CondCtrl* selCond);
  void ChangeCondition(CondCtrl* selCond, int index);

  void UpdateCurrent(bool hasCurrent);
  void UpdateSelection(bool hasSelection);

  void ApplyFilters();

  void Db2Csv(bool backup);
  void Csv2Db(bool backup);

signals:
  void OnChangeSelection(bool hasSelection);
  void OnChangeCurrent(bool hasCurrent);
  void OnNewItem();
  void OnEditItem(int index);
  void OnRemoveItems();

private:
  void OnActivated(const QModelIndex& index);
  void OnCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
  void OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void OnComboBoxTypeCurrentIndexChanged(int index);
  void OnLineEditTextEdited(const QString& text);
  void OnLineEditTextChanged(const QString& text);
  //void OnCompleterActivated(const QModelIndex& index);
  void OnLineEditReturnPressed();
  void OnButtonRemove_clicked();

private slots:
  void on_actionNew_triggered();
  void on_actionAddFilter_triggered();
  void on_spinBoxLimit_valueChanged(int value);
  void on_actionLimit_triggered(bool checked);
  void on_actionEdit_triggered();
  void on_actionRemove_triggered();
  void on_actionClone_triggered();
  void on_actionExport_triggered();
  void on_actionImport_triggered();
  void on_actionReload_triggered();
  void on_actionBackup_triggered();
  void on_actionRestore_triggered();
};

