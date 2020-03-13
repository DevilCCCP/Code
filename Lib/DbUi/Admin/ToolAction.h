#pragma once

#include <Lib/Db/Db.h>
#include <Lib/Include/Common.h>


class ToolAction {
public:
  enum EType {
    eRefresh,
    eTemplate,
    eCreate,
    eCreateR,
    eRemove,
    eLink,
    eUnlink,
    eLinkR,
    eUnlinkR,
    eState,
    eEnable,
    eDisable,
    eReboot
  };

private:
  PROPERTY_GET_SET(EType,       Type)
  PROPERTY_GET_SET(ObjectItemS, MasterItem)
  PROPERTY_GET_SET(ObjectItemS, SlaveItem)
  PROPERTY_GET_SET(QString,     Name)
  PROPERTY_GET_SET(int,         Value)
  ;
public:
  ToolAction(EType _Type, const ObjectItemS& _MasterItem, const ObjectItemS& _SlaveItem = ObjectItemS())
    : mType(_Type), mMasterItem(_MasterItem), mSlaveItem(_SlaveItem)
  { }
  ToolAction(EType _Type, const ObjectItemS& _MasterItem, const QString& _Name, int _Value)
    : mType(_Type), mMasterItem(_MasterItem), mName(_Name), mValue(_Value)
  { }
};

class QCommandLinkButton;

typedef QList<ToolAction> ToolActionList;
typedef QMap<int, QMap<ToolAction::EType, QIcon> > ToolActionIconMap;
typedef QMap<QCommandLinkButton*, ToolActionList> ToolActionMap;
typedef QList<ToolActionList> ToolActionListList;
