#pragma once

#include <QMap>
#include <QMutex>

#include <Lib/Ctrl/CtrlWorker.h>


DefineClassS(MediaServer);
DefineClassS(Media);
DefineClassS(MediaPlayerManager);

class MediaServer: public CtrlWorker
{
  MediaPlayerManagerS   mMediaPlayerManager;

  QMap<QString, MediaS> mMedias;
  QMutex                mMediaMutex;

public: /*internal */
  const MediaPlayerManagerS& GetMediaPlayerManager() { return mMediaPlayerManager; }

protected:
//  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
//  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;
public:
//  /*override */virtual void Stop() Q_DECL_OVERRIDE;

protected:
  /*new */virtual void OnRegisterMedia(const MediaS& media);
  /*new */virtual void OnUnregisterMedia(const MediaS& media);

public:
  bool AddMedia(const MediaS& media);
  bool RemoveMedia(const QString& mediaId, bool warning = true);

protected:
  MediaS FindMedia(const QString& mediaId);

public:
  explicit MediaServer(const MediaPlayerManagerS& _MediaPlayerManager, int _WorkPeriodMs = 500);
  /*override */virtual ~MediaServer();
};
