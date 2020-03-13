#include <QUrl>
#include <QByteArray>

#include <Lib/Log/Log.h>
#include <LibV/MediaServer/H264/H264Sprop.h>
#include <LibV/MediaServer/H264/H264NalUnit.h>

#include "Sdp.h"


SdpS Sdp::CreateFromH264Frame(const char* data, int size)
{
  H264NalUnit nalu(data, size);
  H264Sprop sprop;
  QByteArray sps;
  QByteArray pps;

  while (nalu.FindNext()) {
    bool ok = sprop.Parse(nalu.CurrentUnit(), nalu.CurrentUnitSize());
    if (sprop.HasSps()) {
      sps = QByteArray(nalu.CurrentUnit(), nalu.CurrentUnitSize());
      sprop.ClearSps();
      if (!ok) {
        Log.Warning(QString("Bad sps in frame (%1)").arg(sps.toHex().constData()));
      }
    } else if (sprop.HasPps()) {
      pps = QByteArray(nalu.CurrentUnit(), nalu.CurrentUnitSize());
      sprop.ClearPps();
      if (!ok) {
        Log.Warning(QString("Bad pps in frame (%1)").arg(pps.toHex().constData()));
      }
    }
  }

  if (!sps.isEmpty() && !pps.isEmpty()) {
    SdpS sdp(new Sdp());
    sdp->AddH264Media(sps, pps);
    return sdp;
  }
  return SdpS();
}

bool Sdp::Parse(const QByteArray& data)
{
  QList<QByteArray> lines = data.split('\n');
  if (lines.last().isEmpty()) {
    lines.removeLast();
  }
  for (auto itr = lines.begin(); itr != lines.end(); itr++) {
    QByteArray line = *itr;
    if (line.size() > 0 && line[line.size()-1] == '\r') {
      line = line.mid(0, line.size() - 1);
    }

    if (line.size() < 3) {
      Log.Warning(QString("Sdp parse fail (line: '%1', error: 'too short line')").arg(line.constData()));
      return false;
    } if (line[1] != '=') {
      Log.Warning(QString("Sdp parse fail (line: '%1', error: 'no equal sign')").arg(line.constData()));
      return false;
    }
    char name = line[0];

    switch (name)
    {
    case 'v': break;
    case 'o': // o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
    {
      QList<QByteArray> params = line.split(' ');
      if (params.size() < 6) {
        Log.Warning(QString("Sdp parse warning (line: '%1', warning: 'bad params count')").arg(line.constData()));
        break;
      }
      mId = params[1] + ' ' + params[2];
    }
      break;
    case 's': mName = line.mid(2); break;
    case 'i': break;
    case 'u': break;
    case 'e': break;
    case 'p': break;
    case 'c': break;
    case 'b': break;
    case 't': break;
    case 'r': break;
    case 'z': break;
    case 'k': break;
    case 'm': // m=<media> <port> <proto> <fmt> ...
    {
      Media media;
      QList<QByteArray> params = line.mid(2).split(' ');
      if (params.size() < 4) {
        Log.Warning(QString("Sdp parse fail (line: '%1', error: 'bad params count')").arg(line.constData()));
        return false;
      }
      if (params[0] == "audio") {
        media.Type = Media::eAudio;
      } else if (params[0] == "video") {
        media.Type = Media::eVideo;
      } else if (params[0] == "text") {
        media.Type = Media::eText;
      } else if (params[0] == "application") {
        media.Type = Media::eApplication;
      } else if (params[0] == "message") {
        media.Type = Media::eMessage;
      } else {
        media.Type = Media::eIllegal;
        Log.Warning(QString("Sdp parse warning (line: '%1', warning: 'bad media type')").arg(line.constData()));
      }
      if (!params[2].startsWith("RTP/AVP")) {
        Log.Warning(QString("Sdp parse warning (line: '%1', warning: 'bad media proto')").arg(line.constData()));
      }
      media.Udp = !params[2].endsWith("TCP");
      media.Format = params[3];
      for (int i = 4; i < params.size(); i++) {
        media.Format += ' ' + params[i];
      }

      mMedias.append(media);
    }
      break;
    case 'a':
    {
      QByteArray key;
      QByteArray value;
      int ind = line.indexOf(':', 2);
      if (ind > 0) {
        key = line.mid(2, ind - 2);
        value = line.mid(ind + 1);
      } else {
        key = line.mid(2);
      }
      if (key == "rtpmap") { // a=rtpmap:<payload type> <encoding name>/<clock rate> [/<encoding parameters>]
        if (mMedias.isEmpty()) {
          Log.Warning(QString("Sdp parse warning (line: '%1', warning: 'rtpmap before media')").arg(line.constData()));
          break;
        }
        mMedias.last().Rtpmap = value;
      } else if (key == "fmtp") {
        if (mMedias.isEmpty()) {
          Log.Warning(QString("Sdp parse warning (line: '%1', warning: 'fmtp before media')").arg(line.constData()));
          break;
        }

        mMedias.last().Fmtp = value;
      } else if (key == "control") {
        if (mMedias.isEmpty()) {
          break;
        }
        QString path = QString::fromUtf8(value);
        QUrl url = QUrl::fromUserInput(path);
        mMedias.last().Path = (url.isValid())? url.path(): path;
      }
      break;
    }
    default:
      Log.Warning(QString("Sdp parse warning (line: '%1', warning: 'unknown field')").arg(line.constData()));
      break;
    }
  }
  return true;
}

bool Sdp::GetSprops(int mediaIndex, QByteArray& sprops)
{
  if (mediaIndex >= mMedias.size()) {
    return false;
  }
  QByteArray& fmtp = mMedias[mediaIndex].Fmtp;
  int d1 = fmtp.indexOf("sprop-parameter-sets");
  if (d1 < 0) {
    sprops = QByteArray();
    return true;
  }
  int d2 = fmtp.indexOf(';', d1);
  if (d2 > 0 && fmtp[d2-1] == ' ') {
    d2--;
  }
  sprops = fmtp.mid(d1, (d2 > 0)? d2 - d1: -1);
  return true;
}

bool Sdp::SetSprops(int mediaIndex, const QByteArray& sprops)
{
  if (mediaIndex >= mMedias.size()) {
    return false;
  }
  QByteArray& fmtp = mMedias[mediaIndex].Fmtp;
  int d1 = fmtp.indexOf("sprop-parameter-sets");
  if (d1 < 0) {
    if (!fmtp.isEmpty()) {
      fmtp.append(QByteArray("; "));
    }
    fmtp.append(sprops);
    return true;
  }
  int d2 = fmtp.indexOf(';', d1);
  fmtp.remove(d1, (d2 > 0)? d2 - d1: -1);
  fmtp.insert(d1, sprops);
  return true;
}

QByteArray Sdp::ToText(const QString& hostName)
{
  return ToText(mId, mName, hostName);
}

QByteArray Sdp::ToText(const QByteArray& id, const QString& serverName, const QString& hostName)
{
  QByteArray sdpText("v=0\n");
  sdpText.append(QByteArray("o=- ") + id + " IN IP4 " + hostName.toLatin1() + "\n");
  sdpText.append(QByteArray("s=" + serverName.toUtf8() + "\n"));
  sdpText.append(QByteArray("c=IN IP4 0.0.0.0\n"));
  sdpText.append(QByteArray("t=0 0\n"));
  sdpText.append(QByteArray("a=charset:UTF-8\n"));

  for (auto itr = mMedias.begin(); itr != mMedias.end(); itr++) {
    const Media& media = *itr;
    sdpText.append(QByteArray("m=") + TypeToString(media.Type) + " 0 RTP/AVP " + media.Format + "\n");
    sdpText.append(QByteArray("a=control:") + media.Path.toUtf8() + "\n");
    sdpText.append(QByteArray("a=rtpmap:") + media.Rtpmap + "\n");
    sdpText.append(QByteArray("a=fmtp:") + media.Fmtp + "\n");
    sdpText.append(QByteArray("a=range:npt=0-\n"));
  }

  return sdpText;
}

const char* Sdp::TypeToString(Sdp::Media::EType type)
{
  switch (type) {
  case Media::eAudio: return "audio";
  case Media::eVideo: return "video";
  case Media::eText: return "text";
  case Media::eApplication: return "application";
  case Media::eMessage: return "message";
  case Media::eIllegal: return "illegal";
  default: return "error";
  }
}

void Sdp::AddH264Media(const QByteArray& sps, const QByteArray& pps)
{
  int index = mMedias.size();
  Media media;
  media.Type = Media::eVideo;
  media.Udp = false;
  media.Format = QByteArray("96");
  media.Fmtp = QByteArray("96 packetization-mode=1");
  if (sps.size() >= 4) {
    media.Fmtp.append(QByteArray("; profile-level-id=" + sps.mid(1, 3).toHex()));
  }
  if (!sps.isEmpty() || !pps.isEmpty()) {
    media.Fmtp.append("; sprop-parameter-sets=");
    if (!sps.isEmpty()) {
      media.Fmtp.append(sps.toBase64());
      if (!pps.isEmpty()) {
        media.Fmtp.append(QByteArray(",") + pps.toBase64());
      }
    }
  }
  media.Rtpmap = "96 H264/90000";
  media.Path   = QString("streamid=%1").arg(index);

  mMedias.append(media);
}


Sdp::Sdp()
  : mId("1 1"), mName("Unnamed")
{
}
