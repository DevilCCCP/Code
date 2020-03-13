#pragma once

#include <QSet>
#include <QString>

#include <Lib/Include/Common.h>
#include <Lib/Db/Db.h>


DefineClassS(ToolSchema);
DefineClassS(ToolAction);

class ToolLinkItem {
public:
  enum ELinkType {
    eMasterSlave,
    eMasterSlaveUniq,
    eSameMasterLink,
    eMasterSlaveLink
  };
private:
  PROPERTY_GET_SET(ELinkType,   LinkType)
  PROPERTY_GET_SET(QString,     TypeFrom)
  PROPERTY_GET_SET(QString,     TypeTo)
  ;
public:
  ToolLinkItem() { }
  ToolLinkItem(ELinkType _LinkType, const char* _TypeFrom, const char* _TypeTo)
    : mLinkType(_LinkType), mTypeFrom(_TypeFrom), mTypeTo(_TypeTo) { }
};
typedef QMap<int, const ToolLinkItem*> ToolLinkMap;

class ToolStateItem {
private:
  PROPERTY_GET_SET(int,     StateFrom)
  PROPERTY_GET_SET(int,     StateTo)
  PROPERTY_GET_SET(QString, Type)
  PROPERTY_GET_SET(QString, Name)
  ;
public:
  ToolStateItem() { }
  ToolStateItem(int _StateFrom, int _StateTo, const QString& _Type, const QString& _Name)
    : mStateFrom(_StateFrom), mStateTo(_StateTo), mType(_Type), mName(_Name) { }
};
typedef QMap<int, const ToolStateItem*> ToolStateMap;
typedef const ToolStateItem* ToolStateAction;

class ToolSchema
{
  QList<ToolLinkItem>       mSchema;
  QList<ToolStateItem>      mSchema2;
  QList<ToolStateItem>      mSchema3;
  QList<QString>            mItemsTypes;

  QSet<int>                 mEditable;
  QSet<int>                 mVisible;
  QSet<int>                 mTemplatableItems;
  QSet<int>                 mUnlinkableItems;
  QSet<int>                 mRemovableItems;
  QSet<int>                 mSingleItems;
  QSet<int>                 mUnrestartable;
  QMap<int, ToolLinkMap>    mChildLinks;
  QMap<int, ToolLinkMap>    mParentLinks;
  QMap<int, ToolStateMap>   mStates;
  QMap<int, ToolStateMap>   mAfterCreateStates;
  QMap<int, QString>        mActionFormat;
  bool                      mInit;

public:
  bool IsInit() { return mInit; }
  void SetSchema(const QList<ToolLinkItem>& _Schema, const QStringList& _ItemsTypes, const QList<ToolStateItem>& _Schema2);
  void SetSchema3(const QList<ToolStateItem>& _Schema3);
  const QList<ToolLinkItem>& GetSchema() const { return mSchema; }

  const ToolLinkMap& ParentLinks(int id) { return mParentLinks[id]; }
  const ToolLinkMap& ChildLinks(int id) { return mChildLinks[id]; }

public:
  void Reload(const ObjectTypeTableS& objectTypeTable);
  bool IsEditable(int id);
  bool IsVisible(int id);
  bool CanCreateTemplate(int id);
  bool CanUnlink(int id);
  bool CanRemove(int id);
  bool IsSingle(int id);
  bool IsUnrestartable(int id);
  bool CanChangeState(int typeId, int stateFrom, ToolStateAction* stateItem = nullptr);
  QString ActionToString(const ToolAction& action);
  int AutoChangeCreateState(int typeId, int oldState);

private:
  QString ActionToStringDefault(const ToolAction& action);

public:
  ToolSchema();
};

