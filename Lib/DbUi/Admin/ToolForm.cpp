#include <QWidgetAction>
#include <QCommandLinkButton>
#include <QPainter>
#include <QPixmap>
#include <QMenu>
#include <QMessageBox>

#include <Lib/Db/ObjectType.h>

#include "ToolForm.h"
#include "ui_ToolForm.h"
#include "AdminWindow.h"
#include "DialogActions.h"


const int kIconSize = 20;
const int kMaxVisibleSize = 16;
const int kMinComboSize = 4;

// Defined in QtAppGui.h
const QString GetProgramName();

void ToolForm::UpdateSchema(const ObjectTypeTableS& objectTypeTable)
{
  if (mFreeze) {
    return;
  }

  mToolSchema->Reload(objectTypeTable);

  mActionIcons.Clear();
  mTypeObjects.clear();
  auto typeItems = objectTypeTable->GetItems();
  for (auto itr = typeItems.begin(); itr != typeItems.end(); itr++) {
    auto item = itr.value();
    if (const ObjectTypeItem* typeItem = dynamic_cast<const ObjectTypeItem*>(item.data())) {
      mActionIcons.AddTypeIcon(typeItem->Id, typeItem->Name);
    }

    ObjectItemS objItem(new ObjectItem());
    objItem->Type = item->Id;
    //objItem->Name = typeItem->Descr;
    mTypeObjects[item->Id] = objItem;
  }

  mTypeObjectDefault.reset(new ObjectItem());
}

void ToolForm::Clear()
{
  if (mFreeze) {
    return;
  }

  if (mToolWidget) {
    mToolWidget->deleteLater();
  }
  if (mToolLayout) {
    mToolLayout->deleteLater();
  }

  mToolWidget = new QWidget(this);
  mToolLayout = new QVBoxLayout(mToolWidget);
  mToolLayout->setContentsMargins(0, 0, 0, 0);

  ui->verticalLayoutToolPlace->insertWidget(0, mToolWidget);

  mActions.clear();
  mCustomActions.clear();
  mToolActionMap.clear();
}

bool ToolForm::SetObject(const ObjectTableS& objectTable, const ObjectItemS& item)
{
  if (mFreeze) {
    return true;
  }

  if (!mToolSchema->IsInit()) {
    return false;
  }
  Clear();

  mObjectTable = objectTable;
  mSelectedItem = item;

  if (mSelectedItem) {
    AddTools();
  }
  AddOneAction(ToolAction(ToolAction::eRefresh, ObjectItemS()));
  return true;
}

void ToolForm::AddCustomAction(QAction* action)
{
  if (mFreeze) {
    return;
  }

  QCommandLinkButton* button = new QCommandLinkButton(mToolWidget);
  button->setText(action->text());
  button->setToolTip(action->toolTip());
  button->setIcon(action->icon());
  button->setIconSize(QSize(kIconSize, kIconSize));
  button->setMaximumWidth(width() - ui->verticalLayoutToolPlace->contentsMargins().left() - ui->verticalLayoutToolPlace->contentsMargins().right());

  mToolLayout->addWidget(button);
  button->setProperty("custom", mCustomActions.size());
  mCustomActions.append(action);

  connect(button, &QCommandLinkButton::clicked, this, &ToolForm::OnCustomAction);
}

void ToolForm::Freeze(bool freeze)
{
  mFreeze = freeze;
}

void ToolForm::AddTools()
{
  const QMap<int, int>& masters = mObjectTable->MasterConnection();
  auto itr = masters.find(mSelectedItem->Id);
  QList<ObjectItemS> masterItems;
  mIsSelectedTemplate = false;
  for (; itr != masters.end() && itr.key() == mSelectedItem->Id; itr++) {
    int masterId = itr.value();
    if (masterId == 0) {
      mIsSelectedTemplate = true;
      masterItems.clear();
      break;
    }
    masterItems.append(mObjectTable->GetItem(masterId).staticCast<ObjectItem>());
  }

  if (mToolSchema->CanCreateTemplate(mSelectedItem->Type)) {
    AddOneAction(ToolAction(ToolAction::eTemplate, mSelectedItem));
  }
  if (!masterItems.isEmpty()) {
    bool canRemove = true;
    for (auto itr = masterItems.begin(); itr != masterItems.end(); itr++) {
      ObjectItemS masterItem = *itr;
      if (mToolSchema->CanUnlink(mSelectedItem->Type)) {
        AddOneAction(ToolAction(ToolAction::eUnlinkR, mSelectedItem, masterItem));
        canRemove = false;
      }
    }
    if (canRemove && !mToolSchema->IsSingle(mSelectedItem->Type)) {
      AddOneAction(ToolAction(ToolAction::eRemove, mSelectedItem));
    }
  } else {
    if (!mObjectTable->IsPreDefault(mSelectedItem->Id) && mToolSchema->CanRemove(mSelectedItem->Type)) {
      AddOneAction(ToolAction(ToolAction::eRemove, mSelectedItem));
    }
  }

  const ToolLinkMap& parentLinks = mToolSchema->ParentLinks(mSelectedItem->Type);
  for (auto itr = parentLinks.begin(); itr != parentLinks.end(); itr++) {
    int parentTypeId = itr.key();
    const ToolLinkItem* link = itr.value();
    bool parentExists = false;
    for (auto itr = masterItems.begin(); itr != masterItems.end(); itr++) {
      const ObjectItemS& masterItem = *itr;
      if (masterItem->Type == parentTypeId) {
        parentExists = true;
        break;
      }
    }
    AddParentLink(link, parentTypeId, parentExists);
  }

  if (mIsSelectedTemplate) {
    if (mToolSchema->IsSingle(mSelectedItem->Type)) {
      AddOneAction(ToolAction(ToolAction::eCreate, mSelectedItem));
    }
  } else {
    const ToolLinkMap& childsLinks = mToolSchema->ChildLinks(mSelectedItem->Type);
    for (auto itr = childsLinks.begin(); itr != childsLinks.end(); itr++) {
      int childTypeId = itr.key();
      const ToolLinkItem* link = *itr;
      AddChildLink(link, childTypeId);
    }
  }

  if (!mIsSelectedTemplate) {
    ToolStateAction stateAction;
    if (mToolSchema->CanChangeState(mSelectedItem->Type, mSelectedItem->Status, &stateAction)) {
      AddOneAction(ToolAction(ToolAction::eState, mSelectedItem, stateAction->getName(), stateAction->getStateTo()));
    }
    if (!mToolSchema->IsUnrestartable(mSelectedItem->Type)) {
      AddOneAction(ToolAction(ToolAction::eReboot, mSelectedItem));
    }
  }
}

void ToolForm::AddParentLink(const ToolLinkItem* link, int parentTypeId, bool parentExists)
{
  if (parentExists && link->getLinkType() != ToolLinkItem::eSameMasterLink) {
    return;
  }
  switch (link->getLinkType()) {
  case ToolLinkItem::eMasterSlave:     return AddLinkTo(parentTypeId, false, true);
  case ToolLinkItem::eMasterSlaveUniq: return AddLinkTo(parentTypeId, true, true);
  case ToolLinkItem::eSameMasterLink:  return AddLinkToSameMaster(parentTypeId);
  case ToolLinkItem::eMasterSlaveLink: return AddLinkTo(parentTypeId, false, false);
  }
}

void ToolForm::AddChildLink(const ToolLinkItem* link, int childTypeId)
{
  switch (link->getLinkType()) {
  case ToolLinkItem::eMasterSlave:     return AddLinkFrom(childTypeId, false, true);
  case ToolLinkItem::eMasterSlaveUniq: return AddLinkFrom(childTypeId, true, true);
  case ToolLinkItem::eSameMasterLink:  return AddLinkFromSameMaster(childTypeId);
  case ToolLinkItem::eMasterSlaveLink: return AddLinkFrom(childTypeId, false, false);
  }
}

void ToolForm::AddLinkTo(int parentTypeId, bool uniq, bool create)
{
  if (mIsSelectedTemplate && !create) {
    return;
  }

  QList<ToolAction> actionsLink;
  QList<ToolAction> actionsUnlink;
  auto items = mObjectTable->GetItems();
  for (auto itr = items.begin(); itr != items.end(); itr++) {
    const ObjectItemS& item = itr.value().staticCast<ObjectItem>();
    if (item->Type != parentTypeId || mObjectTable->IsDefault(item->Id)) {
      continue;
    }
    if (uniq && IsChildExist(item->Id, mSelectedItem->Type)) {
      continue;
    }
    if (mIsSelectedTemplate) {
      actionsLink.append(ToolAction(ToolAction::eCreate, item, mSelectedItem));
    } else if (!mObjectTable->SlaveConnection().values(item->Id).contains(mSelectedItem->Id)) {
      actionsLink.append(ToolAction(ToolAction::eLinkR, mSelectedItem, item));
    } else {
      actionsUnlink.append(ToolAction(ToolAction::eUnlinkR, mSelectedItem, item));
    }
  }
  AddMultyActions(actionsLink);
  AddMultyActions(actionsUnlink);
}

void ToolForm::AddLinkToSameMaster(int parentTypeId)
{
  if (mIsSelectedTemplate) {
    return;
  }
  QList<ToolAction> actionsLink;
  QList<ToolAction> actionsUnlink;
  auto items = mObjectTable->GetItems();
  for (auto itr = items.begin(); itr != items.end(); itr++) {
    const ObjectItemS& item = itr.value().staticCast<ObjectItem>();
    if (item->Type != parentTypeId || mObjectTable->IsDefault(item->Id)) {
      continue;
    }
    bool connected;
    if (IsSameMaster(item->Id, false, connected)) {
      if (connected) {
        actionsUnlink.append(ToolAction(ToolAction::eUnlinkR, mSelectedItem, item));
      } else {
        actionsLink.append(ToolAction(ToolAction::eLinkR, mSelectedItem, item));
      }
    }
  }
  AddMultyActions(actionsLink);
  AddMultyActions(actionsUnlink);
}

void ToolForm::AddLinkFrom(int childTypeId, bool uniq, bool create)
{
  if (mIsSelectedTemplate) {
    return;
  }
  QList<ToolAction> actionsCreate;
  QList<ToolAction> actionsLink;
  QList<ToolAction> actionsUnlink;
  auto items = mObjectTable->GetItems();
  for (auto itr = items.begin(); itr != items.end(); itr++) {
    const ObjectItemS& item = itr.value().staticCast<ObjectItem>();
    if (item->Type != childTypeId) {
      continue;
    }
    if (uniq && IsChildExist(mSelectedItem->Id, item->Type)) {
      continue;
    }
    if (mObjectTable->IsDefault(item->Id)) {
      if (create) {
        actionsCreate.append(ToolAction(ToolAction::eCreateR, item, mSelectedItem));
      }
    } else if (create) {
      if (!mObjectTable->MasterConnection().contains(item->Id)) {
        actionsLink.append(ToolAction(ToolAction::eLink, mSelectedItem, item));
      }
    } else if (mObjectTable->MasterConnection().contains(item->Id)) {
      if (!mObjectTable->MasterConnection().values(item->Id).contains(mSelectedItem->Id)) {
        actionsLink.append(ToolAction(ToolAction::eLink, mSelectedItem, item));
      } else {
        actionsUnlink.append(ToolAction(ToolAction::eUnlink, mSelectedItem, item));
      }
    }
  }
  AddMultyActions(actionsCreate);
  AddMultyActions(actionsLink);
  AddMultyActions(actionsUnlink);
}

void ToolForm::AddLinkFromSameMaster(int childTypeId)
{
  if (mIsSelectedTemplate) {
    return;
  }

  QList<ToolAction> actionsLink;
  QList<ToolAction> actionsUnlink;
  auto items = mObjectTable->GetItems();
  for (auto itr = items.begin(); itr != items.end(); itr++) {
    const ObjectItemS& item = itr.value().staticCast<ObjectItem>();
    if (item->Type != childTypeId) {
      continue;
    }
    bool connected;
    if (IsSameMaster(item->Id, true, connected)) {
      if (connected) {
        actionsUnlink.append(ToolAction(ToolAction::eUnlink, mSelectedItem, item));
      } else {
        actionsLink.append(ToolAction(ToolAction::eLink, mSelectedItem, item));
      }
    }
  }
  AddMultyActions(actionsLink);
  AddMultyActions(actionsUnlink);
}

bool ToolForm::IsChildExist(int parentId, int childTypeId)
{
  auto slaves = mObjectTable->SlaveConnection();
  auto itr = slaves.find(parentId);
  for (; itr != slaves.end() && itr.key() == parentId; itr++) {
    int slaveId = itr.value();
    const ObjectItemS& item = mObjectTable->GetItem(slaveId).staticCast<ObjectItem>();
    if (item && item->Type == childTypeId) {
      return true;
    }
  }
  return false;
}

bool ToolForm::IsSameMaster(int idSibling, bool from, bool& connected)
{
  const QMap<int, int>& masters = mObjectTable->MasterConnection();
  QSet<int> mastersSelected = masters.values(mSelectedItem->Id).toSet();
  QSet<int> mastersSibling = masters.values(idSibling).toSet();
  if (from) {
    connected = mastersSibling.contains(mSelectedItem->Id);
  } else {
    connected = mastersSelected.contains(idSibling);
  }
  return !mastersSelected.intersect(mastersSibling).isEmpty();
}

void ToolForm::AddOneAction(const ToolAction& action)
{
  //QWidgetAction *action = new QWidgetAction(this);
  QCommandLinkButton* button = new QCommandLinkButton(mToolWidget);
  button->setText(mToolSchema->ActionToString(action));
  button->setIcon(mActionIcons.GetIcon(action));
  button->setIconSize(QSize(kIconSize, kIconSize));
  button->setMaximumWidth(width() - ui->verticalLayoutToolPlace->contentsMargins().left() - ui->verticalLayoutToolPlace->contentsMargins().right());

  mToolLayout->addWidget(button);
  button->setProperty("action", mActions.size());
  mActions.append(action);

  connect(button, &QCommandLinkButton::clicked, this, &ToolForm::OnAction);
}

void ToolForm::AddMultyActions(const QList<ToolAction>& actions)
{
  if (actions.isEmpty()) {
    return;
  } else if (actions.size() == 1) {
    return AddOneAction(actions.first());
  } else if (actions.size() > kMaxVisibleSize) {
    AddFormActions(actions);
    return;
  }

  ToolAction testAction = actions.first();
  QIcon icon = mActionIcons.GetIcon(testAction);
  testAction.setMasterItem(GetTypeObject(testAction.getMasterItem()));
  testAction.setSlaveItem(GetTypeObject(testAction.getSlaveItem()));
  QCommandLinkButton* button = new QCommandLinkButton(mToolWidget);
  button->setText(mToolSchema->ActionToString(testAction));
  button->setIcon(icon);
  button->setIconSize(QSize(kIconSize, kIconSize));
  button->setMaximumWidth(width() - ui->verticalLayoutToolPlace->contentsMargins().left() - ui->verticalLayoutToolPlace->contentsMargins().right());

  QMenu* menu = new QMenu(button);
  if (actions.size() >= kMinComboSize) {
    AddFormActions(menu, actions);
  }
  for (auto itr = actions.begin(); itr != actions.end(); itr++) {
    const ToolAction& action = *itr;
    QAction* act = menu->addAction(icon, mToolSchema->ActionToString(action));
    act->setData(mActions.size());
    mActions.append(action);

    connect(act, &QAction::triggered, this, &ToolForm::OnMultyAction);
  }
  button->setMenu(menu);
  mToolLayout->addWidget(button);

  connect(button, &QCommandLinkButton::clicked, this, &ToolForm::OnAction);
}

void ToolForm::AddFormActions(const QList<ToolAction>& actions)
{
  ToolAction testAction = actions.first();
  QIcon icon = mActionIcons.GetIcon(testAction);
  testAction.setMasterItem(GetTypeObject(testAction.getMasterItem()));
  testAction.setSlaveItem(GetTypeObject(testAction.getSlaveItem()));
  QCommandLinkButton* button = new QCommandLinkButton(mToolWidget);
  button->setText(mToolSchema->ActionToString(testAction) + "...");
  button->setIcon(icon);
  button->setIconSize(QSize(kIconSize, kIconSize));
  button->setMaximumWidth(width() - ui->verticalLayoutToolPlace->contentsMargins().left() - ui->verticalLayoutToolPlace->contentsMargins().right());
  mToolActionMap[button] = actions;

  mToolLayout->addWidget(button);
  connect(button, &QCommandLinkButton::clicked, this, &ToolForm::OnFormAction);
}

void ToolForm::AddFormActions(QMenu* menu, const QList<ToolAction>& actions)
{
  ToolAction testAction = actions.first();
  QIcon icon = mActionIcons.GetIcon(testAction);
  testAction.setMasterItem(GetTypeObject(testAction.getMasterItem()));
  testAction.setSlaveItem(GetTypeObject(testAction.getSlaveItem()));
  QAction* act = menu->addAction(icon, mToolSchema->ActionToString(testAction) + "...");
  act->setData(mDialogActions.size());
  mDialogActions.append(actions);

  connect(act, &QAction::triggered, this, &ToolForm::OnDialogAction);
}

void ToolForm::OnAction()
{
  QCommandLinkButton* snd = qobject_cast<QCommandLinkButton*>(sender());
  if (!snd) {
    return;
  }

  bool ok;
  int index = snd->property("action").toInt(&ok);
  if (ok && index >= 0 && index < mActions.size()) {
    DoAction(mActions.at(index));
  } else {
    QMessageBox::warning(this, GetProgramName(), QString::fromUtf8("Внутренняя ошибка:\nНе удалось сопоставить действие с кнопкой\n"));
  }
}

void ToolForm::OnFormAction()
{
  QCommandLinkButton* snd = qobject_cast<QCommandLinkButton*>(sender());
  if (!snd) {
    return;
  }

  auto itr = mToolActionMap.find(snd);
  if (itr != mToolActionMap.end()) {
    const ToolActionList& actions = itr.value();
    OpenActionsForm(actions);
  }
}

void ToolForm::OnCustomAction()
{
  QCommandLinkButton* snd = qobject_cast<QCommandLinkButton*>(sender());
  if (!snd) {
    return;
  }

  bool ok;
  int index = snd->property("custom").toInt(&ok);
  if (ok && index >= 0 && index < mCustomActions.size()) {
    mCustomActions.at(index)->trigger();
  } else {
    QMessageBox::warning(this, GetProgramName(), QString::fromUtf8("Внутренняя ошибка:\nНе удалось сопоставить действие с кнопкой\n"));
  }
}

void ToolForm::OnMultyAction()
{
  QAction* snd = qobject_cast<QAction*>(sender());
  if (!snd) {
    return;
  }

  bool ok;
  int index = snd->data().toInt(&ok);
  if (ok && index >= 0 && index < mActions.size()) {
    DoAction(mActions.at(index));
  }
}

void ToolForm::OnDialogAction()
{
  QAction* snd = qobject_cast<QAction*>(sender());
  if (!snd) {
    return;
  }

  bool ok;
  int index = snd->data().toInt(&ok);
  if (ok && index >= 0 && index < mDialogActions.size()) {
    OpenActionsForm(mDialogActions.at(index));
  }
}

void ToolForm::OpenActionsForm(const ToolActionList& actions)
{
  if (actions.isEmpty()) {
    return;
  }

  DialogActions* dialogActions = new DialogActions(this);
  dialogActions->Init(&mActionIcons, actions);
  ToolAction testAction = actions.first();
  testAction.setMasterItem(GetTypeObject(testAction.getMasterItem()));
  testAction.setSlaveItem(GetTypeObject(testAction.getSlaveItem()));
  dialogActions->setWindowIcon(mActionIcons.GetIcon(testAction));
  dialogActions->setWindowTitle(mToolSchema->ActionToString(testAction) + "...");

  if (dialogActions->exec() != QDialog::Accepted) {
    return;
  }
  QList<int> results = dialogActions->SelectedActions();
  foreach (int i, results) {
    DoAction(actions.at(i));
  }
}

void ToolForm::DoAction(const ToolAction& action)
{
  switch (action.getType()) {
  case ToolAction::eRefresh:  mAdminWindow->ReloadTree(); return;
  case ToolAction::eReboot:   return RebootTree(action.getMasterItem());
  case ToolAction::eTemplate: return CreateSlave(0, action.getMasterItem());
  case ToolAction::eCreate:   return action.getSlaveItem()? CreateSlave(action.getMasterItem()->Id, action.getSlaveItem())
                                                          : CreateSingle(action.getMasterItem());
  case ToolAction::eCreateR:  return CreateSlave(action.getSlaveItem()->Id, action.getMasterItem());
  case ToolAction::eRemove:   return RemoveObject(action.getMasterItem());
  case ToolAction::eLink:     return CreateLink(action.getMasterItem()->Id, action.getSlaveItem()->Id);
  case ToolAction::eUnlink:   return RemoveLink(action.getMasterItem()->Id, action.getSlaveItem()->Id);
  case ToolAction::eLinkR:    return CreateLink(action.getSlaveItem()->Id, action.getMasterItem()->Id);
  case ToolAction::eUnlinkR:  return RemoveLink(action.getSlaveItem()->Id, action.getMasterItem()->Id);
  case ToolAction::eEnable:   return SetObjectState(action.getMasterItem(), 0);
  case ToolAction::eDisable:  return SetObjectState(action.getMasterItem(), -1);
  case ToolAction::eState:    return SetObjectState(action.getMasterItem(), action.getValue());
  }
}

void ToolForm::CreateSingle(const ObjectItemS& baseItem)
{
  QString name = GenerateName(baseItem->Name);
  if (mObjectTable->CreateObject(baseItem->Id, name)) {
    mAdminWindow->ReloadTree();
  } else {
    QMessageBox::warning(this, GetProgramName(), QString::fromUtf8("Внутренняя ошибка:\nНе удалось создать объект\n"));
  }
}

void ToolForm::CreateSlave(int masterId, const ObjectItemS& baseItem)
{
  QString name = GenerateName(baseItem->Name);
  if (mObjectTable->CreateSlaveObject(masterId, baseItem->Id, name)) {
    mAdminWindow->ReloadTree();
  } else {
    QMessageBox::warning(this, GetProgramName(), QString::fromUtf8("Внутренняя ошибка:\nНе удалось создать объект\n"));
  }
}

void ToolForm::RemoveObject(const ObjectItemS& item)
{
  if (mObjectTable->RemoveItem(item->Id)) {
    mAdminWindow->ReloadTree();
  } else {
    QMessageBox::warning(this, GetProgramName(), QString::fromUtf8("Внутренняя ошибка:\nНе удалось удалить объект\n"));
  }
}

void ToolForm::CreateLink(int masterId, int slaveId)
{
  if (mObjectTable->CreateLink(masterId, slaveId)) {
    mAdminWindow->ReloadTree();
  } else {
    QMessageBox::warning(this, GetProgramName(), QString::fromUtf8("Внутренняя ошибка:\nНе удалось создать соединение\n"));
  }
}

void ToolForm::RemoveLink(int masterId, int slaveId)
{
  if (mObjectTable->RemoveLink(masterId, slaveId)) {
    mAdminWindow->ReloadTree();
  } else {
    QMessageBox::warning(this, GetProgramName(), QString::fromUtf8("Внутренняя ошибка:\nНе удалось удалить соединение\n"));
  }
}

void ToolForm::SetObjectState(const ObjectItemS& item, int state)
{
  if (mObjectTable->UpdateState(item.data(), state)) {
    mAdminWindow->ReloadTree();
  } else {
    QMessageBox::warning(this, GetProgramName(), QString::fromUtf8("Внутренняя ошибка:\nНе удалось изменить объект\n"));
  }
}

void ToolForm::RebootTree(const ObjectItemS& item)
{
  QStringList updateList;
  QVector<int> idList;
  idList.append(item->Id);
  while (!idList.isEmpty()) {
    QVector<int> idNextList;
    foreach (int id, idList) {
      auto itr = mObjectTable->SlaveConnection().find(id);
      if (itr != mObjectTable->SlaveConnection().end()) {
        for (; itr != mObjectTable->SlaveConnection().end() && itr.key() == id; itr++) {
          idNextList.append(itr.value());
        }
      } else {
        updateList.append(QString::number(id));
      }
    }
    idList.swap(idNextList);
  }

  if (!mObjectTable->UpdateRevisions(QString("WHERE _id IN (%1)").arg(updateList.join(',')))) {
    QMessageBox::warning(this, GetProgramName(), QString::fromUtf8("Внутренняя ошибка:\nНе удалось перегрузить объекты\n"));
  }
}

QString ToolForm::GenerateName(const QString& baseName)
{
  if (mLastNameTemplate != baseName) {
    mLastNameTemplate = baseName;
    int ind = baseName.lastIndexOf(' ');
    if (ind >= 0) {
      bool ok;
      mLastNameAdding = baseName.mid(ind + 1).toInt(&ok);
      if (ok) {
        mLastNameAdding++;
        mLastNameTemplateBase = baseName.mid(0, ind + 1);
        return mLastNameTemplateBase + QString::number(mLastNameAdding);
      }
    }

    mLastNameTemplateBase = mLastNameTemplate + " ";
    mLastNameAdding = 1;
  } else {
    mLastNameAdding++;
  }
  return mLastNameTemplateBase + QString::number(mLastNameAdding);
}

const ObjectItemS& ToolForm::GetTypeObject(const ObjectItemS& item)
{
  if (item) {
    auto itr = mTypeObjects.find(item->Type);
    if (itr != mTypeObjects.end()) {
      return itr.value();
    }
  }
  return mTypeObjectDefault;
}


ToolForm::ToolForm(AdminWindow* _AdminWindow, const Db& _Db, const ToolSchemaS& _ToolSchema, QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::ToolForm)
  , mAdminWindow(_AdminWindow), mDb(_Db)
  , mToolSchema(_ToolSchema), mToolWidget(nullptr), mToolLayout(nullptr)
  , mFreeze(false)
{
  ui->setupUi(this);

  setStyleSheet("font: 8pt \"Segoe UI\";");
}

ToolForm::~ToolForm()
{
  delete ui;
}
