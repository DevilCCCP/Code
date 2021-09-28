#pragma once

#include <QMap>
#include <QMutex>

#include "RtspMedia.h"
#include "../MediaServer.h"


DefineClassS(RtspServer);
DefineClassS(RtspHandler);
DefineClassS(RtspMedia);
DefineClassS(NetServer);
DefineClassS(SdpExtantion);
DefineClassS(MediaPlayer);
DefineClassS(CtrlManager);
DefineClassS(SettingsA);

struct MediaPath {
  RtspMediaW MediaLink;
  int        MediaIndex;
  QByteArray MediaChannel;

  MediaPath(RtspMediaW _MediaLink, int _MediaIndex = -1)
    : MediaLink(_MediaLink), MediaIndex(_MediaIndex)
  { }
  MediaPath(RtspMediaW _MediaLink, QByteArray _MediaChannel, int _MediaIndex = -1)
    : MediaLink(_MediaLink), MediaIndex(_MediaIndex), MediaChannel(_MediaChannel)
  { }
  MediaPath()
    : MediaLink(), MediaIndex(-1)
  { }
};
typedef QMap<QString, MediaPath> MapMedia;

class RtspServer: public MediaServer
{
  QString       mServerName;
  QString       mLogin;
  QString       mPassword;
  QString       mUserAgent;
  MapMedia      mMediaMap;
  QMutex        mMediaMapMutex;

  QMutex        mUniqMutex;
  qint64        mUniqTime;
  int           mUniqCounter;
  quint32       mUniq2Time;
  quint32       mUniq2Counter;

  SdpExtantionS mSdpExtantion;

public: /*internal */
  const QString& ServerName() { return mServerName; }
  const QString& Login() { return mLogin; }
  const QString& Password() { return mPassword; }
  const QString& UserAgent() { return mUserAgent; }
public:
  void SetSdpExtantion(const SdpExtantionS& _SdpExtantion) { mSdpExtantion = _SdpExtantion; }
  const SdpExtantionS& GetSdpExtantion() { return mSdpExtantion; }

public:
  /*override */virtual const char* Name() override { return "Rtsp server"; }
  /*override */virtual const char* ShortName() override { return "Rv"; }

protected:
  /*override */virtual void OnRegisterMedia(const MediaS& media) override;
  /*override */virtual void OnUnregisterMedia(const MediaS& media) override;

public: /*internal */
  const MediaPath& FindMediaPath(const QString& path);
  void RegisterChannel(const RtspMediaS& media, const QByteArray& path, int streamCount);

  QByteArray GetUniqueSessionId();
  QByteArray GetUniqueSessionId2();
  QByteArray GetUniqueChannelId();

  //bool AnnounceWithSdp(const QString& mediaId, const QByteArray& data);

  bool SetupChannelStream(const MediaPath& mediaPath, QByteArray& channelId, const QByteArray& transportIn
                          , QByteArray& transportOut, RtspHandler* handler);

private:
  void GenerateUnique();
  bool FixSdp(Sdp& sdp);
  bool FixSprops(QByteArray& sprops);
  void DumpMediaMap();

public:
  static RtspServerS CreateRtspServer(CtrlManager* manager, SettingsA* settings, const MediaPlayerManagerS& _MediaPlayerManager, NetServerS* netServer = nullptr);

public:
  explicit RtspServer(SettingsA* _Settings, const MediaPlayerManagerS& _MediaPlayerManager);
  /*override */virtual ~RtspServer();
};

