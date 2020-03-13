#include <QMessageBox>
#include <QMenu>
#include <QDesktopWidget>
#include <QStandardItemModel>

#include <Lib/Db/ObjectType.h>
#include <Lib/Ui/DockWidget2.h>
#include <Lib/Ui/UpWaiter.h>
#include <Lib/Common/Var.h>

#include "AdminWindow.h"
#include "ui_AdminWindow.h"
#include "ObjectModel.h"
#include "Admin/AdminWidget.h"
#include "Admin/ToolForm.h"
#include "Admin/PropertyForm.h"
#include "Admin/ToolSchema.h"
#include "Tree/TreeSchema.h"


// Defined in QtAppGui.h
const QString GetProgramName();

const QString kConnectChangeFormat("<html><head/><body><p><a href=\"here\"><span style=\"text-decoration: underline; color:#0000ff;\">%1:%2</span></a></p></body></html>");

enum ETabs {
  eTree   = 0,
  eScript = 1,
  eAbout  = 2
};

AdminWindow::AdminWindow(Db& _Db, UpInfo* _UpInfo, const CtrlManagerS& _Manager, QWidget *parent)
  : MainWindow2(parent)
  , ui(new Ui::AdminWindow)
  , mDb(_Db), mManager(_Manager), mUpWaiter(new UpWaiter(_UpInfo, this))
  , mObjectTypeTable(new ObjectTypeTable(_Db)), mObjectTable(new ObjectTable(_Db))
  , mObjectModel(nullptr), mPropertyForm(nullptr), mToolForm(nullptr), mConnectionsModel(new QStandardItemModel(this)), mInit(false)
  , mSelectedId(0), mSelectedParentId(0), mSelectedNextId(0)
  , mCurrentWidget(nullptr), mCurrentObjWidget(nullptr)
{
  Q_INIT_RESOURCE(DbUi);

  ui->setupUi(this);

  ui->splitterMain->setSizes(QList<int>() << 1 << 500);
  QPalette p = ui->splitterMain->palette();
  p.setColor(QPalette::Background, p.color(QPalette::Background).darker(120));
  ui->splitterMain->setPalette(p);

  if (!Restore()) {
    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
  }
  if (!RegisterSaveSplitter(ui->splitterMain)) {
    ui->splitterMain->setSizes(QList<int>() << 300 << 500);
  }
  RegisterSaveWidgetA(ui->dockWidgetTools, &DockWidget2::OnWindowChanged);
  RegisterSaveWidgetA(ui->dockWidgetProperties, &DockWidget2::OnWindowChanged);

  InitConnections();
}

AdminWindow::~AdminWindow()
{
  delete ui;
}

QVBoxLayout* AdminWindow::SystemObjectLayout() const
{
  return ui->verticalLayoutSystemItem;
}

void AdminWindow::showEvent(QShowEvent* event)
{
  QMainWindow::showEvent(event);

  if (!mInit) {
    Init();

    connect(this, SIGNAL(ReloadTree()), this, SLOT(CreateTree()), Qt::QueuedConnection);
    connect(ui->formScript, SIGNAL(OnScriptDone()), this, SLOT(ScriptDone()), Qt::QueuedConnection);

    mInit = true;
  }
}

void AdminWindow::GetPropertiesSchema(QStringList& schema) const
{
  schema << QString::fromUtf8("Свойство") << QString::fromUtf8("Значение");
}

QWidget* AdminWindow::GetSelectObjectWidget(const ObjectItemS& object)
{
  Q_UNUSED(object);

  return nullptr;
}

QWidget* AdminWindow::GetAboutWidget()
{
  return nullptr;
}

void AdminWindow::InitExtras()
{
}

void AdminWindow::OnObjectTypeUpdated(const ObjectTypeTableS& objectTypeTable)
{
  Q_UNUSED(objectTypeTable);
}

void AdminWindow::OnObjectsUpdated(const ObjectTableS& objectTable)
{
  Q_UNUSED(objectTable);
}

void AdminWindow::AddTabWidget(int index, AdminWidget* widget, const QIcon& icon, const QString& label)
{
  mExtraWidgets.append(widget);
  ui->tabWidgetMain->insertTab(index, widget, icon, label);
}

bool AdminWindow::GetObjectTypeId(const char* abbr, int& id)
{
  if (const NamedItem* item = mObjectTypeTable->GetItemByName(abbr)) {
    id = item->Id;
    return true;
  } else {
    id = 0;
    return false;
  }
}

void AdminWindow::ScriptDone()
{
  mToolForm->Freeze(true);
  CreateTree();
  mToolForm->Freeze(false);
}

void AdminWindow::CreateTree()
{
  if (mObjectModel->GetObjectTypeTable()->Reload()) {
    mObjectModel->UpdateSchema();
    mToolForm->UpdateSchema(mObjectModel->GetObjectTypeTable());

    OnObjectTypeUpdated(mObjectModel->GetObjectTypeTable());
  }
  QModelIndex currentIndex;
  mObjectModel->ReloadTree(mSelectedId, mSelectedParentId, mSelectedNextId, currentIndex, ui->treeViewSystemTree);
//  ui->treeViewSystemTree->expandToDepth(0);
  OnObjectsUpdated(mObjectModel->GetObjectTable());

  QItemSelectionModel* sm = ui->treeViewSystemTree->selectionModel();
  if (currentIndex.isValid()) {
    sm->setCurrentIndex(currentIndex, QItemSelectionModel::SelectCurrent);
  } else {
    OnSysTreeCurrentChanged(currentIndex, QModelIndex());
  }
}

void AdminWindow::Init()
{
  InitTools();
  InitProperties();
  InitTree();
  InitScript();
  InitAbout();
  InitExtras();

  ui->tabWidgetMain->setCurrentIndex(eTree);

  connect(mPropertyForm, &PropertyForm::OnItemNameEdited, this, &AdminWindow::OnPropertyNameEdited);

  CreateTree();
}

void AdminWindow::InitTree()
{
  TreeSchemaS schema(new TreeSchema());
  GetTreeSchema(*schema);
  mObjectModel = new ObjectModel(mDb, schema, this);
  ui->treeViewSystemTree->setModel(mObjectModel);
  mSelectedId = mSelectedParentId = mSelectedNextId = 0;

  QItemSelectionModel* sm = ui->treeViewSystemTree->selectionModel();
  connect(sm, &QItemSelectionModel::currentChanged, this, &AdminWindow::OnSysTreeCurrentChanged);
}

void AdminWindow::InitScript()
{
  ui->formScript->Init(mObjectModel, mToolForm, mPropertyForm);
}

void AdminWindow::InitAbout()
{
  if (QWidget* aboutWidget = GetAboutWidget()) {
    ui->verticalLayoutAbout->addWidget(aboutWidget);
  } else {
    ui->tabWidgetMain->removeTab(eAbout);
  }
}

void AdminWindow::InitConnections()
{
  ui->listViewConnections->setModel(mConnectionsModel);

  mConnectionsModel->clear();

  QDir varDir = GetVarDir();
  auto files = varDir.entryInfoList();
  for (auto itr = files.begin(); itr != files.end(); itr++) {
    const QFileInfo& fileInfo = *itr;
    if (fileInfo.isFile() && fileInfo.baseName().startsWith("connection") && fileInfo.suffix() == "ini") {
      QFile file(fileInfo.absoluteFilePath());
      if (!file.open(QIODevice::ReadOnly)) {
        continue;
      }

      QTextStream in(&file);
      in.setCodec("UTF-8");
      QString text;
      do {
        text = in.readLine().trimmed();
      } while (text.startsWith('#'));
      file.close();

      QStringList info = text.split("::");
      if (info.count() >= 5) {
        if (fileInfo.fileName() != "connection.ini") {
          QStandardItem* item = new QStandardItem(QString("%1:%2").arg(info[0]).arg(info[1]));
          item->setData(fileInfo.completeBaseName());
          mConnectionsModel->appendRow(item);
        } else {
          ui->labelConnectionChange->setText(kConnectChangeFormat.arg(info[0]).arg(info[1]));
        }
      }
    }
  }

  ui->listViewConnections->setVisible(false);
}

void AdminWindow::InitTools()
{
  ToolSchemaS schema(new ToolSchema());
  GetToolSchema(*schema);
  mToolForm = new ToolForm(this, mDb, schema, this);
  ui->verticalLayoutTools->addWidget(mToolForm);
}

void AdminWindow::InitProperties()
{
  mPropertyForm = new PropertyForm(mDb, this);
  ui->verticalLayoutProperties->addWidget(mPropertyForm);

  QStringList schema;
  GetPropertiesSchema(schema);
  mPropertyForm->setColumnsNames(schema);
}

void AdminWindow::OnSysTreeCurrentChanged(const QModelIndex& selected, const QModelIndex& deselected)
{
  Q_UNUSED(deselected);

  if (selected.isValid()) {
    mObjectModel->GetItemId(selected, mSelectedId);
    mObjectModel->GetItemId(selected.parent(), mSelectedParentId);
    mObjectModel->GetItemId(selected.sibling(selected.row() + 1, selected.column()), mSelectedNextId);

    if (!mObjectModel->GetItem(selected, mSelectedItem)) {
      mSelectedItem.clear();
    }
  } else {
    mSelectedItem.clear();
    mSelectedId = mSelectedParentId = mSelectedNextId = 0;
  }

  mToolForm->SetObject(mObjectModel->GetObjectTable(), mSelectedItem);
  mPropertyForm->SetObject(mSelectedItem);

  if (mCurrentObjWidget) {
    mCurrentObjWidget->close();
    mCurrentObjWidget->deleteLater();
  }

  mCurrentObjWidget = GetSelectObjectWidget(mSelectedItem);
  if (mCurrentObjWidget) {
    SystemObjectLayout()->addWidget(mCurrentObjWidget);
  }
}

void AdminWindow::OnPropertyNameEdited()
{
//  emit ReloadTree();
  ui->treeViewSystemTree->update(ui->treeViewSystemTree->currentIndex());
}

void AdminWindow::on_tabWidgetMain_currentChanged(int index)
{
  Q_UNUSED(index);
  if (AdminWidget* aw = qobject_cast<AdminWidget*>(mCurrentWidget)) {
    aw->Deactivate();
  }

  mCurrentWidget = ui->tabWidgetMain->currentWidget();
  if (mCurrentWidget == ui->tabSystem) {
    mPropertyForm->SetObject(mSelectedItem);
    mToolForm->SetObject(mObjectModel->GetObjectTable(), mSelectedItem);
  } else {
    mPropertyForm->Clear();
    mToolForm->Clear();
  }

  if (AdminWidget* aw = qobject_cast<AdminWidget*>(mCurrentWidget)) {
    aw->Activate();
  } else if (mCurrentWidget == ui->tabScript) {
    ui->formScript->Activated();
  }
}

void AdminWindow::on_listViewConnections_activated(const QModelIndex& index)
{
  if (!mDb.OpenFromFile(index.data(Qt::UserRole + 1).toString())) {
    QMessageBox::warning(this, GetProgramName(), "Подключение не удалось");
    return;
  }
  mDb.Close();
  mDb.Connect();
  mToolForm->deleteLater();
  mPropertyForm->deleteLater();
  mObjectModel->deleteLater();
  Init();
}

void AdminWindow::on_labelConnectionChange_linkActivated(const QString&)
{
  if (mConnectionsModel->rowCount() > 0) {
    ui->listViewConnections->setVisible(true);
  }
}
