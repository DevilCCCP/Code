#include <QList>

#include <Lib/Db/ObjectType.h>
#include <Lib/DbUi/Tree/TreeSchema.h>
#include <Lib/DbUi/Admin/ToolSchema.h>
#include <LibV/VideoUi/CameraForm.h>
#include <LibV/VideoUi/MonitorForm.h>

#include "MainWindowZ.h"


void MainWindowZ::GetTreeSchema(TreeSchema& schema) const
{
  QList<TreeRootItem> sch;
  sch.append(TreeRootItem("srv", "Сервера", true));
  //sch.append(TreeRootItem("sr_", "Внешние сервера", true));
  sch.append(TreeRootItem("cam", "Камеры", false));
  sch.append(TreeRootItem("rep", "Хранилища", false));
  sch.append(TreeRootItem("arm", "АРМ оператора", false));
  sch.append(TreeRootItem("upd", "Точки обновления", true));
  sch.append(TreeRootItem("usr", "Пользователи", false));
  sch.append(TreeRootItem("tmp", "Шаблоны", true));
  schema.SetSchema(sch);
}

void MainWindowZ::GetToolSchema(ToolSchema& schema) const
{
  QList<ToolLinkItem> sch;
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "srv", "cam"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "srv", "rep"));
  sch.append(ToolLinkItem(ToolLinkItem::eSameMasterLink, "cam", "rep"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "cam", "vac"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "cam", "vaa"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "vac", "iod"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "vac", "ioo"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "vac", "ign"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "vaa", "arn"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "vaa", "ign"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "srv", "www"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "srv", "uni"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "srv", "rpt"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "rpt", "smp"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "smp", "eml"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "srv", "bak"));

  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "srv", "sch"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "cam", "sch"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "rep", "sch"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "arm", "sch"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "upd", "sch"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "usr", "sch"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "vac", "sch"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "iod", "sch"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "vaa", "sch"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "arn", "sch"));
  QStringList rootItems = QStringList() << "cam:TRSU" << "rep:T" << "www:T" << "upd:SR" << "usr:SR";

  QList<ToolStateItem> sch2;
  sch2.append(ToolStateItem(-1, 1, "rep", "Использовать"));
  sch2.append(ToolStateItem(-1, 0, "cam", "Включить"));
  sch2.append(ToolStateItem(0, -1, "cam", "Выключить"));
  sch2.append(ToolStateItem(-1, 0, "vac", "Включить"));
  sch2.append(ToolStateItem(0, -1, "vac", "Выключить"));
  sch2.append(ToolStateItem(-1, 0, "vaa", "Включить"));
  sch2.append(ToolStateItem(0, -1, "vaa", "Выключить"));
  sch2.append(ToolStateItem(-1, 0, "eml", "Включить"));
  sch2.append(ToolStateItem(0, -1, "eml", "Выключить"));
  schema.SetSchema(sch, rootItems, sch2);
}

void MainWindowZ::GetPropertiesSchema(QStringList& schema) const
{
  schema << QString::fromUtf8("Свойство") << QString::fromUtf8("Значение");
}

QWidget* MainWindowZ::GetSelectObjectWidget(const ObjectItemS& object)
{
  if (object) {
    if (getObjectTable()->IsDefault(object->Id)) {
      return nullptr;
    } else if (object->Type == mCameraType) {
      return new CameraForm(Manager().data(), object, mArmId, this);
    } else if (object->Type == mAnalCType) {
      ObjectItemS objParent;
      if (getObjectTable()->LoadMaster(object->Id, objParent) && objParent->Type == mCameraType) {
        return new CameraForm(Manager().data(), objParent, mArmId, this);
      }
    } else if (object->Type == mAnalIoType) {
      ObjectItemS objParent;
      if (getObjectTable()->LoadMaster(object->Id, objParent) && getObjectTable()->LoadMaster(objParent->Id, objParent)
           && objParent && objParent->Type == mCameraType) {
        return new CameraForm(Manager().data(), objParent, object, GetDb(), eLineWithOrder, this);
      }
    } else if (object->Type == mAnalDoorType || object->Type == mAnalRegNumType || object->Type == mAnalIgnType) {
      ObjectItemS objParent;
      if (getObjectTable()->LoadMaster(object->Id, objParent) && getObjectTable()->LoadMaster(objParent->Id, objParent)
           && objParent && objParent->Type == mCameraType) {
        return new CameraForm(Manager().data(), objParent, object, GetDb(), (object->Type == mAnalIgnType? eAreaIgnore: eAreaGood), this);
      }
    } else if (object->Type == mArmType) {
      return new MonitorForm(GetDb(), object, mCameraType, mCameraType, this);
    }
  }

  return nullptr;
}

void MainWindowZ::OnObjectTypeUpdated(const ObjectTypeTableS& objectTypeTable)
{
  Q_UNUSED(objectTypeTable);

  if (const NamedItem* item = objectTypeTable->GetItemByName("cam")) {
    mCameraType = item->Id;
  }
  if (const NamedItem* item = objectTypeTable->GetItemByName("arm")) {
    mArmType = item->Id;
  }
  if (const NamedItem* item = objectTypeTable->GetItemByName("vac")) {
    mAnalCType = item->Id;
  }
  if (const NamedItem* item = objectTypeTable->GetItemByName("iod")) {
    mAnalIoType = item->Id;
  }
  if (const NamedItem* item = objectTypeTable->GetItemByName("ioo")) {
    mAnalDoorType = item->Id;
  }
  if (const NamedItem* item = objectTypeTable->GetItemByName("arn")) {
    mAnalRegNumType = item->Id;
  }
  if (const NamedItem* item = objectTypeTable->GetItemByName("ign")) {
    mAnalIgnType = item->Id;
  }
}

void MainWindowZ::OnObjectsUpdated(const ObjectTableS& objectTable)
{
  if (mArmId) {
    TableItemS itemBase = objectTable->GetItem(mArmId);
    const ObjectItem* item = dynamic_cast<const ObjectItem*>(itemBase.data());
    if (item && item->Guid == "arm") {
      return;
    }
    mArmId = 0;
  }

  const QMap<int, TableItemS>& itemsMap = objectTable->GetItems();
  for (auto itr = itemsMap.begin(); itr != itemsMap.end(); itr++) {
    const TableItemS& itemBase = itr.value();
    const ObjectItem* item = dynamic_cast<const ObjectItem*>(itemBase.data());
    if (item && item->Guid == "arm") {
      mArmId = item->Id;
      break;
    }
  }
}


MainWindowZ::MainWindowZ(Db& _Db, UpInfo* _UpInfo, const CtrlManagerS& _Manager, QWidget* parent)
  : AdminWindow(_Db, _UpInfo, _Manager, parent)
{
}
