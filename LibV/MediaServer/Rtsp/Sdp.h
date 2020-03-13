#pragma once

#include <QList>
#include <QByteArray>

#include <Lib/Include/Common.h>


DefineClassS(Sdp);

class Sdp
{
public:
  struct Media {
    enum EType {
      eAudio,
      eVideo,
      eText,
      eApplication,
      eMessage,
      eIllegal
    };

    EType      Type;
    bool       Udp;
    QString    Path;
    QByteArray Format;
    QByteArray Rtpmap;
    QByteArray Fmtp;
  };

private:
  QByteArray   mId;
  QByteArray   mName;

  QList<Media> mMedias;

public:
  void SetId(int id, int rev) { mId = QByteArray::number(id) + ' ' + QByteArray::number(rev); }
  void SetName(const QString& name) { mName = name.toUtf8(); }
  const QList<Media>& Medias() { return mMedias; }

public:
  static SdpS CreateFromH264Frame(const char* data, int size);

public:
  bool Parse(const QByteArray& data);
  bool GetSprops(int mediaIndex, QByteArray& sprops);
  bool SetSprops(int mediaIndex, const QByteArray& sprops);
  QByteArray ToText(const QString& hostName);
  QByteArray ToText(const QByteArray& id, const QString& serverName, const QString& hostName);

  static const char* TypeToString(Media::EType type);

  void AddH264Media(const QByteArray& sps, const QByteArray& pps);

public:
  Sdp();
};

/* rfc4566 *
 * 5.  SDP Specification
      Session description
         v=  (protocol version)
         o=  (originator and session identifier)
         s=  (session name)
         i=* (session information)
         u=* (URI of description)
         e=* (email address)
         p=* (phone number)
         c=* (connection information -- not required if included in
              all media)
         b=* (zero or more bandwidth information lines)
         One or more time descriptions ("t=" and "r=" lines; see below)
         z=* (time zone adjustments)
         k=* (encryption key)
         a=* (zero or more session attribute lines)
         Zero or more media descriptions

      Time description
         t=  (time the session is active)
         r=* (zero or more repeat times)

      Media description, if present
         m=  (media name and transport address)
         i=* (media title)
         c=* (connection information -- optional if included at
              session level)
         b=* (zero or more bandwidth information lines)
         k=* (encryption key)
         a=* (zero or more media attribute lines)
 * 6.  SDP Attributes
      a=rtpmap:<payload type> <encoding name>/<clock rate> [/<encoding parameters>]
      a=recvonly
      a=sendrecv
      a=sendonly
      a=type:<conference type>
      a=charset:<character set>
      a=fmtp:<format> <format specific parameters>
*/

/*Examples:
  v=0
  o=- 15662175806809436665 15662175806809436665 IN IP4 X2
  s=Unnamed
  i=N/A
  c=IN IP4 0.0.0.0
  t=0 0
  c=IN IP4 0.0.0.0
  t=0 0
  a=tool:vlc 2.2.1
  a=recvonly
  a=type:broadcast
  a=charset:UTF-8
  a=control:rtsp://127.0.0.1:8555/xxx
  m=video 0 RTP/AVP 96
  b=RR:0
  a=rtpmap:96 MP4V-ES/90000
  a=fmtp:96 profile-level-id=3; config=00000120088684003f18b02240a31f;
  a=control:rtsp://127.0.0.1:8555/xxx/trackID=0

  v=0
  o=- 1438950609252844 1 IN IP4 192.168.56.1
  s=Session streamed by "xxx"
  i=25
  t=0 0
  a=tool:LIVE555 Streaming Media v2012.06.12
  a=type:broadcast
  a=control:*
  a=range:npt=0-
  a=x-qt-text-nam:Session streamed by "KSVD Rtsp Gate"
  a=x-qt-text-inf:25
  m=video 0 RTP/AVP 26
  c=IN IP4 0.0.0.0
  b=AS:4294967295
  a=control:track1

  v=0
  o=- 1270964922 1 IN IP4 10.20.30.4
  s=SONY RTSP Server
  c=IN IP4 0.0.0.0
  t=0 0
  a=range:npt=now-
  m=video 0 RTP/AVP 105
  a=rtpmap:105 H264/90000
  a=control:video
  a=framerate:15.0
  a=fmtp:105 packetization-mode=1; profile-level-id=428020; sprop-parameter-sets=Z0KAINoBQAgR,aM48gA==

    a=0
    v=0
    o=- 0 0 IN IP4 127.0.0.1
    s=No Name
    c=IN IP4 192.168.1.4
    t=0 0
    a=tool:libavformat 56.15.101
    m=video 0 RTP/AVP 96
    b=AS:400
    a=rtpmap:96 MP4V-ES/90000
    a=fmtp:96 profile-level-id=1; config=000001B001000001B58913000001000000012000C48D8F53050B04241463000001B24C61766335362E31332E313030
    a=control:streamid=0
    */

