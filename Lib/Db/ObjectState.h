#pragma once

#include <QDateTime>
#include <QElapsedTimer>

#include "TableNamed.h"

DefineDbClassS(ObjectStateValuesTable);
DefineDbClassS(ObjectStateTypeTable);
DefineDbClassS(ObjectState);


// ================================================
// state values
// --
class ObjectStateValuesItem: public NamedItem
{
public:
  int     Type;
  int     State;
  QString Color;

public:
  /*override*/virtual bool Equals(const TableItem &other) const override;

public:
  ObjectStateValuesItem() { }
  /*override*/virtual ~ObjectStateValuesItem() { }
};

class ObjectStateValuesTable: public TableNamed
{
  QMap<QPair<int, int>, const ObjectStateValuesItem*> mTypeStateIndex;

protected:
  /*override */virtual const char* Name() override;
  /*override */virtual const char* Select() override;
  /*override */virtual bool OnRowFillItem(QueryS& q, TableItemS& unit) override;
  /*override */virtual void CreateIndexes() override;
  /*override */virtual void ClearIndexes() override;

public:
  const ObjectStateValuesItem* GetItemByTypeState(int type, int state);

public:
  ObjectStateValuesTable(const Db& _Db);
  /*override*/virtual ~ObjectStateValuesTable();
};

// ================================================
// state type
// --
class ObjectStateTypeItem: public NamedItem
{
//  QMap<int, ObjectStateValuesItem> mStateValues;

public:
  ObjectStateTypeItem() { }
  /*override*/virtual ~ObjectStateTypeItem() { }
};

class ObjectStateTypeTable: public TableNamed
{
protected:
  /*override*/virtual const char* Select() override;
  /*override*/virtual bool OnRowFillItem(QueryS& q, TableItemS& unit) override;

public:
  ObjectStateTypeTable(const Db& _Db);
  /*override*/virtual ~ObjectStateTypeTable();
};

// ================================================
// state
// --
class ObjectStateItem: public TableItem
{
public:
  int       ObjectId;
  int       ObjectStateTypeId;
  int       State;
  int       ChangeSec;

public:
  /*override*/virtual bool Equals(const TableItem &other) const override;

public:
  ObjectStateItem() { }
  /*override*/virtual ~ObjectStateItem() { }
};

class ObjectState: public Table
{
  ObjectStateItem*            mState;
  QElapsedTimer               mUpdateTimer;
  bool                        mAutoTimer;

  QMap<int, ObjectStateItem*> mObjectIndex;

public:
  //
  // !!! If edit: Update object_states.sql !!!
  //
  enum EStateTypes {
    ePower   = 1,
    eService = 2,
    eConnect = 3,
    eStateTypeIllegal
  };

  enum EPowerState {
    eOff   = 0,
    eOn    = 1,
    eSleep = 2
  };

  enum EServiceState {
    eError        = -2,
    eWarning      = -1,
    eNotAvailable = 0,
    eGood         = 1
  };

  enum EConnectState {
    eDisconnected = -1,
    eConnected    = 1
  };

protected:
  /*override */virtual const char* Name() override;
  /*override*/virtual const char* Select() override;
  /*override*/virtual bool OnRowFillItem(QueryS& q, TableItemS& unit) override;

  /*override*/virtual void CreateIndexes() override;
  /*override*/virtual void ClearIndexes() override;

public:
  bool InitState(int _ObjectId, int _ObjectStateTypeId, int _DefaultState, int _NowState);
  bool UpdateState(int newState);
  bool LoadLogCount(qint64& count);
  bool LoadLogCount(const QString& where, qint64& count);

  ObjectStateItem* GetObjectState(int id);

public:
  ObjectState(const Db& _Db, bool _AutoTimer = true);
  /*override*/virtual ~ObjectState();
};

class ObjectStateTable: public ObjectState
{
public:
  ObjectStateTable(const Db& _Db, bool _AutoTimer = true): ObjectState(_Db, _AutoTimer) { }
  /*override*/virtual ~ObjectStateTable() { }
};
