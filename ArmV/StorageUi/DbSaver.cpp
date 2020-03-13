#include <LibV/Storage/Container.h>
#include <LibV/Storage/DbIndex.h>
#include <Lib/Log/Log.h>

#include "DbSaver.h"


void DbSaver::run()
{
  mContainer.reset(new Container(mContInfo.Path, mContInfo.CellSize, mContInfo.CellPageSize, mContInfo.Capacity));
  mDb.reset(new Db());
  mDbIndex.reset(new DbIndex(*mDb));

  mSuccess = CreateIndexes();
  if (mSuccess) {
    Log.Info(QString("CreateIndexes done"));
  } else {
    Log.Warning(QString("CreateIndexes fail ('%1')").arg(mErrorString));
  }

  mContainer.clear();
}

void DbSaver::Stop()
{
  mStop = true;
}

bool DbSaver::CreateIndexes()
{
  if (!mContainer->ConnectDig()) {
    mErrorString = QString("Connect container fail");
    return false;
  }
  if (!mDb->OpenFromFile(mDbConnection)) {
    mErrorString = QString("Open DB fail");
    return false;
  }
  if (!mDb->Connect()) {
    mErrorString = QString("Connect DB fail");
    return false;
  }
  if (!mDbIndex->Resize(mContInfo.Capacity)) {
    mErrorString = QString("Resize DB fail");
    return false;
  }

  QElapsedTimer timer;
  timer.start();
  Log.Info(QString("CreateIndexes started"));
  QMap<int, QPair<int, qint64> > lastIdMap;
  QPair<int, qint64> lastId = qMakePair(1, 0);
  for (int i = 1; i <= mContInfo.Capacity; i++) {
    int    unitId;
    qint64 startTime;
    qint64 endTime;
    int    condition;
    if (mContainer->OpenInfo(i, unitId, startTime, endTime, condition)) {
      mDbIndex->ConnectUnit(unitId);
      if (!mDbIndex->UpdateCell(i, QDateTime::fromMSecsSinceEpoch(startTime), QDateTime::fromMSecsSinceEpoch(endTime), condition)) {
        mErrorString = QString("Write to Db fail (cell: %1)").arg(i);
        return false;
      }
      QPair<int, qint64>& p = lastIdMap[unitId];
      if (startTime > p.second) {
        p.first  = i;
        p.second = startTime;
      }
      if (startTime > lastId.second) {
        lastId.first  = i;
        lastId.second = startTime;
      }
    } else {
      mDbIndex->ResetCell(i);
    }
    if (mStop) {
      mErrorString = QString("CreateIndexes stopped");
      return false;
    }

    if (timer.elapsed() > 10) {
      timer.start();
      emit PercentChanged(i + 1);
    }
  }

  int lastCell = lastId.first + 1;
  if (lastCell > mContInfo.Capacity) {
    lastCell = 1;
  }
  if (!mDbIndex->RepairCellInfo(0, lastCell, mContInfo.Capacity)) {
    mErrorString = QString("End write to Db fail");
    return false;
  }
  for (auto itr = lastIdMap.begin(); itr != lastIdMap.end(); itr++) {
    int unitId   = itr.key();
    int lastCell = itr.value().first;
    if (!mDbIndex->RepairCellInfo(unitId, lastCell, lastCell)) {
      mErrorString = QString("End write to Db fail");
      return false;
    }
  }
  return true;
}

DbSaver::DbSaver(const ContInfo _ContInfo, const QString& _DbConnection, QObject* parent)
  : QThread(parent)
  , mContInfo(_ContInfo), mDbConnection(_DbConnection)
  , mStop(false)
{
}

