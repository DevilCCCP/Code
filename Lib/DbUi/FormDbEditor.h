#pragma once

#include <QWidget>

#include "Tree/DbTreeSchema.h"


DefineClassS(TreeModelB);
DefineClassS(TreeItemB);
DefineStructS(DbTreeSchema);
class QSplitter;
class QTreeView;
class QToolButton;

namespace Ui {
class FormDbEditor;
}

class FormDbEditor: public QWidget
{
  Ui::FormDbEditor*            ui;

  DbTreeSchemaS                mSchema;
  TreeModelB*                  mTreeModel;
  QList<QAction*>              mCurrentActionList;
  QList<QToolButton*>          mCurrentButtonList;
  QList<TreeItemB*>            mCurrentItemList;
  QMap<QString, DbTreeSchema*> mSchemaMap;

  TreeItemB*                   mCurrentEditItem;
  TableEditSchema*             mCurrentEditSchema;
  QTimer*                      mWarningTimer;

  typedef void (FormDbEditor::*OnTriggeredAction)();

  Q_OBJECT

public:
  explicit FormDbEditor(QWidget* parent = 0);
  ~FormDbEditor();

public:
  QSplitter* Splitter();
  QTreeView* Tree();

public:
  void SetTreeSchema(const DbTreeSchemaS& _Schema, const QStringList& _Headers);
  void Reload();

private:
  void ReloadTable(const QList<DbTreeSchemaS>& schemaChilds, const QMultiMap<qint64, TreeItemB*>& parentMap);
  void LoadRootTable(const DbTreeSchemaS& schema);
  void LoadChildTable(const DbTreeSchemaS& schema, const QMultiMap<qint64, TreeItemB*>& parentMap);
  void LoadLinkTable(const DbTreeSchemaS& schema, const QMultiMap<qint64, TreeItemB*>& parentMap);
  void LoadMultiLinkTable(const DbTreeSchemaS& schema, const QMultiMap<qint64, TreeItemB*>& parentMap);

  void AddCreateAction(DbTreeSchema* schema);
  void AddRemoveAction(DbTreeSchema* schema);
  void AddUnlinkAction(DbTreeSchema* schema);
  void AddUnlinkMultiAction(DbTreeSchema* schema);
  void AddAction(QAction* action, DbTreeSchema* schema, OnTriggeredAction onTriggered);

  bool UpdateItem(DbTreeSchema* schema, DbItemBS& item);
  void CreateActions();
  void CreateRootItem(DbTreeSchema* schema);
  void CreateChildItems(DbTreeSchema* schema);
  void CreateLinkItems(DbTreeSchema* schema);
  void CreateMultiLinkItems(DbTreeSchema* schema);
  void LinkLinkItems(DbTreeSchema* schema);
  void LinkMultiLinkItems(DbTreeSchema* schema);
  void UpdateParents(TreeItemB* treeItem);
  void UpdateParentLink(TreeItemB* treeItem);
  void UpdateParentMultiLink(TreeItemB* treeItem);
  void RemoveItems(DbTreeSchema* schema);
  void UnlinkItems(DbTreeSchema* schema);
  void UnlinkMultiItems(DbTreeSchema* schema);

  void CreateEdit();
  void SaveEdit();
  void ValidateSelected();
  void ValidateItem(TreeItemB* item);

  DbTreeSchema* GetActionSchema();

  void ClearWarning();
  void Warning(const QString& text);

private:
  void OnTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void OnWarningTmeout();
  void OnCreateTriggered();
  void OnLinkTriggered();
  void OnRemoveTriggered();
  void OnUnlinkTriggered();
  void OnUnlinkMultiTriggered();

private slots:
  void on_actionReload_triggered();
  void on_pushButtonSave_clicked();
};
