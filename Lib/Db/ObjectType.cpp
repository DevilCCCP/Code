#include <QUuid>

#include <Lib/Log/Log.h>

#include "ObjectType.h"


bool ObjectItem::Equals(const TableItem& other) const
{
  const ObjectItem& other_ = dynamic_cast<const ObjectItem&>(other);
  return Id == other_.Id && Name == other_.Name && Descr == other_.Descr
      && Type == other_.Type && ParentId == other_.ParentId && Guid == other_.Guid
      && Version == other_.Version && Revision == other_.Revision
      && Uri == other_.Uri && Status == other_.Status;
}

bool ObjectItem::IsEqual(const ObjectItem& other)
{
  return Name == other.Name && Descr == other.Descr && Version == other.Version
      && Revision == other.Revision && Uri == other.Uri && Status == other.Status;
}


const char* ObjectTypeTable::Name()
{
  return "object_type";
}

const char* ObjectTypeTable::Select()
{
  return "SELECT _id, name, descr FROM object_type";
}

bool ObjectTypeTable::OnRowFillItem(QueryS &q, TableItemS &unit)
{
  unit = TableItemS(new ObjectTypeItem());
  return TableNamed::OnRowFillItem(q, unit);
}


ObjectTypeTable::ObjectTypeTable(const Db& _Db)
  : TableNamed(_Db, true)
{
}


QString ObjectTable::GetSetting(int objectId, const QString& settingName)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT value FROM object_settings WHERE _object = %1 AND key = '%2'")
             .arg(objectId).arg(settingName));
  return mDb.ExecuteScalar(q).toString();
}

bool ObjectTable::GetObjectsByType(int objectTypeId, QList<ObjectItemS>& objects)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("%1 WHERE _otype = %2 ORDER BY _id").arg(Select()).arg(objectTypeId));
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    TableItemS unit;
    if (OnRowLoad(q, unit)) {
      objects.append(unit.staticCast<ObjectItem>());
    }
  }
  return true;
}

bool ObjectTable::GetObjectsRealByTypes(const QList<int>& objectTypeId, QList<ObjectItemS>& objects)
{
  QStringList typeRange;
  foreach (int typeId, objectTypeId) {
    typeRange.append(QString::number(typeId));
  }

  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT %1 FROM object o"
                     " LEFT JOIN object_connection c ON c._omaster = 0 AND c._oslave = o._id"
                     " WHERE o._otype IN (%2) AND c._id IS NULL"
                     " ORDER BY o._id").arg(SelectColumns("o")).arg(typeRange.join(',')));
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    TableItemS unit;
    if (OnRowLoad(q, unit)) {
      objects.append(unit.staticCast<ObjectItem>());
    }
  }
  return true;
}

const ObjectItem* ObjectTable::GetParent(int id)
{
  auto itr = mMasterConnection.find(id);
  if (itr != mMasterConnection.end()) {
    if (TableItemS parentItem = GetItem(itr.value())) {
      const ObjectItem* parent = static_cast<const ObjectItem*>(parentItem.data());
      return parent;
    }
  }
  return nullptr;
}

QList<int> ObjectTable::GetItemParents(int id)
{
  return mMasterConnection.values(id);
}

bool ObjectTable::HasParent(int id)
{
  return mMasterConnection.find(id) != mMasterConnection.end();
}

bool ObjectTable::HasChilds(int id)
{
  return mSlaveConnection.find(id) != mSlaveConnection.end();
}

QString ObjectTable::SelectColumns(const QString& prefix)
{
  return QString("%1._id, %1.name, %1.descr, %1._otype, %1._parent, %1.guid, %1.version, %1.revision, %1.uri, %1.status").arg(prefix);
}

const char* ObjectTable::Name()
{
  return "object";
}

const char* ObjectTable::Select()
{
  return "SELECT _id, name, descr, _otype, _parent, guid, version, revision, uri, status FROM object";
}

const char* ObjectTable::Insert()
{
  return "INSERT INTO object(_otype, _parent, guid, name, descr, version, revision, uri, status)"
         " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
}

const char* ObjectTable::Update()
{
  return "UPDATE object SET _otype=?, _parent=?, guid=?, name=?, descr=?, version=?, revision=?, uri=?, status=?";
}

const char* ObjectTable::Delete()
{
  return "DELETE FROM object";
}

bool ObjectTable::OnRowFillItem(QueryS &q, TableItemS &unit)
{
  unit = TableItemS(new ObjectItem());
  ObjectItem* item = static_cast<ObjectItem*>(unit.data());
  int index = 3;
  item->Type = q->value(index++).toInt();
  item->ParentId = q->value(index++).toInt();
  item->Guid = q->value(index++).toString();
  item->Version = q->value(index++).toString();
  item->Revision = q->value(index++).toInt();
  item->Uri = q->value(index++).toString();
  item->Status = q->value(index++).toInt();
  return TableNamed::OnRowFillItem(q, unit);
}

bool ObjectTable::OnSetItem(QueryS& q, const TableItem& unit)
{
  const ObjectItem& item = static_cast<const ObjectItem&>(unit);
  int index = 0;
  q->bindValue(index++, Db::ToKey(item.Type));
  q->bindValue(index++, Db::ToKey(item.ParentId));
  q->bindValue(index++, item.Guid);
  q->bindValue(index++, item.Name);
  q->bindValue(index++, item.Descr);
  q->bindValue(index++, item.Version);
  q->bindValue(index++, item.Revision);
  q->bindValue(index++, item.Uri);
  q->bindValue(index++, item.Status);
  return true;
}

bool ObjectTable::LoadSlaves(int id, QMap<int, ObjectItemS>& items)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT %1 FROM object o"
                     " INNER JOIN object_connection oc ON oc._oslave = o._id"
                     " WHERE oc._omaster = %2;").arg(SelectColumns("o")).arg(id));
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    int id_ = q->value(0).toInt();
    TableItemS unit;
    if (!OnRowFillItem(q, unit)) {
      return false;
    }
    ObjectItemS item = unit.staticCast<ObjectItem>();
    item->Id = id_;
    items[id_] = item;
  }
  return true;
}

bool ObjectTable::LoadSlavesOfTypes(int id, const QString& types, QMap<int, ObjectItemS>& items)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT %1 FROM object o"
                     " INNER JOIN object_connection oc ON oc._oslave = o._id"
                     " WHERE oc._omaster = %2 AND o._otype IN (%3);").arg(SelectColumns("o")).arg(id).arg(types));
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    int id_ = q->value(0).toInt();
    TableItemS unit;
    if (!OnRowFillItem(q, unit)) {
      return false;
    }
    ObjectItemS item = unit.staticCast<ObjectItem>();
    item->Id = id_;
    items[id_] = item;
  }
  return true;
}

bool ObjectTable::LoadMaster(int id, ObjectItemS& item)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT %1 FROM object o"
                     " INNER JOIN object_connection oc ON oc._omaster = o._id"
                     " WHERE oc._oslave = %2;").arg(SelectColumns("o")).arg(id));
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    int id_ = q->value(0).toInt();
    TableItemS unit;
    if (!OnRowFillItem(q, unit)) {
      return false;
    }
    item = unit.staticCast<ObjectItem>();
    item->Id = id_;
    return true;
  }
  item.clear();
  return true;
}

bool ObjectTable::LoadMasters(int id, QList<int>& list)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT _omaster FROM object_connection WHERE _oslave = %1;").arg(id));
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    int id_ = q->value(0).toInt();
    list.append(id_);
  }
  return true;
}

bool ObjectTable::LoadMasterId(int id, int& masterId)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT oc._omaster FROM object_connection oc"
                     " INNER JOIN object o ON o._id = oc._oslave"
                     " WHERE oc._oslave = %1 AND oc._omaster <> o._parent;").arg(id));
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    masterId = q->value(0).toInt();
    return true;
  }
  masterId = 0;
  return false;
}

bool ObjectTable::LoadSlaves(int id, QList<int>& list)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT _oslave FROM object_connection WHERE _omaster = %1;").arg(id));
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    int id_ = q->value(0).toInt();
    list.append(id_);
  }
  return true;
}

bool ObjectTable::ReloadConnections()
{
  auto q = mDb.MakeQuery();
  q->prepare("SELECT _omaster, _oslave FROM object_connection;");
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  mMasterConnection.clear();
  mSlaveConnection.clear();

  while (q->next()) {
    int idM = q->value(0).toInt();
    int idS = q->value(1).toInt();
    mMasterConnection.insertMulti(idS, idM);
    mSlaveConnection.insertMulti(idM, idS);
  }
  return true;
}

bool ObjectTable::IsPreDefault(int id)
{
  return id <= 100;
}

bool ObjectTable::IsDefault(int id)
{
  if (mMasterConnection.isEmpty()) {
    if (!ReloadConnections()) {
      return false;
    }
  }

  auto itr = mMasterConnection.find(id);
  if (itr != mMasterConnection.end() && itr.value() == 0) {
    return true;
  }
  return false;
}

bool ObjectTable::UpdateVersion(int objectId, const QString& version, bool updateRevision)
{
  auto q = mDb.MakeQuery();
  if (updateRevision) {
    q->prepare(QString("UPDATE object SET version = ?, revision = revision + 1 WHERE _id = %1;").arg(objectId));
  } else {
    q->prepare(QString("UPDATE object SET version = ? WHERE _id = %1;").arg(objectId));
  }
  q->bindValue(0, version);
  return mDb.ExecuteNonQuery(q);
}

bool ObjectTable::UpdateUri(int objectId, const QString& uri)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("UPDATE object SET uri = ? WHERE _id = %1;").arg(objectId));
  q->bindValue(0, uri);
  return mDb.ExecuteNonQuery(q);
}

bool ObjectTable::CreateObject(int templId, const QString& name, int* id)
{
  return CreateObject(templId, name, QUuid::createUuid().toString(), id);
}

bool ObjectTable::CreateObject(int templId, const QString& name, const QString& guid, int* id)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT object_create(?, ?, ?, '');"));
  int index = 0;
  q->bindValue(index++, templId);
  q->bindValue(index++, guid);
  q->bindValue(index++, name);

  bool result = mDb.ExecuteQuery(q);
  if (id && result && q->next()) {
    *id = q->value(0).toInt(&result);
  }
  return result;
}

bool ObjectTable::CreateSlaveObject(int masterId, int templId, const QString& name, int* id)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT object_create_slave(?, ?, ?, ?, '');"));
  int index = 0;
  q->bindValue(index++, masterId);
  q->bindValue(index++, templId);
  q->bindValue(index++, QUuid::createUuid().toString());
  q->bindValue(index++, name);

  bool result = mDb.ExecuteQuery(q);
  if (id && result && q->next()) {
    *id = q->value(0).toInt(&result);
  }
  return result;
}

bool ObjectTable::UpdateState(ObjectItem* item, int state)
{
  bool ok = UpdateState(item->Id, state);
  if (ok) {
    item->Status = state;
  }
  return ok;
}

bool ObjectTable::UpdateState(int id, int state)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("UPDATE object SET status = %2 WHERE _id = %1").arg(id).arg(state));
  return mDb.ExecuteNonQuery(q);
}

bool ObjectTable::UpdateRevisions(const QString& where)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("UPDATE object SET revision = revision+1 ") + where);
  return mDb.ExecuteNonQuery(q);
}

bool ObjectTable::GetObjectRevision(int id, int* revision)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT revision FROM object WHERE _id = %1").arg(id));
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }
  if (revision) {
    if (q->next()) {
      *revision = q->value(0).toInt();
    } else {
      *revision = 0;
    }
  }
  return true;
}

bool ObjectTable::GetObjectByGuid(const QString& guid, ObjectItemS& obj)
{
  if (!LoadWhere(QString(" WHERE guid=%1").arg(ToSql(guid)))) {
    return false;
  }
  if (mItems.size() > 1) {
    Log.Warning(QString("Get more then one object from GUID (count: %1)").arg(mItems.size()));
    return false;
  }

  if (!mItems.isEmpty()) {
    obj = mItems.first().staticCast<ObjectItem>();
  } else {
    obj.reset();
  }
  return true;
}

bool ObjectTable::GetObjectIdByGuid(const QString& guid, int& id)
{
  if (!LoadWhere(QString(" WHERE guid=%1").arg(ToSql(guid)))) {
    return false;
  }
  if (mItems.size() > 1) {
    Log.Warning(QString("Get more then one object from GUID (count: %1)").arg(mItems.size()));
    return false;
  }

  if (!mItems.isEmpty()) {
    id = mItems.first()->Id;
  } else {
    id = 0;
  }
  return true;
}

bool ObjectTable::GetDefaultTemplateObjectId(const QString& typeName, int* id)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT o._id FROM object_connection c"
                     " INNER JOIN object o ON o._id = c._oslave"
                     " INNER JOIN object_type t ON o._otype = t._id"
                     " WHERE c._omaster = 0 AND t.name = ?"
                     " LIMIT 1;"));
  q->bindValue(0, typeName);
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }
  if (id) {
    if (q->next()) {
      *id = q->value(0).toInt();
    } else {
      *id = 0;
    }
  }
  return true;
}

bool ObjectTable::CreateLink(int masterId, int slaveId)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("INSERT INTO object_connection(_omaster, _oslave) VALUES (%1, %2);").arg(masterId).arg(slaveId));
  return mDb.ExecuteNonQuery(q);
}

bool ObjectTable::RemoveLink(int masterId, int slaveId)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("DELETE FROM object_connection WHERE _omaster = %1 AND _oslave = %2;").arg(masterId).arg(slaveId));
  return mDb.ExecuteNonQuery(q);
}

bool ObjectTable::RemoveLink(int id)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("DELETE FROM object_connection WHERE _id = %1;").arg(id));
  return mDb.ExecuteNonQuery(q);
}

bool ObjectTable::RemoveSlaves(int masterId)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("DELETE FROM object_connection WHERE _omaster = %1;").arg(masterId));
  return mDb.ExecuteNonQuery(q);
}

bool ObjectTable::ClearAllWithType(int typeId)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("WITH l AS (SELECT o._id FROM object o"
                     " LEFT JOIN object_connection c ON c._oslave = o._id"
                     " WHERE o._otype = %1 AND c._id IS NULL)"
                     " DELETE FROM object o USING l WHERE o._id = l._id;").arg(typeId));
  return mDb.ExecuteNonQuery(q);
}

bool ObjectTable::LoadObjectRevisions(const QString& objTypeIdList, QList<QPair<int, int> >& revisions)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT o._id, o.revision FROM object o"
                     " LEFT JOIN object_connection oc ON oc._oslave = o._id WHERE o._otype IN (%1) AND oc._id IS NULL").arg(objTypeIdList));
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }
  while (q->next()) {
    int id = q->value(0).toInt();
    int rev = q->value(1).toInt();
    revisions.append(qMakePair(id, rev));
  }
  return true;
}

bool ObjectTable::LoginDb(const QString& type, const QString& name, const QString& guid, ObjectItemS& obj)
{
  if (!GetObjectByGuid(guid, obj)) {
    return false;
  }
  if (!obj) {
    ObjectItemS templArmObject;
    if (!GetObjectByGuid(type, templArmObject)) {
      return false;
    }
    if (!templArmObject) {
      return false;
    }
    Log.Info(QString("Create new '%1' (name: '%2', guid: '%3')").arg(type).arg(name).arg(guid));
    if (!CreateObject(templArmObject->Id, name, guid)) {
      return false;
    }
    if (!GetObjectByGuid(guid, obj) || !obj) {
      return false;
    }
  }
  Log.Info(QString("Login '%1' (id: %2, name: '%3', guid: '%4')").arg(type).arg(obj->Id).arg(obj->Name).arg(obj->Guid));
  return true;
}


ObjectTable::ObjectTable(const Db &_Db)
  : TableNamed(_Db)
{
}
