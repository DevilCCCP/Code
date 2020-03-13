#pragma once

#include <vector>
#include <QFile>
#include <QSystemSemaphore>

#include <QDateTime>
#include <Lib/Include/Common.h>
#include <LibV/Include/Frame.h>

#include "Cell.h"


/// Store format:
/// indexes in Db
/// cell[]
class Container
{
  enum EOpenMode {
    eNone,
    eWrite,
    eRead,
    eVerify,
    eIllegal
  };

  const QString     mPath;
  const int         mCellSize;
  const int         mCellPageSize;
  const int         mCapacity;

  // system vars
  //QSystemSemaphore        mFileSemaphore;
  QFile                   mFile;
  int                     mCurrentCell;
  bool                    mDigMode;

  // cell vars
  EOpenMode               mOpenMode;

  // reading
  int                     mReadFilePos;
  std::vector<char>       mReadBuffer;
  int                     mReadBufferPos;

  // cell info
  int                     mCellMagicX;
  std::vector<CellIndex>  mFrameIndexes;
  CellFrontHeader         mFrontHeader;
  CellBackHeader          mBackHeader;

  // writing
  int                     mWriteFilePos;
  std::vector<char>       mWriteBuffer;
  int                     mWriteBufferPos;

  // reading frames
  int                     mCurrentFrameIndex;

  class AccessFile {
    Container* mContainer;
  public:
    inline void Lock() { /*mContainer->mFileSemaphore.acquire();*/ }
    inline void Unlock() { mContainer->mFile.close(); /*mContainer->mFileSemaphore.release();*/ }
    inline AccessFile(Container* _Container): mContainer(_Container) { Lock(); }
    inline ~AccessFile() { Unlock(); }
  };

public:
  int GetCellSize() { return mCellSize; }

public:
  bool Connect(int unitId);
  bool ConnectDig();
  bool Create();

  bool OpenRead(int currentCell, const qint64 &startTime);
  bool OpenWrite(int currentCell, const qint64 &startTime);
  bool OpenRepair(int currentCell, const qint64 &startTime, int prevCell, const qint64 &prevStartTime);
  bool OpenAndScan(int currentCell, int& unitId, qint64& startTime);
  bool OpenInfo(int currentCell, int& unitId, qint64& startTime, qint64& endTime, int& condition);

  bool ExportCell(int currentCell, int unitId, QByteArray& data);
  bool ImportCell(int currentCell, int unitId, const QByteArray& data);

  bool CloseOpenNext(int nextCell, const qint64 &endTime, qint64 &startTime);
  bool CloseWrite(qint64 &startTime, qint64 &endTime);

private:
  bool OpenReadWrite(bool write);

  void SetXBit(int magic, bool reverce = false);
  bool RepairCell();
  bool OpenCell();
  bool FinalCell();

  template<typename StructT> bool ReadStruct(StructT** _struct)
  {
    char** data = reinterpret_cast<char**>(_struct);
    if (!Read(sizeof(StructT), data)) {
      return false;
    }
    return true;
  }
  template<typename StructT> bool WriteStruct(StructT& _struct)
  { return Write(reinterpret_cast<char*>(&_struct), sizeof(StructT)); }

  bool QueryRead(int size);
  bool Read(int size, char **data = nullptr);
  bool ReadSync(int read);
  bool QueryWrite(int size);
  bool Write(char* data, int size);
  bool WriteSync();

  int ReadDataPos();
  int WriteDataPos();
  int ReservedSpace();
  void PrepareRead(int pos = 0, bool verify = false);
  void PrepareWrite(int pos = 0);

public:
  bool CanWriteFrame(const FrameS &frame);
  bool WriteFrame(const FrameS &frame);
  bool SeekFrame(const qint64 &timestamp, bool &found, int maxDifferentMs, bool useKey);
  bool ReadNextFrame(FrameS &frame);
  bool ReadPrevFrame(FrameS &frame);

public:
  Container(const QString& _Path, int _CellSize, int _CellPageSize, int _Capacity);
  ~Container();
};

