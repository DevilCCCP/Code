#include <QMutexLocker>
#include <QDateTime>

#include <Lib/Ctrl/CtrlManager.h>
#include <Lib/Settings/SettingsA.h>
#include <Lib/NetServer/NetServer.h>
#include <Lib/Log/Log.h>
#include <LibV/MediaServer/H264/H264Sprop.h>

#include "RtspServer.h"
#include "RtspHandler.h"
#include "RtspChannel.h"
#include "Sdp.h"
#include "../Media.h"
#include "../MediaPlayer.h"


typedef HandlerManagerC<RtspHandler, RtspServer> RtspManager;
typedef QSharedPointer<RtspManager> RtspManagerS;

//bool RtspServer::AnnounceWithSdp(const QString& mediaId, const QByteArray& data)
//{
//  SdpS sdp(new Sdp());
//  if (!sdp->Parse(data)) {
//    return false;
//  }
//
//  FixSdp(*sdp);
//
//  MediaS media = FindMedia(mediaId);
//  RtspMedia* rtspMedia = static_cast<RtspMedia*>(media.data());
//  if (rtspMedia) {
//    rtspMedia->SetSdp(sdp);
//    return true;
//  }
//
////  RemoveMedia(mediaId, false);
//  media = MediaS(new RtspMedia(mediaId, sdp));
//  return AddMedia(media);
//}

bool RtspServer::SetupChannelStream(const MediaPath& mediaPath, QByteArray& channelId, const QByteArray& transportIn
                                    , QByteArray& transportOut, RtspHandler* handler)
{
  RtspMediaS rtspMedia = mediaPath.MediaLink.toStrongRef();
  if (!rtspMedia) {
    return false;
  }

  RtspChannelS rtspChannel;
  do {
    int mode = 0;
    int clientPort1 = 0;
    int clientPort2 = 0;
    int interleavedCh1 = 0;
    int interleavedCh2 = 0;
    bool tcp = false;
    QList<QByteArray> transportValues = transportIn.split(';');
    QByteArray transportType = transportValues.takeFirst();

    if (!transportType.startsWith("RTP/AVP")) {
      Log.Error(QString("Rtsp server: not implemented transport type '%1'").arg(transportType.constData()));
      break;
    }
    if (transportType.endsWith("/TCP")) {
      tcp = true;
    }

    for (auto itr = transportValues.begin(); itr != transportValues.end(); itr++) {
      QList<QByteArray> keyValue = itr->split('=');
      if (keyValue.size() == 2) {
        if (keyValue[0] == "mode") {
          if (keyValue[1] == "record") {
            mode = 1;
          }
        } else if (keyValue[0] == "client_port") {
          QList<QByteArray> ports = keyValue[1].split('-');
          if (ports.size() == 2) {
            clientPort1 = ports[0].toInt();
            clientPort2 = ports[1].toInt();
          }
        } else if (keyValue[0] == "interleaved") {
          QList<QByteArray> ports = keyValue[1].split('-');
          if (ports.size() == 2) {
            interleavedCh1 = ports[0].toInt();
            interleavedCh2 = ports[1].toInt();
          }
        }
      }
    }

    if (tcp) {
      if (interleavedCh1 == 0 && interleavedCh2 == 0) {
        Log.Warning(QString("RTSP TCP non-interleaved streams not allowed"));
        break;
      }

      rtspChannel = rtspMedia->FindChannel(channelId);

      if (mode) { // record
        if (!mediaPath.MediaChannel.isEmpty()) {
          RtspChannelS rtspChannelOut = rtspMedia->FindChannel(mediaPath.MediaChannel);
          handler->SetOutChannel(rtspChannel);
          rtspChannel->SetOutChannel(rtspChannelOut);
        }
      } else { // play
        if (!rtspChannel) {
          MediaPlayerS player = rtspMedia->GetMediaPlayerManager()->CreateMediaPlayer();
          channelId = GetUniqueChannelId();
          rtspChannel = RtspChannelS(new RtspChannel(rtspMedia.data(), channelId, player));
          rtspMedia->AddChannel(rtspChannel);
          if (!player->Open(rtspChannel.data())) {
            Log.Warning(QString("Open player for interleaved media stream fail (id: %1, channel: %2)")
                        .arg(rtspMedia->Id()).arg(channelId.constData()));
            break;
          }
        }
        if (!rtspChannel->SetIvPorts(mediaPath.MediaIndex, interleavedCh1, interleavedCh2)) {
          Log.Warning(QString("Set channels for interleaved media stream fail (id: %1, path: %2, ch: %3-%4)")
                      .arg(rtspMedia->Id()).arg(mediaPath.MediaChannel.constData()).arg(interleavedCh1).arg(interleavedCh2));
          break;
        }
        rtspChannel->SetHandler(handler);
        handler->SetInChannel(rtspChannel);
        RegisterChannel(rtspMedia, channelId, rtspMedia->GetSdp()->Medias().count());
      }

      transportOut  = QByteArray("RTP/AVP/TCP;unicast");
      transportOut += QByteArray(mode? ";mode=receive": ";mode=play");
      transportOut += QByteArray(";interleaved=") + QByteArray::number(interleavedCh1) + QByteArray("-") + QByteArray::number(interleavedCh2);
      transportOut += QByteArray(";ssrc=") + channelId;
    } else {
      int serverPort1 = 0;
      int serverPort2 = 0;
      if (mode) { // record
        Log.Warning(QString("RTSP UCP Record not allowed"));
        break;
      } else { // play
        if (!rtspChannel) {
          MediaPlayerS player = rtspMedia->GetMediaPlayerManager()->CreateMediaPlayer();
          channelId = GetUniqueChannelId();
          rtspChannel = RtspChannelS(new RtspChannel(rtspMedia.data(), channelId, player));
          rtspMedia->AddChannel(rtspChannel);
          if (!player->Open(rtspChannel.data())) {
            Log.Warning(QString("Open player for udp media stream fail (id: %1, channel: %2)")
                        .arg(rtspMedia->Id()).arg(channelId.constData()));
            break;
          }
        }
        if (!rtspChannel->SetUdp(handler->HostAddress(), mediaPath.MediaIndex, clientPort1, clientPort2, serverPort1, serverPort2)) {
          Log.Warning(QString("Set channels for udp media stream fail (id: %1, path: %2, ports: %3-%4)")
                      .arg(rtspMedia->Id()).arg(mediaPath.MediaChannel.constData()).arg(clientPort1).arg(clientPort2));
          break;
        }
        RegisterChannel(rtspMedia, channelId, rtspMedia->GetSdp()->Medias().count());
      }

      //Transport: RTP/AVP/UDP;unicast;client_port=57232-57233;server_port=59367-59368;ssrc=869951CA;mode=play
      transportOut  = QByteArray("RTP/AVP/UDP;unicast");

      transportOut += QByteArray(mode? ";mode=receive": ";mode=play");
      transportOut += QByteArray(";client_port=") + QByteArray::number(clientPort1) + "-" + QByteArray::number(clientPort2);
      transportOut += QByteArray(";server_port=") + QByteArray::number(serverPort1) + "-" + QByteArray::number(serverPort2);
      transportOut += QByteArray(";ssrc=") + channelId;
    }
    return true;
  } while (false);

  if (rtspChannel) {
    rtspMedia->RemoveChannel(rtspChannel);
  }
  return false;
}

void RtspServer::OnRegisterMedia(const MediaS& media)
{
  QMutexLocker lock(&mMediaMapMutex);
  RtspMediaS rtspMedia = media.staticCast<RtspMedia>();
  SdpS sdp = rtspMedia->GetSdp();
  if (!sdp) {
    Log.Error(QString("Register media without SDP (id: '%1')").arg(rtspMedia->Id()));
    return;
  }
  mMediaMap.insert(rtspMedia->Id(), MediaPath(rtspMedia.toWeakRef()));
  int index = 0;
  for (auto itr = sdp->Medias().begin(); itr != sdp->Medias().end(); itr++, index++) {
    const Sdp::Media& medi = *itr;
    mMediaMap.insert(rtspMedia->Id() + "/" + medi.Path, MediaPath(rtspMedia.toWeakRef(), index));
  }
//  DumpMediaMap();
}

void RtspServer::OnUnregisterMedia(const MediaS& media)
{
  QMutexLocker lock(&mMediaMapMutex);
  RtspMedia* rtspMedia = static_cast<RtspMedia*>(media.data());
  mMediaMap.remove(rtspMedia->Id());
  SdpS sdp = rtspMedia->GetSdp();
  for (auto itr = sdp->Medias().begin(); itr != sdp->Medias().end(); itr++) {
    const Sdp::Media& medi = *itr;
    mMediaMap.remove(medi.Path);
  }
//  DumpMediaMap();
}

const MediaPath& RtspServer::FindMediaPath(const QString& path)
{
  static MediaPath gNullPath;

  QMutexLocker lock(&mMediaMapMutex);
  auto itr = mMediaMap.find(path);
  if (itr != mMediaMap.end()) {
    return itr.value();
  } else {
    return gNullPath;
  }
}

void RtspServer::RegisterChannel(const RtspMediaS& media, const QByteArray& path, int streamCount)
{
  QMutexLocker lock(&mMediaMapMutex);
  mMediaMap[QString("%1/%2").arg(media->Id()).arg(path.constData())] = MediaPath(media.toWeakRef(), path);
  for (int i = 0; i < streamCount; i++) {
    mMediaMap[QString("%1/%2/streamid=%3").arg(media->Id()).arg(path.constData()).arg(i)] = MediaPath(media.toWeakRef(), path, i);
  }
//  DumpMediaMap();
}

QByteArray RtspServer::GetUniqueSessionId()
{
  QMutexLocker lock(&mUniqMutex);
  GenerateUnique();

  return QByteArray::number(mUniqTime) + " " + QByteArray::number(mUniqCounter);
}

QByteArray RtspServer::GetUniqueSessionId2()
{
  QMutexLocker lock(&mUniqMutex);
  GenerateUnique();

  return QByteArray::number(mUniqTime) + QByteArray::number(mUniqCounter);
}

QByteArray RtspServer::GetUniqueChannelId()
{
  quint32 time = (quint32)(QDateTime::currentMSecsSinceEpoch() / 1000);
  time = (time << 8);
  QMutexLocker lock(&mUniqMutex);
  if (time != mUniq2Time) {
    mUniq2Time = time;
    mUniq2Counter = 0x79;
  } else {
    mUniq2Counter++;
  }
  QByteArray result;
  result.append((char)(uchar)(mUniq2Time >> 24));
  result.append((char)(uchar)(mUniq2Time >> 16));
  result.append((char)(uchar)(mUniq2Time >> 8));
  result.append((char)(uchar)(mUniq2Counter));

  return result.toHex();
}

void RtspServer::GenerateUnique()
{
  qint64 time = QDateTime::currentMSecsSinceEpoch();
  if (time > mUniqTime) {
    mUniqTime = time;
    mUniqCounter = 1;
  } else {
    mUniqCounter++;
  }
}

bool RtspServer::FixSdp(Sdp& sdp)
{
  bool fixed = false;
  for (int i = 0; i < 16; i++) {
    QByteArray sprops;
    if (!sdp.GetSprops(i, sprops)) {
      break;
    }
    if (!sprops.isEmpty()) {
      if (FixSprops(sprops)) {
        sdp.SetSprops(i, sprops);
        fixed = true;
      }
    }
  }
  return fixed;
}

bool RtspServer::FixSprops(QByteArray& sprops)
{
  bool changed = false;
  int p1 = sprops.indexOf('=');
  if (p1 < 0) {
    return false;
  }
  p1++;
  QList<QByteArray> nalList = sprops.mid(p1).split(',');
  for (auto itr = nalList.begin(); itr != nalList.end(); itr++) {
    QByteArray& nalBase64 = *itr;
    QByteArray nalBin = QByteArray::fromBase64(nalBase64);
    H264Sprop sprop;
    if (!sprop.Parse(nalBin.constData(), nalBin.size()) && sprop.HasSps()) {
      QByteArray nalFixed(sprop.WriteSize(), 0);
      int writeSize = sprop.Write(nalFixed.data(), nalFixed.size());
      nalFixed.resize(writeSize);
      nalBase64 = nalFixed.toBase64();
      changed = true;
    }
  }

  if (changed) {
    sprops = sprops.left(p1);
    bool first = true;
    for (auto itr = nalList.begin(); itr != nalList.end(); itr++) {
      if (!first) {
        sprops += ',';
      } else {
        first = false;
      }
      sprops += *itr;
    }
  }
  return changed;
}

void RtspServer::DumpMediaMap()
{
  QString info = "Media map info:\n";
  for (auto itr = mMediaMap.begin(); itr != mMediaMap.end(); itr++) {
    const QString& key = itr.key();
    const MediaPath& value = itr.value();
    info += QString("%1 {ind: %2, path: %3}\n").arg(key).arg(value.MediaIndex).arg(value.MediaChannel.constData());
  }
  Log.Info(info);
}

RtspServerS RtspServer::CreateRtspServer(CtrlManager* manager, SettingsA* settings, const MediaPlayerManagerS& _MediaPlayerManager, NetServerS* netServer)
{
  int port = settings->GetValue("RtspPort", 0).toInt();
  if (!port) {
    return RtspServerS();
  }
  RtspServerS rtsp(new RtspServer(settings, _MediaPlayerManager));
  manager->RegisterWorker(rtsp);
  NetServerS server = NetServerS(new NetServer(port, RtspManagerS(new RtspManager(rtsp.data()))));
  if (netServer) {
    *netServer = server;
  }
  manager->RegisterWorker(server);
  return rtsp;
}


RtspServer::RtspServer(SettingsA* _Settings, const MediaPlayerManagerS& _MediaPlayerManager)
  : MediaServer(_MediaPlayerManager)
  , mUniqTime(0), mUniqCounter(1)
{
  mLogin = _Settings->GetValue("Login", "").toString();
  mPassword = _Settings->GetValue("Password", "").toString();
  mServerName = _Settings->GetValue("ServerName", "No name RTSP server").toString();
  mUserAgent = _Settings->GetValue("UserAgent", "RTSP server").toString();
}

RtspServer::~RtspServer()
{
}
