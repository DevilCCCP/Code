#include <QMutexLocker>

#include <Lib/Db/ObjectType.h>
#include <Lib/Settings/DbSettings.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Log/Log.h>

#include "DvrA.h"
#include "FrameChannel.h"
#include "SourceState.h"


const int kChannelLimit = 128;

bool DvrA::DoInit()
{
  if (const NamedItem* item = mDb.getObjectTypeTable()->GetItemByName("aca")) {
    mAnalogTypeId = item->Id;
  } else {
    Log.Fatal(QString("Read analog camera type fail"), true);
  }

  QList<int> cameras;
  if (!mDb.getObjectTable()->LoadSlaves(GetOverseer()->Id(), cameras)) {
    Log.Fatal(QString("Load analog cameras fail"), true);
  }

  QSet<int> unorderedCameras;
  for (auto itr = cameras.begin(); itr != cameras.end(); itr++) {
    int id = *itr;
    DbSettings settings(mDb);
    if (!settings.Open(QString::number(id))) {
      Log.Fatal(QString("Read analog camera settings fail (id: %1)").arg(id), true);
    }
    int channel = settings.GetValue("Channel", 0).toInt();
    if (channel > 0 && channel <= kChannelLimit) {
      if (!mAnalogCameras.contains(channel)) {
        mAnalogCameras[channel] = Channel(id);
        continue;
      } else {
        Log.Warning(QString("Camera with channel already existed (cam: %1, chann: %2)").arg(id).arg(channel));
      }
    }
    unorderedCameras.insert(id);
  }

  for (auto itr = unorderedCameras.begin(); itr != unorderedCameras.end(); itr++) {
    int id = *itr;
    for (int channel = 1; channel <= kChannelLimit; channel++) {
      if (!mAnalogCameras.contains(channel)) {
        mAnalogCameras[channel] = Channel(id);
        break;
      }
    }
  }

  mSourceState = SourceStateS(new SourceState());
  if (!mQuiet) {
    mSourceState->Init(GetOverseer()->Id());
  }

  DbSettings settings(mDb);
  if (!settings.Open(QString::number(GetOverseer()->Id())) || !ReadSettings(&settings)) {
    Log.Fatal(QString("Read settings fail"), true);
  }

  for (auto itr = mAnalogCameras.begin(); itr != mAnalogCameras.end(); itr++) {
    int index = itr.key();
    const Channel& channel = itr.value();
    mChannels.append(index);
    Log.Info(QString("DVR channel: %1 -> camera: %2").arg(index).arg(channel.CameraId));
  }

  return Init();
}

bool DvrA::DoCircle()
{
  if (!mConnected) {
    ConnectDvr();
  } else {
    CheckDvr();
  }

  return true;
}

void DvrA::DoRelease()
{
  Release();
}

void DvrA::Stop()
{
  Imp::Stop();

  Disconnect();
}

bool DvrA::ReadSettings(SettingsA* settings)
{
  Q_UNUSED(settings);
  return true;
}

bool DvrA::Init()
{
  return true;
}

void DvrA::Release()
{
}

void DvrA::OnFrame(int channel, const Frame::Header& frameHeader, const char* data)
{
  QMutexLocker lock(&mMutex);
  auto itr = mAnalogCameras.find(channel);
  if (itr == mAnalogCameras.end()) {
    lock.unlock();
    LOG_WARNING_ONCE(QString("Not registered channel frame received (chann: %1)").arg(channel));
    return;
  }

  Channel* channelInfo = &itr.value();
  if (!channelInfo->FrameChan) {
    channelInfo->FrameChan.reset(new FrameChannel(channelInfo->CameraId));
  }
  channelInfo->FrameChan->Push(frameHeader, data);
  channelInfo->LastFrame.restart();
  SwitchStatus(Connection::eConnected);
  lock.unlock();

  SayWork();
}

void DvrA::ConnectDvr()
{
  OnStatus(Connection::eConnecting);

  if (Connect()) {
    mConnected = true;
  } else {
    OnStatus(Connection::eNotConnected);
  }
}

void DvrA::DisonnectDvr()
{
  OnStatus(Connection::eDisconnected);
  Disconnect();
  mConnected = false;
}

void DvrA::CheckDvr()
{
  bool disconnect = false;
  QMutexLocker lock(&mMutex);
  if (mStatus == Connection::eConnected) {
    disconnect = !CheckTimeout(kFramesLostTimeoutMs);
  } else if (mStatus == Connection::eConnecting) {
    disconnect = !CheckTimeout(kStreamOpenTimeoutMs);
  }
  lock.unlock();

  if (disconnect) {
    DisonnectDvr();
    if (IsAlive()) {
      ConnectDvr();
    }
  }
}

void DvrA::OnStatus(Connection::EStatus status)
{
  QMutexLocker lock(&mMutex);
  SwitchStatus(status);
}

void DvrA::ClearTimeout()
{
  for (auto itr = mAnalogCameras.begin(); itr != mAnalogCameras.end(); itr++) {
    Channel* channelInfo = &itr.value();
    channelInfo->LastFrame.restart();
  }
}

bool DvrA::CheckTimeout(int timeoutMs)
{
  for (auto itr = mAnalogCameras.begin(); itr != mAnalogCameras.end(); itr++) {
    Channel* channelInfo = &itr.value();
    if (channelInfo->LastFrame.elapsed() > timeoutMs) {
      return false;
    }
  }
  return true;
}

void DvrA::SwitchStatus(Connection::EStatus status)
{
  if (!mQuiet) {
    mSourceState->UpdateObjectState(status == Connection::eConnected);
  }

  if (mStatus != status) {
    mStatus = status;
    if (mStatus == Connection::eConnecting || mStatus == Connection::eConnected) {
      ClearTimeout();
    }
    Log.Info(QString("Source %1").arg(Connection::StatusToString(mStatus)));
  }
}


DvrA::DvrA(const Db& _Db, int _WorkPeriodMs)
  : Imp(_WorkPeriodMs, true, true)
  , mDb(_Db)
  , mStatus(Connection::eNone), mConnected(false)
{
}

DvrA::~DvrA()
{
}
