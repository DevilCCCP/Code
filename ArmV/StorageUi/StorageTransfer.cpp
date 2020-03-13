#include <LibV/Storage/Container.h>
#include <Lib/Log/Log.h>

#include "StorageTransfer.h"


void StorageTransfer::run()
{
  foreach (const ContInfo& contInfo, mContInfoSources) {
    ContainerS container(new Container(contInfo.Path, contInfo.CellSize, contInfo.CellPageSize, contInfo.Capacity));
    mContainerSources.append(container);
  }

  mContainerDest.reset(new Container(mContInfoDest.Path, mContInfoDest.CellSize, mContInfoDest.CellPageSize, mContInfoDest.Capacity));

  mSuccess = Transfer();
  if (mSuccess) {
    Log.Info(QString("Transfer done"));
  } else {
    Log.Warning(QString("Transfer fail ('%1')").arg(mErrorString));
  }

  mContainerSources.clear();
}

void StorageTransfer::Stop()
{
  mStop = true;
}

bool StorageTransfer::Transfer()
{
  if (!mContainerDest->Create()) {
    mErrorString = QString("Create dest container fail");
    return false;
  }

  foreach (const ContainerS& container, mContainerSources) {
    if (!container->ConnectDig()) {
      mErrorString = QString("Connect source container fail");
      return false;
    }
  }

  QElapsedTimer timer;
  timer.start();
  Log.Info(QString("Transfer started"));
  QByteArray cellData;
  cellData.resize(mContInfoDest.CellSize);
  for (int i = 0; i < mCellsVector.size(); i++) {
    if (mStop) {
      mErrorString = QString("Transfer stopped");
      return false;
    }
    const CellInfoEx& cellInfo = mCellsVector.at(i);
    if (!mContainerSources[cellInfo.SourceId]->ExportCell(cellInfo.Id, cellInfo.UnitExportId, cellData)) {
      mErrorString = QString("Read storage '%1' cell %2 fail").arg(mContInfoSources.at(cellInfo.SourceId).Path).arg(cellInfo.Id);
      return false;
    }
    if (!mContainerDest->ImportCell(i + 1, cellInfo.UnitImportId, cellData)) {
      mErrorString = QString("Write dest storage '%1' cell %2 fail").arg(mContInfoDest.Path).arg(i);
      return false;
    }

    if (timer.elapsed() > 10) {
      timer.start();
      emit PercentChanged(i + 1);
    }
  }

  return true;
}

StorageTransfer::StorageTransfer(const QVector<ContInfo>& _ContInfoSources, const ContInfo& _ContInfoDest, const CellsVector& _CellsVector, QObject* parent)
  : QThread(parent)
  , mContInfoSources(_ContInfoSources), mContInfoDest(_ContInfoDest), mCellsVector(_CellsVector)
  , mStop(false)
{
}

