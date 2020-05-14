#include <Lib/Log/Log.h>

#include "ObjectState.h"


const int kUpdateTimeMs = 1000;


bool ObjectStateValuesItem::Equals(const TableItem &other) const
{
  const ObjectStateValuesItem& other_ = static_cast<const ObjectStateValuesItem&>(other);
  return Id == other_.Id && Name == other_.Name && Descr == other_.Descr
      && Type == other_.Type && State == other_.State;
}

const char *ObjectStateValuesTable::Name()
{
  return "object_state_values";
}

const char *ObjectStateValuesTable::Select()
{
  return "SELECT _id, color, descr, _ostype, state FROM object_state_values";
}

bool ObjectStateValuesTable::OnRowFillItem(QueryS &q, TableItemS &unit)
{
  unit = TableItemS(new ObjectStateValuesItem());
  ObjectStateValuesItem* item = static_cast<ObjectStateValuesItem*>(unit.data());
  item->Color = q->value(1).toString();
  item->Descr = q->value(2).toString();
  item->Type = q->value(3).toInt();
  item->State = q->value(4).toInt();
  return true;
}

void ObjectStateValuesTable::CreateIndexes()
{
  mTypeStateIndex.clear();
  for (auto itr = mItems.begin(); itr != mItems.end(); itr++) {
    const ObjectStateValuesItem* item = static_cast<const ObjectStateValuesItem*>(itr.value().data());
    mTypeStateIndex.insert(qMakePair(item->Type, item->State), item);
  }
}

void ObjectStateValuesTable::ClearIndexes()
{
  mTypeStateIndex.clear();
}

const ObjectStateValuesItem *ObjectStateValuesTable::GetItemByTypeState(int type, int state)
{
  auto itr = mTypeStateIndex.find(qMakePair(type, state));
  if (itr != mTypeStateIndex.end()) {
    return itr.value();
  }
  return nullptr;
}

ObjectStateValuesTable::ObjectStateValuesTable(const Db& _Db)
  : TableNamed(_Db)
{
}

ObjectStateValuesTable::~ObjectStateValuesTable()
{
}


const char *ObjectStateTypeTable::Select()
{
  return "SELECT _id, name, descr FROM object_state_type;";
}

bool ObjectStateTypeTable::OnRowFillItem(QueryS &q, TableItemS &unit)
{
  unit = TableItemS(new ObjectStateTypeItem());
  return TableNamed::OnRowFillItem(q, unit);
}

ObjectStateTypeTable::ObjectStateTypeTable(const Db &_Db)
  : TableNamed(_Db)
{
}

ObjectStateTypeTable::~ObjectStateTypeTable()
{
}


bool ObjectStateItem::Equals(const TableItem &other) const
{
  const ObjectStateItem& other_ = static_cast<const ObjectStateItem&>(other);
  return Id == other_.Id && ObjectId == other_.ObjectId && ObjectStateTypeId == other_.ObjectStateTypeId
      && State == other_.State && ChangeSec == other_.ChangeSec;
}

const char *ObjectState::Name()
{
  return "object_state";
}

const char *ObjectState::Select()
{
  return "SELECT _id, _object, _ostype, state, extract(epoch from (now()-change_time))::int FROM object_state";
}

bool ObjectState::OnRowFillItem(QueryS &q, TableItemS &unit)
{
  unit = TableItemS(new ObjectStateItem());
  ObjectStateItem* item = static_cast<ObjectStateItem*>(unit.data());
  int index = 1;
  item->ObjectId = q->value(index++).toInt();
  item->ObjectStateTypeId = q->value(index++).toInt();
  item->State = q->value(index++).toInt();
  item->ChangeSec = q->value(index++).toInt();
  return true;
}

void ObjectState::CreateIndexes()
{
  mObjectIndex.clear();
  for (auto itr = mItems.begin(); itr != mItems.end(); itr++) {
    ObjectStateItem* item = static_cast<ObjectStateItem*>(itr.value().data());
    mObjectIndex[item->ObjectId] = item;
  }
}

void ObjectState::ClearIndexes()
{
  mObjectIndex.clear();
}

bool ObjectState::InitState(int _ObjectId, int _ObjectStateTypeId, int _DefaultState, int _NowState)
{
  if (mState) {
    Log.Warning("Init new state in one object");
  }
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT object_state_init(%1, %2, %3, %4);")
             .arg(_ObjectId).arg(_ObjectStateTypeId).arg(_DefaultState).arg(_NowState));
  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return false;
  }
  QVariant res = q->value(0);
  if (!res.isValid()) {
    return false;
  }

  int id = res.toInt();
  TableItemS item = GetItem(id);
  if (!item) {
    return false;
  }
  mState = dynamic_cast<ObjectStateItem*>(item.data());
  if (mAutoTimer) {
    mUpdateTimer.start();
  }
  return !!mState;
}

bool ObjectState::UpdateState(int newState)
{
  if (!mState) {
    LOG_WARNING_ONCE("update state fail, not init");
    return false;
  }
  if (mAutoTimer && mState->State == newState && mUpdateTimer.elapsed() < kUpdateTimeMs) {
    return true;
  }
  mState->State = newState;
  auto q = mDb.MakeQuery();
  q->prepare(QString("UPDATE object_state SET state = %1, change_time = NOW() WHERE _id = %2;")
             .arg(mState->State).arg(mState->Id));
  if (mDb.ExecuteNonQuery(q)) {
    if (mAutoTimer) {
      mUpdateTimer.restart();
    }
    return true;
  } else {
    return false;
  }
}

bool ObjectState::LoadLogCount(qint64& count)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT reltuples::bigint AS approximate_row_count FROM pg_class WHERE relname = 'object_state_log';"));
  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return false;
  }
  count = q->value(0).toLongLong();
  return true;
}

bool ObjectState::LoadLogCount(const QString& where, qint64& count)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT COUNT(_id) FROM object_state_log ") + where);
  if (!mDb.ExecuteQuery(q) || !q->next()) {
    return false;
  }
  count = q->value(0).toLongLong();
  return true;
}

ObjectStateItem* ObjectState::GetObjectState(int id)
{
  auto itr = mObjectIndex.find(id);
  if (itr != mObjectIndex.end()) {
    return itr.value();
  }
  return nullptr;
}


ObjectState::ObjectState(const Db& _Db, bool _AutoTimer)
  : Table(_Db)
  , mState(nullptr), mAutoTimer(_AutoTimer)
{
}

ObjectState::~ObjectState()
{
}

