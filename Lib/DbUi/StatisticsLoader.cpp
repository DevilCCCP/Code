#include <QElapsedTimer>

#include <Lib/Db/ObjectState.h>
#include <Lib/Db/ObjectStateHours.h>
#include <Lib/Log/Log.h>

#include "StatisticsLoader.h"


const int kLogPackSize = 1000;
const int kUpdatePeriodMs = 100;

void StatisticsLoader::Load(int days, const QDateTime& from, const QDateTime& to)
{
  mDays = days;
  mStop = false;
  QElapsedTimer updateTimer;
  updateTimer.start();
  if (!PrepareLogs(from)) {
    return;
  }

  QString where = QString("WHERE change_time >= %1 AND change_time < %2").arg(ToSql(from)).arg(ToSql(to));
  mCountMax = 1;
  mObjectStateTable->LoadLogCount(where, mCountMax);

  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT _ostate, old_state, new_state, change_time FROM object_state_log %2 ORDER BY change_time, _id LIMIT %1 OFFSET ?")
             .arg(kLogPackSize).arg(where));
  for (qint64 totalCount = 0; !mStop; ) {
    q->addBindValue(totalCount);
    if (!mDb.ExecuteQuery(q)) {
      mResultsMap.clear();
      return;
    }
    int count = 0;
    for (; q->next(); count++) {
      int type     = q->value(0).toInt();
      int oldState = q->value(1).toInt();
      int newState = q->value(2).toInt();
      QDateTime ts = q->value(3).toDateTime();

      LoadOne(type, oldState, newState, ts);
    }

    totalCount += count;
    if (updateTimer.elapsed() >= kUpdatePeriodMs) {
      emit CountCanged(totalCount, mCountMax);
      updateTimer.start();
    }

    if (count < kLogPackSize) {
      break;
    }
  }

//  // Ends with last state
//  q->prepare(QString("SELECT _id, state, change_time FROM object_state"));
//  if (!mDb.ExecuteQuery(q)) {
//    mResultsMap.clear();
//    return;
//  }
//  while (q->next()) {
//    int type     = q->value(0).toInt();
//    int state    = q->value(1).toInt();
//    QDateTime ts = q->value(2).toDateTime();

//    LoadOne(type, state, ts);
//  }

  // Ends with min('to', now()) as newState
  q->prepare(QString("WITH s AS (SELECT _id, change_time FROM object_state"
                     " UNION"
                     " (SELECT _ostate, MIN(change_time) FROM object_state_log WHERE change_time > %1 GROUP BY _ostate)"
                     " ORDER BY _id, change_time)"
                     " SELECT _id, MIN(change_time) FROM s GROUP BY _id;").arg(ToSql(to)));
  if (!mDb.ExecuteQuery(q)) {
    mResultsMap.clear();
    return;
  }
  while (q->next()) {
    int type     = q->value(0).toInt();
    QDateTime ts = q->value(1).toDateTime();

    FinishOne(type, ts);
  }

//  for (auto itr = mCurrentMap.begin(); itr != mCurrentMap.end(); itr++) {
//    int            id = itr.key();
//    const State& stat = itr.value();
//    LoadOne(id, stat.NewState, 0, to);
//  }
}

void StatisticsLoader::LoadFast(int days, const QDateTime& from, const QDateTime& to)
{
  mDays = days;
  mStop = false;
  QElapsedTimer updateTimer;
  updateTimer.start();
  if (!PrepareFastLogs()) {
    return;
  }

  QString where = QString("WHERE triggered_hour >= %1 AND triggered_hour < %2").arg(ToSql(from)).arg(ToSql(to));
  mObjectStateHoursTable->SelectCount(where, mCountMax);

  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT _object, state_good, state_bad, triggered_hour FROM object_state_hours %2 ORDER BY triggered_hour, _id LIMIT %1 OFFSET ?")
             .arg(kLogPackSize).arg(where));
  for (qint64 totalCount = 0; !mStop; ) {
    q->addBindValue(totalCount);
    if (!mDb.ExecuteQuery(q)) {
      mResultsMap.clear();
      return;
    }
    int count = 0;
    for (; q->next(); count++) {
      int        id = q->value(0).toInt();
      int goodState = q->value(1).toInt();
      int  badState = q->value(2).toInt();
      QDateTime  ts = q->value(3).toDateTime();

      LoadFastOne(id, goodState, badState, ts);
    }

    totalCount += count;
    if (updateTimer.elapsed() >= kUpdatePeriodMs) {
      emit CountCanged(totalCount, mCountMax);
      updateTimer.start();
    }

    if (count < kLogPackSize) {
      break;
    }
  }

  // Add n/a
  for (auto itr = mResultsMap.begin(); itr != mResultsMap.end(); itr++) {
    StatisticsMap* statMap = &itr.value();
    for (auto itr = statMap->begin(); itr != statMap->end(); itr++) {
      Statistics* statistics = &itr.value();
      statistics->Gray = qMax((qint64)0, (qint64)(24*60*60*1000) * mDays - (statistics->Green + statistics->Red));
    }
  }
}

void StatisticsLoader::Stop()
{
  mStop = true;
}

bool StatisticsLoader::PrepareLogs(const QDateTime& from)
{
  mResultsMap.clear();
  if (!mDb.Connect()) {
    return false;
  }
  mObjectStateTable->Clear();
  if (!mObjectStateTable->Load()) {
    return false;
  }
  const QMap<int, TableItemS>& items = mObjectStateTable->GetItems();

  mCurrentMap.clear();
  for (auto itr = items.begin(); itr != items.end(); itr++) {
    const TableItemS& item = itr.value();
    const ObjectStateItem* objectStateItem = static_cast<const ObjectStateItem*>(item.data());
    mCurrentMap[objectStateItem->Id] = State(objectStateItem->ObjectId, from);
  }

  auto q = mDb.MakeQuery();
  q->prepare(QString("WITH t AS (SELECT _ostate, MAX(change_time) AS change_time"
                     " FROM object_state_log"
                     " WHERE change_time < %1 GROUP BY _ostate)\n"
                     " SELECT l._ostate, l.new_state FROM t JOIN object_state_log l ON l._ostate = t._ostate AND l.change_time = t.change_time;")
             .arg(ToSql(from)));
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }
  while (q->next()) {
    int     type = q->value(0).toInt();
    int newState = q->value(1).toInt();
    auto itr = mCurrentMap.find(type);
    if (itr != mCurrentMap.end()) {
      State* stats = &itr.value();
      stats->LastChange = from;
      stats->NewState   = newState;
    }
  }
  return true;
}

bool StatisticsLoader::PrepareFastLogs()
{
  mResultsMap.clear();
  if (!mDb.Connect()) {
    return false;
  }
  return true;
}

void StatisticsLoader::LoadOne(int type, int oldState, int newState, const QDateTime& ts)
{
  auto itr = mCurrentMap.find(type);
  if (itr == mCurrentMap.end()) {
    return;
  }

  State* stats = &itr.value();
  stats->NewState = newState;
  if (!stats->LastChange.isValid()) {
    stats->LastChange = ts;
    return;
  }

  for (QDateTime time = stats->LastChange; time < ts; ) {
    QDate refDay = time.date();
    QDate nextDay;
    if (mDays >= 28) {
      refDay = refDay.addDays(-(refDay.day() - 1));
      nextDay = refDay.addMonths(1);
    } else if (mDays >= 7) {
      refDay = refDay.addDays(-(refDay.dayOfWeek() - 1));
      nextDay = refDay.addDays(7);
    } else {
      nextDay = refDay.addDays(1);
    }
    QDateTime nextTs(nextDay, QTime(0, 0, 0));
    if (nextTs > ts) {
      nextTs = ts;
    }
    Statistics* statistics = &mResultsMap[stats->ObjectId][refDay];
    qint64 period = nextTs.toMSecsSinceEpoch() - time.toMSecsSinceEpoch();
    if (oldState > 0) {
      statistics->Green += period;
    } else if (oldState < 0) {
      statistics->Red += period;
    } else {
      statistics->Gray += period;
    }

    time = nextTs;
  }
  stats->LastChange = ts;
}

void StatisticsLoader::FinishOne(int type, const QDateTime& ts)
{
  auto itr = mCurrentMap.find(type);
  if (itr == mCurrentMap.end()) {
    return;
  }

  State* stats = &itr.value();
  if (stats->LastChange.isValid()) {
    int oldState = stats->NewState;
    LoadOne(type, oldState, 0, ts);
  }
}

void StatisticsLoader::LoadFastOne(int id, int goodState, int badState, const QDateTime& ts)
{
  QDate refDay = ts.date();
  if (mDays >= 28) {
    refDay = refDay.addDays(-(refDay.day() - 1));
  } else if (mDays >= 7) {
    refDay = refDay.addDays(-(refDay.dayOfWeek() - 1));
  }
  Statistics* statistics = &mResultsMap[id][refDay];
  statistics->Green += goodState;
  statistics->Red   += badState;
}


StatisticsLoader::StatisticsLoader(QObject* parent)
  : QObject(parent)
  , mCountMax(1), mStop(false)
{
  mDb.OpenDefault();
  mObjectStateTable.reset(new ObjectState(mDb));
  mObjectStateHoursTable.reset(new ObjectStateHoursTable(mDb));
}

