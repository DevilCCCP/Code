#include <LibV/Storage/Container.h>
#include <Lib/Log/Log.h>

#include "StorageScaner.h"


void StorageScaner::run()
{
  mContainer.reset(new Container(mContInfo.Path, mContInfo.CellSize, mContInfo.CellPageSize, mContInfo.Capacity));

  mCellInfoList.clear();
  Scan();

  mContainer.clear();
}

void StorageScaner::Stop()
{
  mStop = true;
}

bool StorageScaner::Scan()
{
  if (!mContainer->ConnectDig()) {
    Log.Warning(QString("Connect container fail"));
    return false;
  }

  QElapsedTimer timer;
  timer.start();
  Log.Info(QString("Scan started"));
  for (int i = 1; i <= mContInfo.Capacity; i++) {
    if (mStop) {
      Log.Info(QString("Scan stopped"));
      return false;
    }
    CellInfo cellInfo;
    if (mContainer->OpenAndScan(i, cellInfo.UnitId, cellInfo.StartTime)) {
      cellInfo.Id = i;
      mCellInfoList.append(cellInfo);
    }

    if (timer.elapsed() > 10) {
      timer.start();
      emit PercentChanged(i);
    }
  }

  Log.Info(QString("Scan done"));
  return true;
}


StorageScaner::StorageScaner(const ContInfo& _ContInfo, QObject* parent)
  : QThread(parent)
  , mContInfo(_ContInfo)
  , mStop(false)
{
}

