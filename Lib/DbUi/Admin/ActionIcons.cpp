#include <Lib/Common/Icon.h>
#include <Lib/Db/ObjectType.h>

#include "ActionIcons.h"


void ActionIcons::Clear()
{
  mIconsMap.clear();
}

void ActionIcons::AddTypeIcon(int typeId, const QString& typeName)
{
  QString baseIcon(":/ObjTree/" + typeName);
  QIcon baseIco(baseIcon);
  if (!QIcon(baseIcon).availableSizes().isEmpty()) {
    QMap<ToolAction::EType, QIcon>& mapActions = mIconsMap[typeId];
    mapActions[ToolAction::eTemplate]    = IconFromImage(baseIcon, QStringLiteral(":/ObjTree/tmp"));
    mapActions[ToolAction::eCreate]      =
        mapActions[ToolAction::eCreateR] = IconFromImage(baseIcon, QStringLiteral(":/ObjTree/Icons/Add.png"));
    mapActions[ToolAction::eRemove]      = IconFromImage(baseIcon, QStringLiteral(":/ObjTree/Icons/Remove.png"));
    mapActions[ToolAction::eLink]        =
        mapActions[ToolAction::eLinkR]   = IconFromImage(baseIcon, QStringLiteral(":/ObjTree/Icons/Link.png"));
    mapActions[ToolAction::eUnlink]      =
        mapActions[ToolAction::eUnlinkR] = IconFromImage(baseIcon, QStringLiteral(":/ObjTree/Icons/Unlink.png"));
    mapActions[ToolAction::eEnable]      = baseIco;
    mapActions[ToolAction::eDisable]     = GrayIcon(baseIco);
  }
}

const QIcon ActionIcons::GetIcon(const ToolAction& action) const
{
  bool primeIcon = true;
  ToolAction::EType actionType = action.getType();
  switch (actionType) {
  case ToolAction::eRefresh:  return mRefreshIcon;
  case ToolAction::eReboot:   return mRebootIcon;
  case ToolAction::eTemplate: primeIcon = true; break;
  case ToolAction::eCreate:   primeIcon = true; break;
  case ToolAction::eCreateR:  primeIcon = true; break;
  case ToolAction::eRemove:   primeIcon = true; break;
  case ToolAction::eLink:     primeIcon = false; break;
  case ToolAction::eUnlink:   primeIcon = false; break;
  case ToolAction::eLinkR:    primeIcon = false; break;
  case ToolAction::eUnlinkR:  primeIcon = false; break;
  case ToolAction::eEnable:   primeIcon = true; break;
  case ToolAction::eDisable:  primeIcon = true; break;
  case ToolAction::eState:    primeIcon = true; actionType = (action.getValue() >= 0)? ToolAction::eEnable: ToolAction::eDisable; break;
  }

  if (const ObjectItemS& item = (primeIcon)? action.getMasterItem(): action.getSlaveItem()) {
    auto itr = mIconsMap.find(item->Type);
    if (itr != mIconsMap.end()) {
      const QMap<ToolAction::EType, QIcon>& mapActions = itr.value();
      auto itr = mapActions.find(actionType);
      if (itr != mapActions.end()) {
        return itr.value();
      }
    }
  }
  return QIcon();
}


ActionIcons::ActionIcons()
  : mRefreshIcon(QIcon(":/ObjTree/Icons/Refresh.png")), mRebootIcon(QIcon(":/ObjTree/Icons/Reboot.png"))
{
}
