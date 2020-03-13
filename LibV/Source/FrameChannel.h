#pragma once

#include <QMap>

#include <LibV/Include/Frame.h>


DefineClassS(FrameChannel);
DefineClassS(QSharedMemory);

struct ChannelInfo {
  static const int kMagic = 0x06660666;
  static const int kMaxDataShmem = 8;
  enum EState {
    eEmpty = 0,
    eFull  = 1,
  };
  struct FrameData {
    int Index;
    int State;
    int Size;
  };

  int       Magic;
  int       CreateIndex;
  int       ReadIndex;
  int       WriteIndex;
  FrameData Pool[kMaxDataShmem];
};

class FrameChannel
{
  int                       mChannelId;
  QSharedMemoryS            mInfoShmem;
  ChannelInfo*              mChannelInfo;
  int                       mDataMaxSize;
  QMap<int, QSharedMemoryS> mDataShmemMap;
  QList<QByteArray>         mInnerFrames;

  int                       mCurrentSlot;
  ChannelInfo::FrameData*   mCurrentPool;
  QSharedMemory*            mCurrentDataShmem;

public:
  void Push(const Frame::Header& frameHeader, const char* data);
  bool Pop(const FrameS& frame);

private:
  bool FindWriteShmem(int size);
  bool FindReadShmem();

  void PushInner(const Frame::Header& frameHeader, const char* data);
  void WriteDataShmem(const Frame::Header& frameHeader, const char* data);
  void WriteDataShmem(const QByteArray& frameData);
  void ReadDataShmem(const FrameS& frame);

  bool InitInfoShmem(bool readOnly);
  void CreateNewDataShmem();
  bool AttachDataShmem();
  bool AttachIfExistsDataShmem();

  QString GetInfoName();
  QString GetDataName(int index);

public:
  FrameChannel(int _ChannelId);
};
