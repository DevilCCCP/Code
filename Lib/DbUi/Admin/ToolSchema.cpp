#include <Lib/Db/ObjectType.h>

#include "ToolSchema.h"
#include "ToolAction.h"


/*
 * ItemsTypes:
 * E (editable) - can create/delete (all exept server, arm, etc)
 * V (visible) - unused
 * T (template) - can create template
 * R (removable) - can be removed
 * S (single) - can be created without any connection
 * U (unlinkable) - can be unlinked from server (can't be removed before unlink)
 * X (unrestartable) - has no restart button
*/
void ToolSchema::SetSchema(const QList<ToolLinkItem>& _Schema, const QStringList& _ItemsTypes, const QList<ToolStateItem>& _Schema2)
{
  mSchema = _Schema;
  mSchema2 = _Schema2;
  mItemsTypes = _ItemsTypes;
  mInit = false;
}

void ToolSchema::SetSchema3(const QList<ToolStateItem>& _Schema3)
{
  mSchema3 = _Schema3;
}

void ToolSchema::Reload(const ObjectTypeTableS& objectTypeTable)
{
  mChildLinks.clear();
  mParentLinks.clear();
  for (int i = 0; i < mSchema.size(); i++) {
    const ToolLinkItem& item = mSchema.at(i);
    const NamedItem* itemTypeFrom = objectTypeTable->GetItemByName(item.getTypeFrom());
    const NamedItem* itemTypeTo = objectTypeTable->GetItemByName(item.getTypeTo());
    if (itemTypeFrom && itemTypeTo) {
      mChildLinks[itemTypeFrom->Id][itemTypeTo->Id] = &item;
      mParentLinks[itemTypeTo->Id][itemTypeFrom->Id] = &item;
    }
  }

  for (int i = 0; i < mSchema2.size(); i++) {
    const ToolStateItem& item = mSchema2.at(i);
    const NamedItem* itemType = objectTypeTable->GetItemByName(item.getType());
    if (itemType) {
      mStates[itemType->Id][item.getStateFrom()] = &item;
    }
  }

  for (int i = 0; i < mSchema3.size(); i++) {
    const ToolStateItem& item = mSchema3.at(i);
    const NamedItem* itemType = objectTypeTable->GetItemByName(item.getType());
    if (itemType) {
      mAfterCreateStates[itemType->Id][item.getStateFrom()] = &item;
    }
  }

  mUnlinkableItems.clear();
  mRemovableItems.clear();
  for (auto itr = mItemsTypes.begin(); itr != mItemsTypes.end(); itr++) {
    QString itemTypes = *itr;
    QStringList pairList = itemTypes.split(':');
    if (pairList.size() < 2) {
      continue;
    }
    QString typeName = pairList[0];
    QString parameters = pairList[1];
    if (const NamedItem* itemType = objectTypeTable->GetItemByName(typeName)) {
      if (parameters.contains('E')) {
        mEditable.insert(itemType->Id);
      }
      if (parameters.contains('V')) {
        mVisible.insert(itemType->Id);
      }
      if (parameters.contains('T')) {
        mTemplatableItems.insert(itemType->Id);
      }
      if (parameters.contains('R')) {
        mRemovableItems.insert(itemType->Id);
      }
      if (parameters.contains('S')) {
        mSingleItems.insert(itemType->Id);
      }
      if (parameters.contains('U')) {
        mUnlinkableItems.insert(itemType->Id);
      }
      if (parameters.contains('X')) {
        mUnrestartable.insert(itemType->Id);
      }
    }
  }
  mInit = true;
}

bool ToolSchema::IsEditable(int id)
{
  return mEditable.contains(id);
}

bool ToolSchema::IsVisible(int id)
{
  return mVisible.contains(id);
}

bool ToolSchema::CanCreateTemplate(int id)
{
  return mTemplatableItems.contains(id);
}

bool ToolSchema::CanUnlink(int id)
{
  return mUnlinkableItems.contains(id);
}

bool ToolSchema::CanRemove(int id)
{
  return mRemovableItems.contains(id);
}

bool ToolSchema::IsSingle(int id)
{
  return mSingleItems.contains(id);
}

bool ToolSchema::IsUnrestartable(int id)
{
  return mUnrestartable.contains(id);
}

bool ToolSchema::CanChangeState(int typeId, int stateFrom, ToolStateAction* stateItem)
{
  auto itr = mStates.find(typeId);
  if (itr != mStates.end()) {
    const ToolStateMap& stateMap = itr.value();
    auto itr = stateMap.find(stateFrom);
    if (itr != stateMap.end()) {
      if (stateItem) {
        *stateItem = itr.value();
      }
      return true;
    }
  }

  return false;
}

QString ToolSchema::ActionToString(const ToolAction& action)
{
//  auto itr = mActionFormat.find(action.getType());
//  QString fmt = (itr != mActionFormat.end())? itr.value(): ActionToStringDefault(action);
  return ActionToStringDefault(action);
}

int ToolSchema::AutoChangeCreateState(int typeId, int oldState)
{
  auto itr = mAfterCreateStates.find(typeId);
  if (itr != mAfterCreateStates.end()) {
    const ToolStateMap& stateMap = itr.value();
    auto itr = stateMap.find(oldState);
    if (itr != stateMap.end()) {
      return itr.value()->getStateTo();
    }
  }

  return oldState;
}

QString ToolSchema::ActionToStringDefault(const ToolAction& action)
{
  switch (action.getType()) {
  case ToolAction::eRefresh:  return QString::fromUtf8("Обновить");
  case ToolAction::eReboot:   return QString::fromUtf8("Перезапустить");
  case ToolAction::eTemplate: return QString::fromUtf8("Шаблон из %1").arg(action.getMasterItem()->Name);
  case ToolAction::eCreate:   return (action.getSlaveItem())? QString::fromUtf8("Создать на %1").arg(action.getMasterItem()->Name)
                                                            : QString::fromUtf8("Создать %1").arg(action.getMasterItem()->Name);
  case ToolAction::eCreateR:  return QString::fromUtf8("Создать %1").arg(action.getMasterItem()->Name);
  case ToolAction::eRemove:   return QString::fromUtf8("Удалить %1").arg(action.getMasterItem()->Name);
  case ToolAction::eLink:     return QString::fromUtf8("Добавить %1").arg(action.getSlaveItem()->Name);
  case ToolAction::eUnlink:   return QString::fromUtf8("Отключить %1").arg(action.getSlaveItem()->Name);
  case ToolAction::eLinkR:    return QString::fromUtf8("Добавить к %1").arg(action.getSlaveItem()->Name);
  case ToolAction::eUnlinkR:  return QString::fromUtf8("Отключить %1 от %2")
        .arg(action.getMasterItem()->Name).arg(action.getSlaveItem()->Name);
  case ToolAction::eEnable:   return QString::fromUtf8("Включить %1").arg(action.getMasterItem()->Name);
  case ToolAction::eDisable:  return QString::fromUtf8("Отключить %1").arg(action.getMasterItem()->Name);
  case ToolAction::eState:    return QString::fromUtf8("%2 %1").arg(action.getMasterItem()->Name).arg(action.getName());
  }
  return "";
}


ToolSchema::ToolSchema()
  : mInit(false)
{
}

