#include <Lib/Log/Log.h>
#include <Lib/Settings/MemSettings.h>

#include "Storage.h"
#include "DbIndex.h"
#include "Container.h"
#include "Cell.h"

const int kSeekNearSecs = 15; // Ищем в текущей ячейке, либо в соседних, без обращения к таблице индексов

bool Storage::Create()
{
  if (!Prepare()) {
    Log.Warning("Prepare Db connection fail");
    return false;
  }
  return mDbIndex->Resize(mCapacity) && mContainer->Create();
}

bool Storage::Open(int _UnitId)
{
  if (!Prepare()) {
    return false;
  }
  mUnitId = _UnitId;
  mDbIndex->ConnectUnit(mUnitId);
  mCurrentCell = 0;
  mLastWriteTimestamp = 0;
  if (!mContainer->Connect(mUnitId)) {
    return false;
  }
  return true;
}

bool Storage::CloseWrite()
{
  if (!mCurrentCell) {
    return true;
  }

  qint64 startTime, endTime;
  if (mContainer->CloseWrite(startTime, endTime)) {
    return mDbIndex->UpdateCell(mCurrentCell, QDateTime::fromMSecsSinceEpoch(startTime)
                               , (endTime)? QDateTime::fromMSecsSinceEpoch(endTime): QDateTime(), eCellPartialSaved);
  } else {
    return false;
  }
}

void Storage::Reset()
{
  mCurrentCell = 0;
}

bool Storage::Prepare()
{
  if (mDb.IsConnected()) {
    return true;
  }
  if (!mDb.OpenFromFile(mDbConnectionFile)) {
    Log.Error(QString("Invalid db connection file '%1'").arg(mDbConnectionFile));
    return false;
  }
  return mDb.Connect();
}

bool Storage::WriteFrame(FrameS &frame)
{
  if (!mCurrentCell) {
    mLastWriteTimestamp = 0;
    if (!mDbIndex->GetCurrentCell(mCurrentCell)) {
      Log.Error(QString("Get current cell fail (id: %1)").arg(mUnitId));
      return false;
    }
    const DbIndexItem* item = static_cast<const DbIndexItem*>(mDbIndex->GetItem(mCurrentCell).data());
    if (!item) {
      Log.Error(QString("Get cell info fail (id: %1, cell: %2)").arg(mUnitId).arg(mCurrentCell));
      return false;
    }
    if (!item->StartTime.isValid()) {
      mNextCell = mCurrentCell;
      if (!mDbIndex->GetLastCell(mCurrentCell)) {
        return false;
      }
      if (mCurrentCell) {
        item = static_cast<const DbIndexItem*>(mDbIndex->GetItem(mCurrentCell).data());
        if (!item || !item->StartTime.isValid()) {
          Log.Error(QString("Get cell info fail (id: %1, cell: %2)").arg(mUnitId).arg(mCurrentCell));
          return false;
        }
      } else {
        mCurrentCell = mNextCell;
        mNextCell = 0;
      }
    }
    qint64 startTime = (item->Condition != eCellEmpty)? item->StartTime.toMSecsSinceEpoch(): frame->GetHeader()->Timestamp;

    if (!mContainer->OpenWrite(mCurrentCell, startTime)) {
      return false;
    }
    if (!mDbIndex->UpdateCell(mCurrentCell, QDateTime::fromMSecsSinceEpoch(startTime), QDateTime(), eCellWriting)) {
      return false;
    }
  }

  if (frame->GetHeader()->Timestamp <= mLastWriteTimestamp) {
    Log.Trace(QString("Timestamp of writing frame less or equals last, fixing"));
    frame->GetHeader()->Timestamp = mLastWriteTimestamp + 1;
  }
  mLastWriteTimestamp = frame->GetHeader()->Timestamp;

  if (!mContainer->CanWriteFrame(frame)) {
    if (!mNextCell) {
      if (!mDbIndex->GetNextCell(mNextCell)) {
        Log.Error(QString("Get next cell fail (id: %1)").arg(mUnitId));
        return false;
      }
    }
    qint64 startTime;
    qint64 endTime = frame->GetHeader()->Timestamp;
    if (!mContainer->CloseOpenNext(mNextCell, endTime, startTime)) {
      Log.Error(QString("Close cell fail (id: %1, cell: %2)").arg(mUnitId).arg(mCurrentCell));
      return false;
    }
    if (!mDbIndex->UpdateCell(mCurrentCell, QDateTime::fromMSecsSinceEpoch(startTime)
                             , QDateTime::fromMSecsSinceEpoch(endTime), eCellFullSaved)) {
      return false;
    }
    if (!mDbIndex->UpdateCell(mNextCell, QDateTime::fromMSecsSinceEpoch(endTime), QDateTime(), eCellWriting)) {
      return false;
    }
    mCurrentCell = mNextCell;
    mNextCell = 0;
  }

  if (!mContainer->WriteFrame(frame)) {
    Log.Warning(QString("Write frame fail (id: %1, cell: %2)").arg(mUnitId).arg(mCurrentCell));
    return false;
  }
  return true;
}

bool Storage::SeekFrame(const qint64& timestamp, bool& found, int maxDifferentMs, bool useKey)
{
  if (mCurrentCell > 0) {
    if (mContainer->SeekFrame(timestamp, found, maxDifferentMs, useKey) && found) {
      return true;
    }
  }
  QDateTime startTimestamp;
  if (!mDbIndex->FindCell(QDateTime::fromMSecsSinceEpoch(timestamp), mCurrentCell, startTimestamp)) {
    Log.Warning(QString("Find cell in db fail (ts: %1)").arg(QDateTime::fromMSecsSinceEpoch(timestamp).toString(Qt::ISODate)));
    return false;
  }

  if (!mContainer->OpenRead(mCurrentCell, startTimestamp.toMSecsSinceEpoch())) {
    Log.Warning(QString("Open cell fail (ts: %1, cell: %2)").arg(QDateTime::fromMSecsSinceEpoch(timestamp).toString(Qt::ISODate)).arg(mCurrentCell));
    return false;
  }
  return mContainer->SeekFrame(timestamp, found, maxDifferentMs, useKey);
}

bool Storage::ReadFrame(FrameS& frame, bool forward)
{
  return forward? ReadNextFrame(frame): ReadPrevFrame(frame);
}

bool Storage::ReadNextFrame(FrameS &frame)
{
  if (!mCurrentCell) {
    Log.Warning("Try read frame witout seek");
    return false;
  }

  return mContainer->ReadNextFrame(frame);
}

bool Storage::ReadPrevFrame(FrameS& frame)
{
  if (!mCurrentCell) {
    Log.Warning("Try read frame witout seek");
    return false;
  }

  return mContainer->ReadPrevFrame(frame);
}

SettingsAS Storage::CopySettings(const QString& _DbConnectionFile, SettingsA& _Settings)
{
  QString path = _Settings.GetValue("Path").toString();
  int cellSize = _Settings.GetValue("CellSize").toInt();
  int pageSize = _Settings.GetValue("PageSize").toInt();
  int capacity = _Settings.GetValue("Capacity").toInt();

  SettingsAS settings = SettingsAS(new MemSettings());
  settings->Open(".tmp");
  settings->SetValue("Path", path);
  settings->SetValue("CellSize", cellSize);
  settings->SetValue("PageSize", pageSize);
  settings->SetValue("Capacity", capacity);
  settings->SetValue("Connection", _DbConnectionFile);

  return settings;
}

QString Storage::ConnectionFile(int id)
{
  return QString("storage_%1").arg(id, 6, 10, QChar('0'));
}

int Storage::GetCellLimit()
{
  return mContainer->GetCellSize();
}

Storage::Storage(SettingsA &_Settings, const QString &_DbConnectionFile)
  : mDbConnectionFile(_DbConnectionFile)
  , mUnitId(0), mCurrentCell(0), mNextCell(0)
{
  if (mDbConnectionFile.isEmpty()) {
    mDbConnectionFile = _Settings.GetValue("Connection").toString();
  }
  QString path = _Settings.GetValue("Path").toString();
  int cellSize = _Settings.GetValue("CellSize").toInt();
  int pageSize = _Settings.GetValue("PageSize").toInt();
  mCapacity = _Settings.GetValue("Capacity").toInt();
  mContainer = ContainerS(new Container(path, cellSize, pageSize, mCapacity));
  mDbIndex = DbIndexS(new DbIndex(mDb));
}

Storage::~Storage()
{
}

