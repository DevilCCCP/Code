#include <QTimer>
#include <QFormLayout>

#include <Lib/Common/Icon.h>
#include <Lib/Ui/DialogName.h>

#include "FormDbEditor.h"
#include "ui_FormDbEditor.h"
#include "Tree/TreeModelB.h"
#include "Tree/TreeValidator.h"
#include "TableEditSchema.h"
#include "DialogDbEditSelect.h"


const int kWarningClearTimeoutMs = 15000;
const QString kWarningMsg("<html><head/><body><p><span style=\" color:#aa0000;\">%1</span></p></body></html>");

FormDbEditor::FormDbEditor(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormDbEditor)
  , mTreeModel(new TreeModelB(this))
  , mWarningTimer(new QTimer(this))
{
  Q_INIT_RESOURCE(DbUi);

  ui->setupUi(this);

  ui->toolButtonReload->setDefaultAction(ui->actionReload);

  ui->treeViewDb->setModel(mTreeModel);
  ui->treeViewDb->addAction(ui->actionReload);
  ui->treeViewDb->setContextMenuPolicy(Qt::ActionsContextMenu);

  ui->widgetWarning->setVisible(false);
  mWarningTimer->setSingleShot(true);
  mWarningTimer->setInterval(kWarningClearTimeoutMs);

  connect(ui->treeViewDb->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FormDbEditor::OnTreeSelectionChanged);
  connect(mWarningTimer, &QTimer::timeout, this, &FormDbEditor::OnWarningTmeout);
}

FormDbEditor::~FormDbEditor()
{
  delete ui;
}


QSplitter* FormDbEditor::Splitter()
{
  return ui->splitter;
}

QTreeView* FormDbEditor::Tree()
{
  return ui->treeViewDb;
}

void FormDbEditor::SetTreeSchema(const DbTreeSchemaS& _Schema, const QStringList& _Headers)
{
  mSchema = _Schema;
  mTreeModel->SetHeaders(_Headers);
}

void FormDbEditor::Reload()
{
  mTreeModel->Clear();
  ReloadTable(mSchema->TreeChilds, QMap<qint64, TreeItemB*>());
  OnTreeSelectionChanged(QItemSelection(), QItemSelection());
  ui->treeViewDb->expandAll();
}

void FormDbEditor::ReloadTable(const QList<DbTreeSchemaS>& schemaChilds, const QMap<qint64, TreeItemB*>& parentMap)
{
  foreach (const DbTreeSchemaS& schema, schemaChilds) {
    if (schema->Icon.isNull()) {
      schema->Icon = QIcon(schema->IconName);
    }
    switch (schema->Connection) {
    case DbTreeSchema::eRoot     : LoadRootTable(schema); break;
    case DbTreeSchema::eChild    : LoadChildTable(schema, parentMap); break;
    case DbTreeSchema::eLink     : LoadLinkTable(schema, parentMap); break;
    case DbTreeSchema::eMultiLink: LoadMultiLinkTable(schema, parentMap); break;
    }
  }
}

void FormDbEditor::LoadRootTable(const DbTreeSchemaS& schema)
{
  QVector<DbItemBS> itemsList;
  schema->Table->Select(QString("ORDER BY _id"), itemsList);
  QMap<qint64, TreeItemB*> currentMap;
  foreach (const DbItemBS& item, itemsList) {
    TreeItemBS treeItem(new TreeItemB(item, schema.data()));
    mTreeModel->AppendChild(treeItem);
    currentMap[item->Id] = treeItem.data();
  }

  ReloadTable(schema->TreeChilds, currentMap);
}

void FormDbEditor::LoadChildTable(const DbTreeSchemaS& schema, const QMap<qint64, TreeItemB*>& parentMap)
{
  QVector<DbItemBS> itemsList;
  schema->Table->Select(QString("ORDER BY _id"), itemsList);
  QMap<qint64, TreeItemB*> currentMap;
  foreach (const DbItemBS& item, itemsList) {
    qint64 keyId = item->Key(schema->KeyIndex);

    for (auto itr = parentMap.find(keyId); itr != parentMap.end() && itr.key() == keyId; itr++) {
      TreeItemBS treeItem(new TreeItemB(item, schema.data()));
      itr.value()->AppendChild(treeItem);
      currentMap.insertMulti(item->Id, treeItem.data());
    }
  }

  ReloadTable(schema->TreeChilds, currentMap);
}

void FormDbEditor::LoadLinkTable(const DbTreeSchemaS& schema, const QMap<qint64, TreeItemB*>& parentMap)
{
  QMap<qint64, TreeItemB*> parentLinkMap;
  for (auto itr = parentMap.begin(); itr != parentMap.end(); itr++) {
    TreeItemB* treeItem = itr.value();
    qint64           id = treeItem->Item()->Key(schema->KeyIndex);
    parentLinkMap.insertMulti(id, treeItem);
  }

  QVector<DbItemBS> itemsList;
  schema->Table->Select(QString("ORDER BY _id"), itemsList);
  QMap<qint64, TreeItemB*> currentMap;
  foreach (const DbItemBS& item, itemsList) {
    qint64 keyId = item->Id;
    auto itr = parentLinkMap.find(keyId);
    for (; itr != parentLinkMap.end() && itr.key() == keyId; itr++) {
      TreeItemBS treeItem(new TreeItemB(item, schema.data()));
      itr.value()->AppendChild(treeItem);
      currentMap.insertMulti(item->Id, treeItem.data());
    }
  }

  ReloadTable(schema->TreeChilds, currentMap);
}

void FormDbEditor::LoadMultiLinkTable(const DbTreeSchemaS& schema, const QMap<qint64, TreeItemB*>& parentMap)
{
  QMap<qint64, DbItemBS> itemsMap;
  schema->Table->Select(QString("ORDER BY _id"), itemsMap);
  QVector<DbItemBS> itemsLinkList;
  schema->MultiLinkTable->Select(QString(""), itemsLinkList);

  QMap<qint64, TreeItemB*> currentMap;
  foreach (const DbItemBS& linkItem, itemsLinkList) {
    qint64 parentId = linkItem->Key(schema->KeyIndex);
    qint64  childId = linkItem->Key(schema->MultiLinkIndex);

    auto itrch = itemsMap.find(childId);
    if (itrch != itemsMap.end()) {
      for (auto itrp  = parentMap.find(parentId); itrp != parentMap.end() && itrp.key() == parentId; itrp++) {
        const DbItemBS& item = itrch.value();
        TreeItemBS treeItem(new TreeItemB(item, schema.data()));
        treeItem->SetMultiLinkId(linkItem->Id);
        itrp.value()->AppendChild(treeItem);
        currentMap.insertMulti(item->Id, treeItem.data());
      }
    }
  }

  ReloadTable(schema->TreeChilds, currentMap);
}

void FormDbEditor::AddCreateAction(DbTreeSchema* schema)
{
  QAction* action = new QAction(IconFromImage(schema->IconName, ":/Icons/Add.png"), QString(tr("Create %1")).arg(schema->Name), this);
  AddAction(action, schema, &FormDbEditor::OnCreateTriggered);
  switch (schema->Connection) {
  case DbTreeSchema::eRoot     :
  case DbTreeSchema::eChild    : break;
  case DbTreeSchema::eLink     :
  case DbTreeSchema::eMultiLink:
    action = new QAction(IconFromImage(schema->IconName, ":/Icons/Link.png"), QString(tr("Link %1")).arg(schema->Name), this);
    AddAction(action, schema, &FormDbEditor::OnLinkTriggered);
    break;
  }
}

void FormDbEditor::AddRemoveAction(DbTreeSchema* schema)
{
  QAction* action = new QAction(IconFromImage(schema->IconName, ":/Icons/Remove.png")
                                , QString(tr("Remove %1")).arg(schema->Name), this);
  AddAction(action, schema, &FormDbEditor::OnRemoveTriggered);
}

void FormDbEditor::AddUnlinkAction(DbTreeSchema* schema)
{
  QAction* action = new QAction(IconFromImage(schema->IconName, ":/Icons/Unlink.png")
                                , QString(tr("Unlink %1")).arg(schema->Name), this);
  AddAction(action, schema, &FormDbEditor::OnUnlinkTriggered);
}

void FormDbEditor::AddUnlinkMultiAction(DbTreeSchema* schema)
{
  QAction* action = new QAction(IconFromImage(schema->IconName, ":/Icons/Unlink.png")
                                , QString(tr("Unlink %1")).arg(schema->Name), this);
  AddAction(action, schema, &FormDbEditor::OnUnlinkMultiTriggered);
}

void FormDbEditor::AddAction(QAction* action, DbTreeSchema* schema, OnTriggeredAction onTriggered)
{
  QToolButton* button = new QToolButton(this);
  button->setDefaultAction(action);
  mCurrentActionList.append(action);
  mCurrentButtonList.append(button);
  ui->horizontalLayoutControls->insertWidget(mCurrentButtonList.size(), button);

  action->setProperty("Schema", schema->Name);
  mSchemaMap[schema->Name] = schema;
  connect(action, &QAction::triggered, this, onTriggered);
}

bool FormDbEditor::UpdateItem(DbTreeSchema* schema, DbItemBS& item)
{
  if (!item || !item->Id) {
    return false;
  }

  QVector<DbItemBS> itemList;
  if (schema->Table->Select(QString("WHERE _id=%1").arg(item->Id), itemList) && itemList.size() == 1) {
    DbItemBS loadItem = itemList.first();
    if (!item->Equals(*loadItem)) {
      item = loadItem;
      return true;
    }
  }
  return false;
}

void FormDbEditor::CreateActions()
{
  foreach (QAction* action, mCurrentActionList) {
    ui->treeViewDb->removeAction(action);
    action->deleteLater();
  }
  mCurrentActionList.clear();
  foreach (QToolButton* button, mCurrentButtonList) {
    button->deleteLater();
  }
  mCurrentButtonList.clear();

  QModelIndexList selectedIndexes = ui->treeViewDb->selectionModel()->selectedRows();
  bool hasSchema = false;
  DbTreeSchema* itemSchema = nullptr;
  mCurrentItemList.clear();
  foreach (const QModelIndex& index, selectedIndexes) {
    TreeItemB* item = (TreeItemB*)index.internalPointer();
    mCurrentItemList.append(item);
    if (hasSchema && item->Schema() != itemSchema) {
      return;
    } else {
      itemSchema = item->Schema();
      hasSchema = true;
    }
  }
  if (hasSchema) {
    if (itemSchema->Connection == DbTreeSchema::eRoot) {
      AddCreateAction(itemSchema);
    }
    foreach (const DbTreeSchemaS& schema, itemSchema->TreeChilds) {
      AddCreateAction(schema.data());
    }
    switch (itemSchema->Connection) {
    case DbTreeSchema::eRoot     : break;
    case DbTreeSchema::eChild    : break;
    case DbTreeSchema::eLink     : AddUnlinkAction(itemSchema); break;
    case DbTreeSchema::eMultiLink: AddUnlinkMultiAction(itemSchema); break;
    }
    AddRemoveAction(itemSchema);
  } else {
    foreach (const DbTreeSchemaS& schema, mSchema->TreeChilds) {
      AddCreateAction(schema.data());
    }
  }
}

void FormDbEditor::CreateRootItem(DbTreeSchema* schema)
{
  DbItemBS item;
  if (!schema->Table->Create(item)) {
    Warning(QString(tr("Create %1 fail")).arg(schema->Name));
    if (!item) {
      return;
    }
  }

  UpdateItem(schema, item);

  TreeItemBS treeItem(new TreeItemB(item, schema));
  mTreeModel->AppendChild(treeItem);
}

void FormDbEditor::CreateChildItems(DbTreeSchema* schema)
{
  foreach (TreeItemB* parentItem, mCurrentItemList) {
    if (!parentItem->Item()->Id) {
      Warning(QString(tr("Can't attach to unexisted %1")).arg(parentItem->Schema()->Name));
      continue;
    }
    DbItemBS item;
    schema->Table->Create(item);
    if (!item) {
      Warning(QString(tr("Create %1 fail")).arg(schema->Name));
      continue;
    }
    item->SetKey(schema->KeyIndex, parentItem->Item()->Id);
    if (!schema->Table->Insert(item)) {
      Warning(QString(tr("Create %1 fail")).arg(schema->Name));
    }
    TreeItemBS treeItem(new TreeItemB(item, schema));
    mTreeModel->AppendChild(parentItem, treeItem);
    ui->treeViewDb->expand(mTreeModel->IndexFromItem(parentItem));
    UpdateParents(treeItem.data());
  }
}

void FormDbEditor::CreateLinkItems(DbTreeSchema* schema)
{
  foreach (TreeItemB* parentItem, mCurrentItemList) {
    DbItemBS item;
    if (!schema->Table->Create(item)) {
      Warning(QString(tr("Create %1 fail")).arg(schema->Name));
      if (!item) {
        continue;
      }
    }
    TreeItemBS treeItem(new TreeItemB(item, schema));
    for (int i = 0; i < parentItem->ChildCount(); i++) {
      if (parentItem->Child(i)->Schema() == schema) {
        mTreeModel->RemoveChild(parentItem, i);
        break;
      }
    }
    mTreeModel->AppendChild(parentItem, treeItem);
    ui->treeViewDb->expand(mTreeModel->IndexFromItem(parentItem));
    UpdateParents(treeItem.data());
  }
}

void FormDbEditor::CreateMultiLinkItems(DbTreeSchema* schema)
{
  foreach (TreeItemB* parentItem, mCurrentItemList) {
    DbItemBS item;
    if (!schema->Table->Create(item)) {
      Warning(QString(tr("Create %1 fail")).arg(schema->Name));
      if (!item) {
        continue;
      }
    }
    TreeItemBS treeItem(new TreeItemB(item, schema));
    mTreeModel->AppendChild(parentItem, treeItem);
    UpdateParents(treeItem.data());
  }
}

void FormDbEditor::LinkLinkItems(DbTreeSchema* schema)
{
  if (mCurrentItemList.isEmpty()) {
    return;
  }

  DialogDbEditSelect dialogDbEditSelect(this);
  QVector<DbItemBS> itemsList;
  schema->Table->Select(QString("ORDER BY _id"), itemsList);
  dialogDbEditSelect.Init(itemsList, schema);
  dialogDbEditSelect.SetSingleItem();
  int result = dialogDbEditSelect.exec();
  if (result != QDialog::Accepted) {
    return;
  }
  dialogDbEditSelect.GetResult(itemsList);
  if (itemsList.isEmpty()) {
    return;
  } else if (itemsList.size() > 1) {
    Warning(tr("Can't link more than one item"));
    return;
  }
  const DbItemBS& item = itemsList.first();

  foreach (TreeItemB* parentItem, mCurrentItemList) {
    parentItem->Item()->SetKey(schema->KeyIndex, item->Id);
    if (!parentItem->Schema()->Table->Update(parentItem->Item())) {
      Warning(QString(tr("Link %1 fail")).arg(schema->Name));
    }
    TreeItemBS treeItem(new TreeItemB(item, schema));
    for (int i = 0; i < parentItem->ChildCount(); i++) {
      if (parentItem->Child(i)->Schema() == schema) {
        mTreeModel->RemoveChild(parentItem, i);
        break;
      }
    }
    mTreeModel->AppendChild(parentItem, treeItem);
    ui->treeViewDb->expand(mTreeModel->IndexFromItem(parentItem));
    UpdateParents(treeItem.data());
  }
}

void FormDbEditor::LinkMultiLinkItems(DbTreeSchema* schema)
{
  if (mCurrentItemList.isEmpty()) {
    return;
  }

  DialogDbEditSelect dialogDbEditSelect(this);
  QVector<DbItemBS> itemsList;
  schema->Table->Select(QString("ORDER BY _id"), itemsList);
  dialogDbEditSelect.Init(itemsList, schema);
  int result = dialogDbEditSelect.exec();
  if (result != QDialog::Accepted) {
    return;
  }
  dialogDbEditSelect.GetResult(itemsList);
  if (itemsList.isEmpty()) {
    return;
  }

  foreach (TreeItemB* parentItem, mCurrentItemList) {
    DbTreeSchema* parentSchema = parentItem->Schema();
    foreach (const DbItemBS& item, itemsList) {
      DbItemBS linkItem;
      schema->MultiLinkTable->New(linkItem);
      if (!linkItem) {
        Warning(QString(tr("Can't attach %1 to %2 fail")).arg(schema->Name, parentSchema->Name));
        return;
      }
      linkItem->SetKey(schema->KeyIndex, parentItem->Item()->Id);
      linkItem->SetKey(schema->MultiLinkIndex, item->Id);
      if (!schema->MultiLinkTable->Update(linkItem) || !linkItem->Id) {
        Warning(QString(tr("Attach %1 to %2 fail")).arg(schema->Name, parentSchema->Name));
        return;
      }
      TreeItemBS treeItem(new TreeItemB(item, schema));
      treeItem->SetMultiLinkId(linkItem->Id);
      mTreeModel->AppendChild(parentItem, treeItem);
      ui->treeViewDb->expand(mTreeModel->IndexFromItem(parentItem));
    }
  }
}

void FormDbEditor::UpdateParents(TreeItemB* treeItem)
{
  if (!treeItem->Item()->Id) {
    return;
  }

  DbTreeSchema* schema = treeItem->Schema();
  switch (schema->Connection) {
  case DbTreeSchema::eRoot     : break;
  case DbTreeSchema::eChild    : break;
  case DbTreeSchema::eLink     : UpdateParentLink(treeItem); break;
  case DbTreeSchema::eMultiLink: UpdateParentMultiLink(treeItem); break;
  }
}

void FormDbEditor::UpdateParentLink(TreeItemB* treeItem)
{
  DbTreeSchema* schema = treeItem->Schema();
  const DbItemBS& item = treeItem->Item();
  TreeItemB* parentItem = treeItem->ParentItem();
  DbTreeSchema* parentSchema = parentItem->Schema();
  const DbItemBS& parent = parentItem->Item();

  if (parent->Key(schema->KeyIndex) == item->Id) {
    return;
  }

  bool updateUp = parent->Key(schema->KeyIndex) == 0;
  parent->SetKey(schema->KeyIndex, item->Id);
  if (!parentSchema->Table->Update(parent)) {
    Warning(QString(tr("Update %1 fail")).arg(parentSchema->Name));
    return;
  }

  mTreeModel->UpdateItem(parentItem, QList<int>() << Qt::DecorationRole);
  if (updateUp) {
    UpdateParents(parentItem);
  }
}

void FormDbEditor::UpdateParentMultiLink(TreeItemB* treeItem)
{
  DbTreeSchema* schema = treeItem->Schema();
  const DbItemBS& item = treeItem->Item();
  TreeItemB* parentItem = treeItem->ParentItem();
  DbTreeSchema* parentSchema = parentItem->Schema();
  const DbItemBS& parent = parentItem->Item();

  DbItemBS linkItem;
  schema->MultiLinkTable->New(linkItem);
  if (!linkItem) {
    Warning(QString(tr("Can't attach %1 to %2 fail")).arg(schema->Name, parentSchema->Name));
    return;
  }
  linkItem->SetKey(schema->KeyIndex, parent->Id);
  linkItem->SetKey(schema->MultiLinkIndex, item->Id);
  if (!schema->MultiLinkTable->Update(linkItem)) {
    Warning(QString(tr("Attach %1 to %2 fail")).arg(schema->Name, parentSchema->Name));
    return;
  }

  mTreeModel->UpdateItem(treeItem, QList<int>() << Qt::DecorationRole);
}

void FormDbEditor::RemoveItems(DbTreeSchema* schema)
{
  foreach (TreeItemB* item, mCurrentItemList) {
    if (!schema->Table->Delete(item->Item()->Id)) {
      Warning(QString(tr("Remove %1 fail")).arg(schema->Name));
      return;
    }
    mTreeModel->RemoveItem(item);
  }
}

void FormDbEditor::UnlinkItems(DbTreeSchema* schema)
{
  foreach (TreeItemB* item, mCurrentItemList) {
    const DbItemBS& parentItem = item->ParentItem()->Item();
    DbTreeSchema* parentSchema = item->ParentItem()->Schema();
    parentItem->SetKey(schema->KeyIndex, 0);
    if (!parentSchema->Table->Update(parentItem)) {
      Warning(QString(tr("Unlink %1 fail")).arg(schema->Name));
      parentItem->SetKey(schema->KeyIndex, item->Item()->Id);
      return;
    }
    mTreeModel->RemoveItem(item);
  }
}

void FormDbEditor::UnlinkMultiItems(DbTreeSchema* schema)
{
  foreach (TreeItemB* item, mCurrentItemList) {
    if (!schema->MultiLinkTable->Delete(item->MultiLinkId())) {
      Warning(QString(tr("Unlink %1 fail")).arg(schema->Name));
      return;
    }
    mTreeModel->RemoveItem(item);
  }
}

void FormDbEditor::CreateEdit()
{
  mCurrentEditItem = nullptr;
  mCurrentEditSchema = nullptr;
  if (ui->widgetEdit) {
    ui->widgetEdit->deleteLater();
    ui->widgetEdit = new QWidget(ui->widgetElement);
    ui->widgetEdit->setObjectName(QStringLiteral("widgetEdit"));
    ui->verticalLayoutEdit->insertWidget(0, ui->widgetEdit);
  }
  ui->widgetEdit->setVisible(false);
  ui->widgetEditControls->setVisible(false);
  QModelIndexList indexes = ui->treeViewDb->selectionModel()->selectedIndexes();
  if (indexes.size() != 1) {
    return;
  }
  QModelIndex index = indexes.first();
  if (!index.isValid()) {
    return;
  }

  mCurrentEditItem = mTreeModel->ItemFromIndex(index);
  DbItemBS item = mCurrentEditItem->Item();
  if (UpdateItem(mCurrentEditItem->Schema(), item)) {
    mCurrentEditItem->SetItem(item);
    mTreeModel->UpdateItem(mCurrentEditItem);
  }

  QFormLayout* formLayoutEdit = new QFormLayout(ui->widgetEdit);
  formLayoutEdit->setObjectName(QStringLiteral("formLayoutEdit"));
  formLayoutEdit->setContentsMargins(0, 0, 0, 0);

  if (!mCurrentEditItem->Schema()->EditSchema) {
    return;
  }

  mCurrentEditSchema = mCurrentEditItem->Schema()->EditSchema.data();
  for (int i = 0; i < mCurrentEditSchema->Columns.size(); i++) {
    const TableEditSchema::ColumnSchema& columnSchema = mCurrentEditSchema->Columns.at(i);
    formLayoutEdit->addRow(columnSchema.Lable, columnSchema.ColumnEdit->CreateControl(ui->widgetEdit));
    QVariant v = mCurrentEditItem->Item()->Data(i);
    columnSchema.ColumnEdit->LoadValue(v);
  }
  if (!mCurrentEditSchema->Compact) {
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setVerticalStretch(1);
    ui->widgetEdit->setSizePolicy(sizePolicy);
  }

  ui->widgetEdit->setVisible(true);
  ui->widgetEditControls->setVisible(true);
}

void FormDbEditor::SaveEdit()
{
  for (int i = 0; i < mCurrentEditSchema->Columns.size(); i++) {
    const TableEditSchema::ColumnSchema& columnSchema = mCurrentEditSchema->Columns.at(i);
    QVariant v;
    columnSchema.ColumnEdit->SaveValue(v);
    mCurrentEditItem->Item()->SetData(i, v);
  }
  if (!mCurrentEditItem->Schema()->Table->Update(mCurrentEditItem->Item())) {
    Warning(QString(tr("Update %1 fail")).arg(mCurrentEditItem->Schema()->Name));
  } else {
    ValidateItem(mCurrentEditItem);
  }
  mTreeModel->UpdateItem(mCurrentEditItem);
}

void FormDbEditor::ValidateSelected()
{
  OnWarningTmeout();

  QModelIndexList indexes = ui->treeViewDb->selectionModel()->selectedIndexes();
  if (indexes.size() != 1) {
    return;
  }

  QModelIndex index = indexes.first();
  if (!index.isValid()) {
    return;
  }

  TreeItemB* selectedEditItem = mTreeModel->ItemFromIndex(index);
  if (!selectedEditItem->Item()->Id) {
    Warning(QString(tr("Current %1 is not created")).arg(selectedEditItem->Schema()->Name));
    return;
  }
  ValidateItem(selectedEditItem);
}

void FormDbEditor::ValidateItem(TreeItemB* item)
{
  if (!item->Schema()->Validator) {
    return;
  }

  QList<DbItemBS> parentItemList;
  for (TreeItemB* parent = item->ParentItem(); parent; parent = parent->ParentItem()) {
    parentItemList.append(parent->Item());
  }
  QString errorText;
  if (!item->Schema()->Validator->IsValid(item->Item(), parentItemList, errorText)) {
    Warning(errorText);
    return;
  }
}

DbTreeSchema* FormDbEditor::GetActionSchema()
{
  return mSchemaMap[sender()->property("Schema").toString()];
}

void FormDbEditor::ClearWarning()
{
  ui->widgetWarning->setVisible(false);
  mWarningTimer->stop();
}

void FormDbEditor::Warning(const QString& text)
{
  ui->labelWarning->setText(kWarningMsg.arg(text));
  ui->widgetWarning->setVisible(true);
  mWarningTimer->start();
}

void FormDbEditor::OnTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_UNUSED(selected);
  Q_UNUSED(deselected);

  CreateActions();
  CreateEdit();
  ValidateSelected();
}

void FormDbEditor::OnWarningTmeout()
{
  ui->widgetWarning->setVisible(false);
}

void FormDbEditor::OnCreateTriggered()
{
  ClearWarning();
  if (DbTreeSchema* schema = GetActionSchema()) {
    switch (schema->Connection) {
    case DbTreeSchema::eRoot     : CreateRootItem(schema); break;
    case DbTreeSchema::eChild    : CreateChildItems(schema); break;
    case DbTreeSchema::eLink     : CreateLinkItems(schema); break;
    case DbTreeSchema::eMultiLink: CreateMultiLinkItems(schema); break;
    }
  }
}

void FormDbEditor::OnLinkTriggered()
{
  ClearWarning();
  if (DbTreeSchema* schema = GetActionSchema()) {
    switch (schema->Connection) {
    case DbTreeSchema::eRoot     : Warning(QString(tr("Can't link to root item"))); break;
    case DbTreeSchema::eChild    : Warning(QString(tr("Can't link with type of item"))); break;
    case DbTreeSchema::eLink     : LinkLinkItems(schema); break;
    case DbTreeSchema::eMultiLink: LinkMultiLinkItems(schema); break;
    }
  }
}

void FormDbEditor::OnRemoveTriggered()
{
  ClearWarning();

  if (DbTreeSchema* schema = GetActionSchema()) {
    RemoveItems(schema);
  }
}

void FormDbEditor::OnUnlinkTriggered()
{
  ClearWarning();

  if (DbTreeSchema* schema = GetActionSchema()) {
    UnlinkItems(schema);
  }
}

void FormDbEditor::OnUnlinkMultiTriggered()
{
  ClearWarning();

  if (DbTreeSchema* schema = GetActionSchema()) {
    UnlinkMultiItems(schema);
  }
}

void FormDbEditor::on_actionReload_triggered()
{
  ClearWarning();

  Reload();
}

void FormDbEditor::on_pushButtonSave_clicked()
{
  ClearWarning();

  SaveEdit();
}
