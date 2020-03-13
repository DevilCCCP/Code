#pragma once

#include <QMap>
#include <QVector>
#include <QMutex>

#include "Sdp.h"
#include "../Media.h"


DefineClassS(RtspMedia);
DefineClassS(RtspChannel);
DefineClassS(Sdp);

typedef QMap<QByteArray, RtspChannelS> MapChannel;

class RtspMedia: public Media
{
  MediaPlayerManagerS mMpManager;
  SdpS                mSdp;
  MapChannel          mMapChannel;
  QMutex              mChannelMutex;

public:
  void SetSdp(SdpS& _Sdp) { mSdp = _Sdp; }
  const SdpS& GetSdp() { return mSdp; }
  const MediaPlayerManagerS& GetMediaPlayerManager() { return mMpManager; }

protected:
  /*override */virtual bool OnTest() Q_DECL_OVERRIDE;
  /*override */virtual void OnChannelAdd(const ChannelS& channel) Q_DECL_OVERRIDE;
  /*override */virtual void OnChannelRemove(const ChannelS& channel) Q_DECL_OVERRIDE;

public:
  RtspChannelS FindChannel(const QByteArray& id);

public:
  explicit RtspMedia(const QString& _Id, const MediaPlayerManagerS& _MpManager, const SdpS& _Sdp);
  /*override */virtual ~RtspMedia();
};
