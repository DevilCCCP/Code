#pragma once

#include <QMap>

#include "TableNamed.h"

DefineClassS(ObjectTypeTable);
DefineClassS(ObjectTypeItem);
DefineClassS(ObjectTable);
DefineClassS(ObjectItem);

class ObjectTypeItem: public NamedItem
{
public:
  ObjectTypeItem() { }
  /*override*/virtual ~ObjectTypeItem() { }
};

class ObjectTypeTable: public TableNamed
{
protected:
  /*override */virtual const char* Name() override;
  /*override */virtual const char* Select() override;
  /*override */virtual bool OnRowFillItem(QueryS& q, TableItemS& unit) override;

public:
  ObjectTypeTable(const Db& _Db);
  /*override */virtual ~ObjectTypeTable() { }
};

class ObjectItem: public NamedItem
{
public:
  int      Type;
  int      ParentId;
  QString  Guid;
  QString  Version;
  int      Revision;
  QString  Uri;
  int      Status;

public:
  /*override*/virtual bool Equals(const TableItem &other) const override;

public:
  bool IsEqual(const ObjectItem& other);

public:
  ObjectItem(): Type(0), ParentId(0) { }
  /*override*/virtual ~ObjectItem() { }
};

class ObjectTable: public TableNamed
{
  QMultiMap<int, int> mMasterConnection;
  QMultiMap<int, int> mSlaveConnection;

public:
  const QMultiMap<int, int>& MasterConnection() { return mMasterConnection; }
  const QMultiMap<int, int>& SlaveConnection() { return mSlaveConnection; }
  QString GetSetting(int objectId, const QString& settingName);
  bool GetObjectsByType(int objectTypeId, QList<ObjectItemS>& objects);
  bool GetObjectsRealByTypes(const QList<int>& objectTypeId, QList<ObjectItemS>& objects);
  const ObjectItem* GetParent(int id);
  QList<int> GetItemParents(int id);
  bool HasParent(int id);
  bool HasChilds(int id);

public:
  QString SelectColumns(const QString& prefix);

protected:
  /*override */virtual const char* Name() override;
  /*override */virtual const char* Select() override;
  /*override */virtual const char* Insert() override;
  /*override */virtual const char* Update() override;
  /*override */virtual const char* Delete() override;
  /*override */virtual bool OnRowFillItem(QueryS& q, TableItemS& unit) override;
  /*override */virtual bool OnSetItem(QueryS& q, const TableItem& unit) override;

public:
  bool LoadSlaves(int id, QMap<int, ObjectItemS>& items);
  bool LoadSlavesOfTypes(int id, const QString& types, QMap<int, ObjectItemS>& items);
  bool LoadSlaves(int id, QList<int>& list);
  bool LoadMaster(int id, ObjectItemS& item);
  bool LoadMasters(int id, QList<int>& list);
  bool LoadMasterId(int id, int& masterId);
  bool ReloadConnections();
  bool IsPreDefault(int id);
  bool IsDefault(int id);
  bool UpdateVersion(int objectId, const QString& version, bool updateRevision);
  bool UpdateUri(int objectId, const QString& uri);

  bool CreateObject(int templId, const QString& name, int* id = nullptr);
  bool CreateObject(int templId, const QString& name, const QString& guid, int* id = nullptr);
  bool CreateSlaveObject(int masterId, int templId, const QString& name, int* id = nullptr);

  bool UpdateState(ObjectItem* item, int state);
  bool UpdateState(int id, int state);

  bool UpdateRevisions(const QString& where);

  bool GetObjectRevision(int id, int* revision);
  bool GetObjectByGuid(const QString& guid, ObjectItemS& obj);
  bool GetObjectIdByGuid(const QString& guid, int& id);
  bool GetDefaultTemplateObjectId(const QString& typeName, int* id = nullptr);

  bool CreateLink(int masterId, int slaveId);
  bool RemoveLink(int masterId, int slaveId);
  bool RemoveLink(int id);
  bool RemoveSlaves(int masterId);

  bool ClearAllWithType(int typeId);

  bool LoadObjectRevisions(const QString& objTypeIdList, QList<QPair<int, int> >& revisions);

  bool LoginDb(const QString& type, const QString& name, const QString& guid, ObjectItemS& obj);

public:
  ObjectTable(const Db& _Db);
  /*override*/virtual ~ObjectTable() { }
};

