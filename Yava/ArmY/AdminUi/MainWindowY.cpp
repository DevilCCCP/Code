#include <QList>

#include <Lib/Db/ObjectType.h>
#include <Lib/DbUi/TreeSchema.h>
#include <Lib/DbUi/ToolSchema.h>
#include <LibV/Ui/CameraForm.h>
#include <LibV/Ui/MonitorForm.h>

#include "MainWindowY.h"


void MainWindowY::GetTreeSchema(TreeSchema& schema) const
{
  QList<TreeRootItem> sch;
  sch.append(TreeRootItem("srv", "Сервера"));
  sch.append(TreeRootItem("cam", "Камеры"));
  sch.append(TreeRootItem("rep", "Хранилища"));
  sch.append(TreeRootItem("arm", "АРМ оператора"));
  sch.append(TreeRootItem("upd", "Точки обновления"));
  sch.append(TreeRootItem("usr", "Пользователи"));
  sch.append(TreeRootItem("tmp", "Шаблоны"));
  schema.SetSchema(sch);
}

void MainWindowY::GetToolSchema(ToolSchema& schema) const
{
  QList<ToolLinkItem> sch;
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "srv", "cam"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "srv", "rep"));
  sch.append(ToolLinkItem(ToolLinkItem::eSameMasterLink, "cam", "rep"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "cam", "va1"));
//  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlave, "va1", "xxd"));

  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "srv", "sch"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "cam", "sch"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "rep", "sch"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "arm", "sch"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "upd", "sch"));
  sch.append(ToolLinkItem(ToolLinkItem::eMasterSlaveUniq, "va1", "sch"));
  QStringList rootItems = QStringList() << "cam:TRSU" << "rep:T" << "upd:SR" << "usr:SR";

  QList<ToolStateItem> sch2;
  sch2.append(ToolStateItem(-1, 1, "rep", "Использовать"));
  sch2.append(ToolStateItem(1, 0, "cam", "Включить"));
  sch2.append(ToolStateItem(0, 1, "cam", "Выключить"));
  sch2.append(ToolStateItem(1, 0, "va1", "Включить"));
  sch2.append(ToolStateItem(0, 1, "va1", "Выключить"));
  schema.SetSchema(sch, rootItems, sch2);
}

void MainWindowY::GetPropertiesSchema(QStringList& schema) const
{
  schema << QString::fromUtf8("Свойство") << QString::fromUtf8("Значение");
}

QWidget*MainWindowY::GetSelectObjectWidget(const ObjectItemS& object)
{
  if (object) {
    if (getObjectTable()->IsDefault(object->Id)) {
      return nullptr;
    } else if (object->Type == mCameraType) {
      return new CameraForm(Manager().data(), object, this);
    } else if (object->Type == mAnalType) {
      ObjectItemS objParent;
      if (getObjectTable()->LoadMaster(object->Id, objParent) && objParent->Type == mCameraType) {
        return new CameraForm(Manager().data(), objParent, this);
      }
    } else if (object->Type == mArmType) {
      return new MonitorForm(GetDb(), object, mCameraType, this);
    }
  }

  return nullptr;
}


MainWindowY::MainWindowY(const Db& _Db, const CtrlManagerS& _Manager, QWidget* parent)
  : MainWindow(_Db, _Manager, parent)
  , mObjectType(new ObjectType(_Db))
{
  GetObjectTypeId("cam", mCameraType);
  GetObjectTypeId("arm", mArmType);
  GetObjectTypeId("va",  mAnalType);
}
