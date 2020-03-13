#include <QMutexLocker>

#include <Lib/Log/Log.h>

#include "MediaServer.h"
#include "Media.h"


bool MediaServer::DoCircle()
{
  QMutexLocker lock(&mMediaMutex);
  for (auto itr = mMedias.begin(); itr != mMedias.end(); ) {
    MediaS media = itr.value();
    if (!media->Test()) {
      itr = mMedias.erase(itr);
      media->Stop();
    } else {
      itr++;
    }
  }
  return true;
}

void MediaServer::OnRegisterMedia(const MediaS& media)
{
  Q_UNUSED(media);
}

void MediaServer::OnUnregisterMedia(const MediaS& media)
{
  Q_UNUSED(media);
}

bool MediaServer::AddMedia(const MediaS& media)
{
  const QString& mediaId = media->Id();
  QMutexLocker lock(&mMediaMutex);
  auto itr = mMedias.find(mediaId);
  if (itr != mMedias.end()) {
    Log.Warning(QString("Add media fail, already exists (Id: '%1')").arg(mediaId));
    return false;
  }

  mMedias[mediaId] = media;
  lock.unlock();
  OnRegisterMedia(media);
  Log.Info(QString("Add media (Id: '%1')").arg(mediaId));
  return true;
}

bool MediaServer::RemoveMedia(const QString& mediaId, bool warning)
{
  QMutexLocker lock(&mMediaMutex);
  auto itr = mMedias.find(mediaId);
  if (itr != mMedias.end()) {
    MediaS media = itr.value();
    mMedias.erase(itr);
    lock.unlock();
    OnUnregisterMedia(media);
    media->Stop();
    Log.Info(QString("Remove media (Id: '%1')").arg(mediaId));
    return true;
  } else {
    lock.unlock();
    if (warning) {
      Log.Warning(QString("Remove media fail, not exists (Id: '%1')").arg(mediaId));
    }
    return false;
  }
}

MediaS MediaServer::FindMedia(const QString& mediaId)
{
  QMutexLocker lock(&mMediaMutex);
  auto itr = mMedias.find(mediaId);
  if (itr != mMedias.end()) {
    return itr.value();
  }
  return MediaS();
}


MediaServer::MediaServer(const MediaPlayerManagerS& _MediaPlayerManager, int _WorkPeriodMs)
  : CtrlWorker(_WorkPeriodMs)
  , mMediaPlayerManager(_MediaPlayerManager)
{
}

MediaServer::~MediaServer()
{
}

