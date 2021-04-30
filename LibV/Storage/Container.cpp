#include <algorithm>

#include <Lib/Log/Log.h>
#include <Lib/Common/Format.h>

#include "Container.h"


const int kBlockSize = 512;


bool Container::Connect(int unitId)
{
  mBackHeader.UnitId = mFrontHeader.UnitId = unitId;
  mOpenMode = eNone;

  AccessFile lock(this);
  if (!mFile.open(QIODevice::ReadOnly)) {
    Log.Warning(QString("Storage '%1' access fail (error: %2)").arg(mPath).arg(mFile.errorString()));
    mOpenMode = eNone;
    return false;
  }
  qint64 size = mFile.size();
  return size >= mCellSize * mCapacity;
}

bool Container::ConnectDig()
{
  mDigMode = true;
  return Connect(-1);
}

bool Container::Create()
{
  mOpenMode = eNone;

  AccessFile lock(this);
  if (!mFile.open(QIODevice::WriteOnly)) {
    Log.Error(QString("Storage '%1' access fail (error: %2)").arg(mPath).arg(mFile.errorString()));
    return false;
  }
  if (!mFile.resize((qint64)mCellSize * mCapacity)) {
    Log.Error(QString("Storage '%1' resize fail (error: %2)").arg(mPath).arg(mFile.errorString()));
    return false;
  }
  qint64 size = mFile.size();
  if (size < mCellSize * mCapacity) {
    Log.Error(QString("File has wrong size (sz: %1, req: %2)").arg(FormatBytes(size)).arg(FormatBytes((qint64)mCellSize * mCapacity)));
  }
  return true;
}

bool Container::OpenRead(int currentCell, const qint64 &startTime)
{
  mCurrentCell = currentCell;

  mFrontHeader.StartTime = startTime;
  return OpenReadWrite(false);
}

bool Container::OpenWrite(int currentCell, const qint64 &startTime)
{
  mCurrentCell = currentCell;

  mFrontHeader.StartTime = startTime;
  mFrontHeader.PrevCell = 0;
  mFrontHeader.PrevStartTime = 0;
  return OpenReadWrite(true);
}

bool Container::OpenRepair(int currentCell, const qint64 &startTime, int prevCell, const qint64 &prevStartTime)
{
  mCurrentCell = currentCell;

  mFrontHeader.StartTime = startTime;
  mFrontHeader.PrevCell = prevCell;
  mFrontHeader.PrevStartTime = prevStartTime;
  return OpenReadWrite(true);
}

bool Container::OpenAndScan(int currentCell, int& unitId, qint64& startTime)
{
  mCurrentCell = currentCell;

  if (!OpenReadWrite(false)) {
    return false;
  }

  unitId = mFrontHeader.UnitId;
  startTime = mFrontHeader.StartTime;
  return true;
}

bool Container::OpenInfo(int currentCell, int& unitId, qint64& startTime, qint64& endTime, int& condition)
{
  mCurrentCell = currentCell;

  if (!OpenReadWrite(false)) {
    return false;
  }

  unitId    = mBackHeader.UnitId;
  startTime = mBackHeader.StartTime;
  endTime   = mBackHeader.EndTime;
  condition = mBackHeader.Condition;
  return true;
}

bool Container::ExportCell(int currentCell, int unitId, QByteArray& data)
{
  mCurrentCell = currentCell;
  data.resize(mCellSize);

  AccessFile lock(this);
  if (!mFile.open(QIODevice::ReadOnly)) {
    Log.Warning(QString("Storage '%1' read access fail (error: %2)").arg(mPath).arg(mFile.errorString()));
    return false;
  }
  if (!mFile.seek((qint64)(mCurrentCell - 1) * mCellSize)) {
    Log.Warning(QString("Storage '%1' seek fail (error: %2)").arg(mPath).arg(mFile.errorString()));
    return false;
  }

  if (mFile.read(data.data(), mCellSize) != mCellSize) {
    Log.Warning(QString("Storage '%1' read cell fail (error: %2)").arg(mPath).arg(mFile.errorString()));
    return false;
  }

  const CellFrontHeader* frontHeader = reinterpret_cast<const CellFrontHeader*>(data.data());
  if ((frontHeader->MagicA & kCellMagicFlag) != kCellMagicA) {
    Log.Warning(QString("Storage '%1' validate cell fail (magic A)").arg(mPath));
    return false;
  }
  if (frontHeader->UnitId != unitId) {
    Log.Warning(QString("Storage '%1' validate cell fail (unit id)").arg(mPath));
    return false;
  }
  return true;
}

bool Container::ImportCell(int currentCell, int unitId, const QByteArray& data)
{
  mCurrentCell = currentCell;
  if (data.size() != mCellSize) {
    Log.Warning(QString("Storage '%1' validate cell fail (size)").arg(mPath));
    return false;
  }
  const CellFrontHeader* frontHeader = reinterpret_cast<const CellFrontHeader*>(data.data());
  if ((frontHeader->MagicA & kCellMagicFlag) != kCellMagicA) {
    Log.Warning(QString("Storage '%1' validate cell fail (magic)").arg(mPath));
    return false;
  }
  if (unitId != frontHeader->UnitId) {
    CellFrontHeader* frontHeaderEdit = const_cast<CellFrontHeader*>(frontHeader);
    frontHeaderEdit->UnitId = unitId;
  }
  mCellMagicX = (frontHeader->MagicA & kCellMagicX);
  const CellBackHeader* backHeader = reinterpret_cast<const CellBackHeader*>(data.data() + mCellSize - sizeof(CellBackHeader));
  if (backHeader->MagicC == (kCellMagicC | mCellMagicX) && backHeader->MagicD == (kCellMagicD | mCellMagicX)) {
    if (backHeader->UnitId != unitId) {
      CellBackHeader* backHeaderEdit = const_cast<CellBackHeader*>(backHeader);
      backHeaderEdit->UnitId = unitId;
    }
  }

  AccessFile lock(this);
  if (!mFile.open(QIODevice::ReadWrite)) {
    Log.Warning(QString("Storage '%1' write access fail (error: %2)").arg(mPath).arg(mFile.errorString()));
    return false;
  }
  if (!mFile.seek((qint64)(mCurrentCell - 1) * mCellSize)) {
    Log.Warning(QString("Storage '%1' seek fail (error: %2)").arg(mPath).arg(mFile.errorString()));
    return false;
  }

  if (mFile.write(data) != mCellSize) {
    Log.Warning(QString("Storage '%1' write cell fail (error: %2)").arg(mPath).arg(mFile.errorString()));
    return false;
  }
  return true;
}

bool Container::CloseOpenNext(int nextCell, const qint64 &endTime, qint64 &startTime)
{
  if (mOpenMode != eWrite) {
    Log.Warning(QString("Container::CloseOpenNext wrong mode (mode: %1)").arg(mOpenMode));
    if (!RepairCell()) {
      return false;
    }
  }
  mBackHeader.Condition = eCellFullSaved;
  mBackHeader.NextCell = nextCell;
  mBackHeader.EndTime = endTime;
  startTime = mBackHeader.StartTime;
  if (!FinalCell()) {
    return false;
  }

  mFrontHeader.PrevCell = mCurrentCell;
  mFrontHeader.PrevStartTime = mFrontHeader.StartTime;
  mFrontHeader.StartTime = endTime;

  mCurrentCell = nextCell;

  return OpenReadWrite(true);
}

bool Container::CloseWrite(qint64& startTime, qint64& endTime)
{
  if (mOpenMode != eWrite) {
    Log.Warning(QString("Container::Close wrong mode (mode: %1)").arg(mOpenMode));
    if (!RepairCell()) {
      return false;
    }
  }
  mBackHeader.Condition = eCellPartialSaved;
  mBackHeader.NextCell = 0;
  mBackHeader.EndTime = (mFrameIndexes.empty())? 0: mFrameIndexes.back().Timestamp;
  startTime = mBackHeader.StartTime;
  endTime = mBackHeader.EndTime;
  return FinalCell();
}

bool Container::OpenReadWrite(bool write)
{
  mCellMagicX = 0;
  mFrameIndexes.clear();
  mBackHeader.StartTime = mFrontHeader.StartTime;

  PrepareRead(0, true);

  // validate front header
  CellFrontHeader* frontHeader;
  if (!ReadStruct(&frontHeader)) {
    return false;
  }
  bool broken = (frontHeader->MagicA & kCellMagicFlag) != kCellMagicA;
  bool illegal = frontHeader->UnitId != mFrontHeader.UnitId || qAbs(frontHeader->StartTime - mFrontHeader.StartTime) > 1000;
  if (illegal && mDigMode) {
    mBackHeader.UnitId = mFrontHeader.UnitId = frontHeader->UnitId;
    mFrontHeader.StartTime = mBackHeader.StartTime = frontHeader->StartTime;
    illegal = false;
  }

  if (write) {
    if (!broken && illegal) {
      // reverse x-bit for safe rewrite all data
      SetXBit(frontHeader->MagicA, true);
    }
    if (broken || illegal) {
      Log.Trace("Storage: open not legal, rewrite it");
      SetXBit(0);
      PrepareWrite();
      return WriteStruct(mFrontHeader);
    } else {
      SetXBit(frontHeader->MagicA);
      Log.Trace("Storage: open legal, open or repair");
      if (OpenCell() || RepairCell()) {
        PrepareWrite(mWriteFilePos);
        if (mWriteFilePos == 0) {
          return WriteStruct(mFrontHeader);
        } else {
          return true;
        }
      }
    }
  } else /*if (read)*/ {
    if (!broken && !illegal) {
      SetXBit(frontHeader->MagicA);
      mFrontHeader = *frontHeader;
      if (OpenCell()) {
        if (!mDigMode) {
          PrepareRead();
        } else {
          PrepareRead(sizeof(CellFrontHeader));
        }
        return true;
      }
    }
  }
  return false;
}

void Container::SetXBit(int magic, bool reverce)
{
  if (reverce) {
    mCellMagicX = (magic & kCellMagicX)? 0: kCellMagicX;
  } else {
    mCellMagicX = (magic & kCellMagicX);
  }
  mFrontHeader.MagicA = kCellMagicA | mCellMagicX;
  mBackHeader.MagicC = kCellMagicC | mCellMagicX;
  mBackHeader.MagicD = kCellMagicD | mCellMagicX;
}

bool Container::RepairCell()
{
  PrepareRead();
  if (!Read(sizeof(CellFrontHeader))) {
    return false;
  }

  mWriteFilePos = sizeof(CellFrontHeader);
  forever {
    CellFrameHeader* frameHeaderEx;
    Frame::Header* frameHeader;
    if (!ReadStruct(&frameHeaderEx) || !ReadStruct(&frameHeader)) {
      return false;
    }
    if (frameHeaderEx->MagicE != (kCellMagicE | mCellMagicX) || frameHeader->Size > mCellSize / 2) {
      break;
    }
    int size = frameHeader->Size - sizeof(Frame::Header);
    if (!QueryRead(size) || !Read(size)) {
      return false;
    }

    CellIndex index;
    index.Disp = ReadDataPos();
    index.Timestamp = frameHeader->Timestamp;
    index.Key = frameHeader->Key;
    mFrameIndexes.push_back(index);

    mWriteFilePos = index.Disp;
  }
  return true;
}

bool Container::OpenCell()
{
  PrepareRead(0, true);
  // validate back header
  int posBc = mCellSize - sizeof(CellBackHeader);
  CellBackHeader* backHeader;
  PrepareRead(posBc, true);
  if (!ReadStruct(&backHeader)) {
    return false;
  }
  if (backHeader->MagicC != (kCellMagicC | mCellMagicX) || backHeader->MagicD != (kCellMagicD | mCellMagicX)
      || backHeader->UnitId != mBackHeader.UnitId || qAbs(backHeader->StartTime - mBackHeader.StartTime) > 1000) {
    return false;
  }
  mBackHeader = *backHeader;

  int posInd = backHeader->FrameIndexDisp;
  if (posInd >= posBc || posInd < 0) {
    return false;
  }
  CellIndexHeader* indexHeader;
  PrepareRead(posInd, true);
  if (!ReadStruct(&indexHeader)) {
    return false;
  }
  if (indexHeader->MagicB != (kCellMagicB | mCellMagicX)) {
    return false;
  }
  CellIndex* frameIndex;
  for (int indexCount = (posBc - posInd) / sizeof(CellIndex); indexCount > 0; indexCount--) {
    if (!ReadStruct(&frameIndex)) {
      return false;
    }
    mFrameIndexes.push_back(*frameIndex);
    mWriteFilePos = frameIndex->Disp;
  }
  mCurrentFrameIndex = 0;
  return true;
}

bool Container::FinalCell()
{
  Log.Trace(QString("Final cell (count: %1, avg. fs: %2)").arg((int)mFrameIndexes.size()).arg((mCellSize - ReservedSpace())/mFrameIndexes.size()));
  if (!WriteSync()) {
    return false;
  }
  mBackHeader.FrameIndexDisp = mCellSize - ReservedSpace();
  PrepareWrite(mBackHeader.FrameIndexDisp);
  CellIndexHeader cellIndexHeader(mCellMagicX);
  if (!WriteStruct(cellIndexHeader)) {
    return false;
  }
  for (int i = 0; i < (int)mFrameIndexes.size(); i++) {
    if (!WriteStruct(mFrameIndexes[i])) {
      return false;
    }
  }
  if (!WriteStruct(mBackHeader)) {
    return false;
  }
  if (!WriteSync()) {
    return false;
  }
  if (WriteDataPos() != mCellSize) {
    Log.Error(QString("Close cell write pos wrong (pos: %1, right: %2)").arg(WriteDataPos()).arg(mCellSize));
    return false;
  }
  mFrameIndexes.clear();
  return true;
}

bool Container::QueryRead(int size)
{
  int dataPos = ReadDataPos();
  return dataPos + size <= mCellSize;
}

bool Container::Read(int size, char** data)
{
  if (mOpenMode != eRead && mOpenMode != eVerify) {
    Log.Warning(QString("Storage '%1' read fail, wrong mode (mode: %2)").arg(mPath).arg(mOpenMode));
    mOpenMode = eNone;
    return false;
  }

  if (mReadBufferPos + size > (int)mReadBuffer.size())
  {
    int blockSize = (mOpenMode == eRead)? mCellPageSize: kBlockSize;
    int readBlockSize = blockSize - (mReadFilePos % blockSize);
    if (readBlockSize < kBlockSize) {
      readBlockSize += blockSize;
    }
    while (mReadBufferPos + size > (int)mReadBuffer.size() + readBlockSize) {
      readBlockSize += kBlockSize;
    }
    if (mReadFilePos + readBlockSize > mCellSize) {
      readBlockSize = mCellSize - mReadFilePos;
      if (mReadBufferPos + size > (int)mReadBuffer.size() + readBlockSize) {
        Log.Error(QString("Storage '%1' read fail, out of cell (read size: %2)").arg(mPath).arg(size));
        return false;
      }
    }

    if (data == nullptr) {
      mReadBuffer.resize(0);
      mReadBufferPos = 0;
    } else if (mReadBufferPos >= 256) {
      int newSize = mReadBuffer.size() - mReadBufferPos;
      memcpy(mReadBuffer.data(), mReadBuffer.data() + mReadBufferPos, newSize);
      mReadBuffer.resize(newSize);
      mReadBufferPos = 0;
    }

    if (!ReadSync(readBlockSize)) {
      return false;
    }
  }

  if (data) {
    *data = mReadBuffer.data() + mReadBufferPos;
  }
  mReadBufferPos += size;
  return true;
}

bool Container::ReadSync(int read)
{
  AccessFile lock(this);
  if (!mFile.open(QIODevice::ReadOnly)) {
    Log.Warning(QString("Storage '%1' read access fail (error: %2)").arg(mPath).arg(mFile.errorString()));
    mOpenMode = eNone;
    return false;
  }
  if (!mFile.seek((qint64)(mCurrentCell - 1) * mCellSize + mReadFilePos)) {
    Log.Warning(QString("Storage '%1' seek fail (error: %2)").arg(mPath).arg(mFile.errorString()));
    mOpenMode = eNone;
    return false;
  }

  int pos = (int)mReadBuffer.size();
  mReadBuffer.resize(pos + read);
  if (mFile.read(mReadBuffer.data() + pos, read) != read) {
    Log.Warning(QString("Storage '%1' read %3 bytes fail (error: %2)").arg(mPath).arg(mFile.errorString()).arg(read));
    mOpenMode = eNone;
    return false;
  }
  mReadFilePos += read;
  return true;
}

bool Container::QueryWrite(int size)
{
  int dataPos = WriteDataPos();
  return dataPos + size + ReservedSpace() <= mCellSize;
}

bool Container::Write(char *data, int size)
{
  if (mOpenMode != eWrite) {
    Log.Warning(QString("Storage '%1' read fail, wrong mode (mode: %2)").arg(mPath).arg(mOpenMode));
    mOpenMode = eNone;
    return false;
  }

  int writeBlockSize = mCellPageSize - (mWriteFilePos % mCellPageSize);

  if (mWriteBufferPos + size > writeBlockSize)
  {
    int preSize = writeBlockSize - mWriteBufferPos;
    memcpy(mWriteBuffer.data() + mWriteBufferPos, data, preSize);
    mWriteBufferPos += preSize;
    data += preSize;
    size -= preSize;

    if (!WriteSync()) {
      return false;
    }
  }

  memcpy(mWriteBuffer.data() + mWriteBufferPos, data, size);
  mWriteBufferPos += size;
  return true;
}

bool Container::WriteSync()
{
  if (!mWriteBufferPos) {
    return true;
  }

  AccessFile lock(this);
  if (!mFile.open(QIODevice::ReadWrite)) {
    Log.Warning(QString("Storage '%1' write access fail (error: %2)").arg(mPath).arg(mFile.errorString()));
    mOpenMode = eNone;
    return false;
  }
  if (!mFile.seek((qint64)(mCurrentCell - 1) * mCellSize + mWriteFilePos)) {
    Log.Warning(QString("Storage '%1' seek fail (error: %2)").arg(mPath).arg(mFile.errorString()));
    mOpenMode = eNone;
    return false;
  }

  if (mFile.write(mWriteBuffer.data(), mWriteBufferPos) != mWriteBufferPos) {
    Log.Warning(QString("Storage '%1' write %3 bytes fail (error: %2)").arg(mPath).arg(mFile.errorString()).arg(mWriteBufferPos));
    mOpenMode = eNone;
    return false;
  }
  mWriteFilePos += mWriteBufferPos;
  mWriteBufferPos = 0;
  return true;
}

int Container::ReadDataPos()
{
  return mReadFilePos + mReadBufferPos - mReadBuffer.size();
}

int Container::WriteDataPos()
{
  return mWriteFilePos + mWriteBufferPos;
}

int Container::ReservedSpace()
{
  return sizeof(CellBackHeader) + sizeof(CellIndexHeader) + sizeof(CellIndex) * mFrameIndexes.size();
}

void Container::PrepareRead(int pos, bool verify)
{
  mReadFilePos = pos;
  mReadBufferPos = 0;
  mReadBuffer.resize(0);
  mOpenMode = verify? eVerify: eRead;
}

void Container::PrepareWrite(int pos)
{
  mWriteFilePos = pos;
  mWriteBufferPos = 0;
  mOpenMode = eWrite;
}

bool Container::CanWriteFrame(const FrameS &frame)
{
  return QueryWrite(frame->GetHeader()->Size + sizeof(CellIndex) + sizeof(CellFrameHeader));
}

bool Container::WriteFrame(const FrameS &frame)
{
  CellFrameHeader frameHeaderEx(mCellMagicX);
  if (!WriteStruct(frameHeaderEx)) {
    return false;
  }
  if (!Write(frame->Data(), frame->Size())) {
    return false;
  }

  Frame::Header* frameHeader = frame->GetHeader();
  CellIndex index;
  index.Disp = WriteDataPos();
  index.Timestamp = frameHeader->Timestamp;
  index.Key = frameHeader->Key;
  mFrameIndexes.push_back(index);
  return true;
}

bool Container::SeekFrame(const qint64 &timestamp, bool &found, int maxDifferentMs, bool useKey)
{
  for (int count = 0; count < 2; count ++)
  {
    auto itr = std::upper_bound(mFrameIndexes.begin(), mFrameIndexes.end(), timestamp);
    int upperIndex = (itr == mFrameIndexes.end())? (int)mFrameIndexes.size(): (int)(itr - mFrameIndexes.begin());
    for (int i = upperIndex - 1; i >= 0; i--) {
      CellIndex& index = mFrameIndexes[i];
      if (index.Timestamp <= timestamp && index.Timestamp + maxDifferentMs > timestamp) {
        if (!useKey || index.Key) {
          found = true;
          mCurrentFrameIndex = i;
          int framePos = (mCurrentFrameIndex > 0)? mFrameIndexes[mCurrentFrameIndex-1].Disp: sizeof(CellFrontHeader);
          PrepareRead(framePos);
          return true;
        } else {

        }
      } else {
        found = false;
        return true;
      }
    }
    Log.Trace("Key frame not found in this cell, switch prev");
    if (mFrontHeader.PrevCell == 0 || !OpenRead(mFrontHeader.PrevCell, mFrontHeader.PrevStartTime)) {
      return false;
    }
  }
  return false;
}

bool Container::ReadNextFrame(FrameS &frame)
{
  if (mCurrentFrameIndex >= (int)mFrameIndexes.size()) {
    if (mDigMode) {
      return false;
    }
    if (mBackHeader.Condition != eCellFullSaved) {
      Log.Trace("Reopen opened cell");
      int oldFrameIndex = mCurrentFrameIndex;
      if (OpenReadWrite(false) && oldFrameIndex < (int)mFrameIndexes.size()) {
        mCurrentFrameIndex = oldFrameIndex;
        int framePos = (mCurrentFrameIndex > 0)? mFrameIndexes[mCurrentFrameIndex-1].Disp: sizeof(CellFrontHeader);
        PrepareRead(framePos);
      } else {
        return false;
      }
    } else if (OpenRead(mBackHeader.NextCell, mBackHeader.EndTime)) {
      Log.Trace("Open next cell");
      PrepareRead(sizeof(CellFrontHeader));
      mCurrentFrameIndex = 0;
    } else {
      return false;
    }
  }

  CellFrameHeader* frameHeader;
  if (!ReadStruct(&frameHeader)) {
    return false;
  }
  if (frameHeader->MagicE != (kCellMagicE | mCellMagicX)) {
    // empty frame (frame may be rewritten)
    return true;
  }
  int size = mFrameIndexes[mCurrentFrameIndex].Disp - ReadDataPos();
  if (size < 0 || size > mCellSize / 2) {
    Log.Warning(QString("Read frame fail (cell: %1, index disp: %2, data pos: %3)")
                .arg(mCurrentCell).arg(mFrameIndexes[mCurrentFrameIndex].Disp).arg(ReadDataPos()));
    return false;
  }
  frame = FrameS(new Frame());
  char* data;
  if (!Read(size, &data)) {
    return false;
  }
  frame->AppendData(data, size);
  mCurrentFrameIndex++;
  return true;
}

bool Container::ReadPrevFrame(FrameS& frame)
{
  forever {
    if (mCurrentFrameIndex < 0) {
      if (OpenRead(mFrontHeader.PrevCell, mFrontHeader.PrevStartTime)) {
        Log.Trace("Open prev cell");
        mCurrentFrameIndex = (int)mFrameIndexes.size() - 1;
      } else {
        return false;
      }
    }

    if (mCurrentFrameIndex >= 0 && mFrameIndexes[mCurrentFrameIndex].Key) {
      break;
    }
    mCurrentFrameIndex--;
  }

  if (mCurrentFrameIndex > 0) {
    PrepareRead(mFrameIndexes[mCurrentFrameIndex - 1].Disp);
  } else {
    PrepareRead(sizeof(CellFrontHeader));
  }
  CellFrameHeader* frameHeader;
  if (!ReadStruct(&frameHeader)) {
    return false;
  }
  if (frameHeader->MagicE != (kCellMagicE | mCellMagicX)) {
    // empty frame (frame may be rewritten)
    return true;
  }
  int size = mFrameIndexes[mCurrentFrameIndex].Disp - ReadDataPos();
  if (size < 0 || size > mCellSize / 2) {
    Log.Warning(QString("Read frame fail (cell: %1, index disp: %2, data pos: %3)")
                .arg(mCurrentCell).arg(mFrameIndexes[mCurrentFrameIndex].Disp).arg(ReadDataPos()));
    return false;
  }
  frame = FrameS(new Frame());
  char* data;
  if (!Read(size, &data)) {
    return false;
  }
  frame->AppendData(data, size);

//  Log.Trace(QString("Read one (ts: %1, size: %2, key: %3)").arg(QDateTime::fromMSecsSinceEpoch(frame->GetHeader()->Timestamp).toString())
//    .arg(frame->Size()).arg((int)frame->GetHeader()->Key));

  mCurrentFrameIndex--;
  return true;
}


Container::Container(const QString &_Path, int _CellSize, int _CellPageSize, int _Capacity)
  : mPath(_Path), mCellSize(_CellSize), mCellPageSize(_CellPageSize), mCapacity(_Capacity)
  /*, mFileSemaphore(_Path, 1, QSystemSemaphore::Open)*/, mFile(_Path), mCurrentCell(0), mDigMode(false)
{
  mReadBuffer.reserve(mCellPageSize + kBlockSize);
  mWriteBuffer.resize(mCellPageSize);
}


Container::~Container()
{
}

