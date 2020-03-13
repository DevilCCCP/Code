#include <QSharedMemory>

#include <Lib/Log/Log.h>
#include <Local/ModuleNames.h>

#include "FrameChannel.h"


const int kInnerBufferLimit = 68;

void FrameChannel::Push(const Frame::Header& frameHeader, const char* data)
{
  if (!InitInfoShmem(false)) {
    return PushInner(frameHeader, data);
  }

  if (!mDataMaxSize) {
    if (frameHeader.Key) {
      mDataMaxSize = 2 * frameHeader.Size;
      Log.Info(QString("Max shmem size set to %1").arg(mDataMaxSize));
    } else {
      return PushInner(frameHeader, data);
    }
  }

  if (mDataMaxSize < frameHeader.Size) {
    Log.Info(QString("Max shmem size enlarged to %1").arg(mDataMaxSize));
    mDataMaxSize = 2 * frameHeader.Size;
  }

  while (!mInnerFrames.isEmpty()) {
    int nextSize = mInnerFrames.first().size();
    if (!FindWriteShmem(nextSize)) {
      return PushInner(frameHeader, data);
    }
    WriteDataShmem(mInnerFrames.first());
    mInnerFrames.removeFirst();
  }

  if (FindWriteShmem(frameHeader.Size)) {
    WriteDataShmem(frameHeader, data);
  } else {
    PushInner(frameHeader, data);
  }
}

bool FrameChannel::Pop(const FrameS& frame)
{
  if (!InitInfoShmem(true)) {
    return false;
  }

  if (!FindReadShmem()) {
    return false;
  }

  ReadDataShmem(frame);
  return true;
}

bool FrameChannel::FindWriteShmem(int size)
{
  for (mCurrentSlot = 0; mCurrentSlot < ChannelInfo::kMaxDataShmem; mCurrentSlot++) {
    mCurrentPool = &mChannelInfo->Pool[mCurrentSlot];
    if (mCurrentPool->State != ChannelInfo::eEmpty) {
      continue;
    }

    if (mCurrentPool->Index == 0) {
      CreateNewDataShmem();
    } else {
      auto itr = mDataShmemMap.find(mCurrentPool->Index);
      if (itr == mDataShmemMap.end()) {
        if (!AttachDataShmem()) {
          return false;
        }
      } else {
        mCurrentDataShmem = itr.value().data();
      }
    }

    if (mCurrentDataShmem->size() < size) {
      CreateNewDataShmem();
    }
    return true;
  }

  return false;
}

bool FrameChannel::FindReadShmem()
{
  for (mCurrentSlot = 0; mCurrentSlot < ChannelInfo::kMaxDataShmem; mCurrentSlot++) {
    mCurrentPool = &mChannelInfo->Pool[mCurrentSlot];
    if (mCurrentPool->State != ChannelInfo::eFull) {
      continue;
    }

    auto itr = mDataShmemMap.find(mCurrentPool->Index);
    if (mCurrentPool->Index == 0) {
      Log.Warning(QString("Empty lost data shmem (slot: %1, channel: %2)")
                  .arg(mCurrentSlot).arg(mChannelId));
      mCurrentPool->State = ChannelInfo::eEmpty;
      continue;
    } else if (itr != mDataShmemMap.end()) {
      mCurrentDataShmem = itr.value().data();
    } else {
      if (!AttachDataShmem()) {
        continue;
      }
    }
    return true;
  }

  return false;
}

void FrameChannel::PushInner(const Frame::Header& frameHeader, const char* data)
{
  QByteArray frameData;
  frameData.resize(frameHeader.Size);
  memcpy(frameData.data(), &frameHeader, sizeof(Frame::Header));
  memcpy(frameData.data() + sizeof(Frame::Header), data, frameHeader.Size - sizeof(Frame::Header));

  mInnerFrames.append(frameData);
  if (mInnerFrames.size() > kInnerBufferLimit) {
    int oldSize = mInnerFrames.size();
    while (mInnerFrames.size() > kInnerBufferLimit) {
      mInnerFrames.removeFirst();
    }
    while (!mInnerFrames.isEmpty()) {
      const QByteArray& fData = mInnerFrames.takeFirst();
      const Frame::Header* fHeader = (const Frame::Header*)fData.constData();
      if (fHeader->Key) {
        break;
      }
      mInnerFrames.removeFirst();
    }
    int newSize = mInnerFrames.size();
    Log.Warning(QString("Channel %1 remove frames (cnt: %2, old: %3, new: %4)")
                .arg(mChannelId).arg(oldSize - newSize).arg(oldSize).arg(newSize));
  }
}

void FrameChannel::WriteDataShmem(const Frame::Header& frameHeader, const char* data)
{
  char* shmemData = (char*)mCurrentDataShmem->data();
  memcpy(shmemData, (const char*)&frameHeader, sizeof(Frame::Header));
  memcpy(shmemData + sizeof(Frame::Header), data, frameHeader.Size - sizeof(Frame::Header));
  mCurrentPool->Size = frameHeader.Size;
  mCurrentPool->State = ChannelInfo::eFull;
}

void FrameChannel::WriteDataShmem(const QByteArray& frameData)
{
  Q_ASSERT(((const Frame::Header*)frameData.constData())->Size == frameData.size());

  char* shmemData = (char*)mCurrentDataShmem->data();
  memcpy(shmemData, frameData.constData(), frameData.size());
  mCurrentPool->Size = frameData.size();
  mCurrentPool->State = ChannelInfo::eFull;
}

void FrameChannel::ReadDataShmem(const FrameS& frame)
{
  frame->ReserveData(mCurrentPool->Size - sizeof(Frame::Header));
  memcpy(frame->Data(), mCurrentDataShmem->data(), mCurrentPool->Size);

  Q_ASSERT(mCurrentPool->Size == frame->GetHeader()->Size);
  mCurrentPool->State = ChannelInfo::eEmpty;
}

bool FrameChannel::InitInfoShmem(bool readOnly)
{
  if (mChannelInfo) {
    return mChannelInfo->Magic == ChannelInfo::kMagic;
  }

  if (!readOnly && mInfoShmem->create(sizeof(ChannelInfo))) {
    Log.Info(QString("Created info shmem (channel: %1, name: '%2')").arg(mChannelId).arg(mInfoShmem->key()));
    mChannelInfo = static_cast<ChannelInfo*>(mInfoShmem->data());
    memset(mChannelInfo, 0, sizeof(ChannelInfo));
    mChannelInfo->Magic = ChannelInfo::kMagic;
    return true;
  } else if (mInfoShmem->attach()) {
    Log.Info(QString("Attached info shmem (channel: %1, name: '%2')").arg(mChannelId).arg(mInfoShmem->key()));
    mChannelInfo = static_cast<ChannelInfo*>(mInfoShmem->data());
    if (mChannelInfo->Magic == ChannelInfo::kMagic) {
      if (!readOnly) {
        for (mCurrentSlot = 0; mCurrentSlot < ChannelInfo::kMaxDataShmem; mCurrentSlot++) {
          mCurrentPool = &mChannelInfo->Pool[mCurrentSlot];
          if (mCurrentPool->Index) {
            AttachIfExistsDataShmem();
          }
        }
      }
      return true;
    }
    if (!readOnly) {
      Log.Fatal(QString("Attached bad info shmem (channel: %1, name: '%2')").arg(mChannelId).arg(mInfoShmem->key()), true);
    }
  }
  return false;
}

void FrameChannel::CreateNewDataShmem()
{
  int index = mChannelInfo->CreateIndex + 1;
  QSharedMemoryS shmem(mCurrentDataShmem = new QSharedMemory(GetDataName(index)));
  if (!shmem->create(mDataMaxSize)) {
    Log.Fatal(QString("Create data shmem fail (slot: %1, channel: %2, name: '%3')")
              .arg(mCurrentSlot).arg(mChannelId).arg(shmem->key()), true);
  }
  Log.Info(QString("Created data shmem (slot: %1, channel: %2, name: '%3')")
           .arg(mCurrentSlot).arg(mChannelId).arg(shmem->key()));
  mCurrentPool->Index = index;
  mChannelInfo->CreateIndex = index;
  mDataShmemMap[index] = shmem;
}

bool FrameChannel::AttachDataShmem()
{
  int index = mCurrentPool->Index;
  QSharedMemoryS shmem(mCurrentDataShmem = new QSharedMemory(GetDataName(index)));
  if (!shmem->attach()) {
    mCurrentPool->Index = 0;
    Log.Warning(QString("Attach data shmem fail (slot: %1, channel: %2, name: '%3')")
              .arg(mCurrentSlot).arg(mChannelId).arg(shmem->key()));
    return false;
  }
  Log.Info(QString("Attached data shmem (slot: %1, channel: %2, name: '%3')")
           .arg(mCurrentSlot).arg(mChannelId).arg(shmem->key()));
  mDataShmemMap[index] = shmem;
  return true;
}

bool FrameChannel::AttachIfExistsDataShmem()
{
  int index = mCurrentPool->Index;
  QSharedMemoryS shmem(mCurrentDataShmem = new QSharedMemory(GetDataName(index)));
  if (!shmem->attach()) {
    mCurrentPool->Index = 0;
    Log.Warning(QString("Lost data shmem (slot: %1, channel: %2, name: '%3')")
                .arg(mCurrentSlot).arg(mChannelId).arg(shmem->key()));
    return false;
  }
  Log.Info(QString("Attached data shmem (slot: %1, channel: %2, name: '%3')")
           .arg(mCurrentSlot).arg(mChannelId).arg(shmem->key()));
  mDataShmemMap[index] = shmem;
  return true;
}

QString FrameChannel::GetInfoName()
{
  return QString("%1_channel_info_%2").arg(kVideo).arg(mChannelId);
}

QString FrameChannel::GetDataName(int index)
{
  return QString("%1_channel_data_%2_%3").arg(kVideo).arg(mChannelId).arg(index);
}


FrameChannel::FrameChannel(int _ChannelId)
  : mChannelId(_ChannelId), mInfoShmem(new QSharedMemory(GetInfoName())), mChannelInfo(nullptr), mDataMaxSize(0)
{
}

