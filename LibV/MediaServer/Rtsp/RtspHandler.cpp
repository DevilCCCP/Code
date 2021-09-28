#include <QDateTime>
#include <QUrl>
#include <QLocale>
#include <QtEndian>
#include <QHostInfo>

#include <Lib/Log/Log.h>

#include "RtspHandler.h"
#include "RtspServer.h"
#include "RtspChannel.h"
#include "Sdp.h"
#include "SdpExtantion.h"
#include "../MediaPlayer.h"
#include "../TrFrame.h"


#define LOG_RTSP_COMMAND
#ifdef LOG_RTSP_COMMAND
#define LogCmd(X) Log.Info(X)
#else
#define LogCmd(X) Log.Trace(X)
#endif

#include <QThread>
#include <QElapsedTimer>
class RhTester: public QThread
{
  static const int kRepeatePeriodMs = 2000;
  static const int kRepeateCount = 2;

  RtspHandler*  mRtspHandler;
  QByteArray    mChannelId;
  QString       mMediaName;

  QElapsedTimer mTimer;
  int           mRepeate;

public:
  virtual void run() override
  {
    mTimer.start();
    for (int i = 1; i <= kRepeateCount; i++) {
      while (mTimer.elapsed() < i * kRepeatePeriodMs) {
        msleep(100);
      }
      QByteArray range = QString("clock=20160901T06%1-").arg(i * 1000).toLatin1();
      QByteArray scale = "2";
      if (i == 1) {
        range = QString("clock=20160901T071502-").toLatin1();
        scale = "-1";
      } else {
        range = QString("clock=20160901T071500-").toLatin1();
        scale = "1";
      }
      HeaderMap map;
      map.insert("Range", range);
      map.insert("Scale", scale);
      mRtspHandler->PlayFake(mChannelId, mMediaName, map);
    }
  }

public:
  RhTester(RtspHandler* _RtspHandler, QByteArray    _ChannelId, QString       _MediaName)
    : mRtspHandler(_RtspHandler), mChannelId(_ChannelId), mMediaName(_MediaName)
  {
    Log.Info("Run RhTester");
  }
};

static const int kSpamLimit = 4;

bool RtspHandler::Receive(SyncSocket* socket, bool& sendDone)
{
  InitRequest(socket);
  ReadRequest(4);

  if (Request().size() > 0) {
    if (Request()[0] == '$') {
      sendDone = false;
      return ReceiveRtspInterleaved();
    } else if (Request()[0] == 'R' && Request()[1] == 'T' && Request()[2] == 'S' && Request()[3] == 'P') {
      return ReceiveRtspReport();
    } else {
      //Log.Warning(QString("Non RTSP packet received"));
      return HttpHandler::Receive(socket, sendDone);
    }
  }
  return true;
}

void RtspHandler::OnDisconnected()
{
  if (mRtspChannelOut) {
    RtspChannelS channel = mRtspChannelOut.toStrongRef();
    if (channel) {
      channel->GetMedia()->RemoveChannel(channel);
    }
    mRtspChannelOut.clear();
  }
  if (mRtspChannelIn) {
    RtspChannelS channel = mRtspChannelIn.toStrongRef();
    if (channel) {
      channel->GetMedia()->RemoveChannel(channel);
    }
    mRtspChannelIn.clear();
  }

  HttpHandler::OnDisconnected();
}

bool RtspHandler::Options(const QString& path)
{
  LogCmd("OPTIONS");

  AntiSpam(2);

  Q_UNUSED(path);
  PrepareAnswer();

  Answer().append("Public: DESCRIBE,SETUP,TEARDOWN,PLAY,PAUSE,GET_PARAMETER\r\n\r\n");
  return true;
}

//bool RtspHandler::Announce(const QString& path, const QList<File>& files)
//{
//  LogCmd("ANNOUNCE");
//
//  AntiSpam(2);
//  QString mediaName = GetMediaPath(path);
//
//  do {
//    if (files.size() != 1) {
//      Log.Warning(QString("Bad ANNOUNCE (file size: %1)").arg(files.size()));
//      break;
//    }
//    auto itr1 = Headers().find("Content-Type");
//    if (itr1 == Headers().end()) {
//      Log.Warning(QString("Bad ANNOUNCE (Content-Type: missed)"));
//      break;
//    }
//    QByteArray contentType = itr1.value();
//    if (contentType != "application/sdp") {
//      Log.Warning(QString("Bad ANNOUNCE (Content-Type: %1)").arg(contentType.constData()));
//      break;
//    }
//
//    const MediaPath& mediaPath = mRtspServer->FindMediaPath(mediaName);
//    if (mediaPath.MediaIndex >= 0) {
//      ErrorAnswer(460, "Only aggregate operation allowed");
//      return true;
//    } else if (!mediaPath.MediaChannel.isEmpty()) {
//      Log.Info(QString("ANNOUNCE to prepared path"));
//    } else if (!mRtspServer->AnnounceWithSdp(mediaName, files.first().Data)) {
//      break;
//    }
//
//    PrepareAnswer();
//    Answer().append("\r\n");
//    return true;
//  } while (false);
//
//  ErrorAnswer();
//  return true;
//}

bool RtspHandler::Describe(const QString& path)
{
  LogCmd("DESCRIBE");

  AntiSpam(3);
  QString mediaName = GetMediaPath(path);

  const MediaPath& mediaPath = mRtspServer->FindMediaPath(mediaName);
  if (!mediaPath.MediaLink || mediaPath.MediaIndex >= 0) {
    Log.Warning(QString("Ask DESCRIBE unregistered media (media: '%1')").arg(mediaName));
    ErrorAnswer(404, "Not Found");
    return true;
  }

  RtspMediaS rtspMedia = mediaPath.MediaLink.toStrongRef();
  if (!rtspMedia || !rtspMedia->GetSdp()) {
    Log.Warning(QString("Can't DESCRIBE uninit media (media: '%1')").arg(mediaName));
    ErrorAnswer(404, "Not Found");
    return true;
  }

  mPlayOption = Params();

  PrepareAnswer();

  QByteArray sdpText = rtspMedia->GetSdp()->ToText(mRtspServer->GetUniqueSessionId(), mRtspServer->ServerName(), QHostInfo::localHostName());
  if (SdpExtantionS sdpExt = mRtspServer->GetSdpExtantion()) {
    sdpExt->SdpExtraInit(mediaName, sdpText);
  }
  Answer().append("Content-Type: application/sdp\r\n");
  Answer().append("Content-Base: " + path + "\r\n");
  Answer().append("Content-Length: " + QByteArray::number(sdpText.size()) + "\r\n\r\n");
  Answer().append(sdpText);

  return true;
}

bool RtspHandler::Setup(const QString& path)
{
  LogCmd("SETUP");

  AntiSpam(4);
  QString mediaName = GetMediaPath(path);

  QByteArray channelId;
  auto itr2 = Headers().find("Session");
  if (itr2 != Headers().end()) {
    channelId = itr2.value();
  }

  do {
    const MediaPath& mediaPath = mRtspServer->FindMediaPath(mediaName);
    if (!mediaPath.MediaLink) {
      Log.Warning(QString("SETUP request not existed media (stream: '%1')").arg(mediaName));
      break;
    } else if (mediaPath.MediaIndex < 0) {
      ErrorAnswer(459, "Aggregate Operation Not Allowed");
      return true;
    //} else if (!mediaPath.MediaLink.toStrongRef()->CanSetup()) {
    //  ErrorAnswer(455, "Method Not Valid in This State");
    //  return true;
    }

    auto itr1 = Headers().find("Transport");
    if (itr1 == Headers().end()) {
      Log.Warning(QString("SETUP Transport missed"));
      break;
    }
    QByteArray transportIn = itr1.value();

    QByteArray transportOut;
    if (!mRtspServer->SetupChannelStream(mediaPath, channelId, transportIn, transportOut, this)) {
      break;
    }

    PrepareAnswer();
    Answer().append("Session: " + channelId + "\r\n");
    Answer().append("Transport: " + transportOut + "\r\n");
    Answer().append("\r\n");
    return true;
  } while (false);

  ErrorAnswer();
  return true;
}

//bool RtspHandler::Record(const QString& path)
//{
//  LogCmd("RECORD");

//  AntiSpam(5);
//  QString mediaName = GetMediaPath(path);

//  const MediaPath& mediaPath = mRtspServer->FindMediaPath(mediaName);
//  if (!mediaPath.MediaLink) {
//    Log.Warning(QString("RECORD request not existed media (stream: '%1')").arg(mediaName));
//    return true;
//  } else if (mediaPath.MediaIndex >= 0) {
//    ErrorAnswer(460, "Only aggregate operation allowed");
//    return true;
//  }
//  RtspMediaS rtspMedia = mediaPath.MediaLink.toStrongRef();
//  if (!rtspMedia) {
//    ErrorAnswer(404, "Not Found");
//    return true;
//  }

//  QByteArray channelId;
//  if (FindChannel(rtspMedia, channelId)) {
//    PrepareAnswer();
//    Answer().append("\r\n");
//  } else {
//    ErrorAnswer();
//  }
//  return true;
//}

bool RtspHandler::Play(const QString& path)
{
  LogCmd("PLAY");

  AntiSpam(6);
  QByteArray channelId;
  RtspMediaS rtspMedia;
  RtspChannelS rtspChannel;
  if (!GetMediaChannel("PLAY", path, channelId, rtspMedia, rtspChannel)) {
    return true;
  }

  HeaderMap params = Headers();
  for (auto itr = mPlayOption.begin(); itr != mPlayOption.end(); itr++) {
    const QByteArray& param = *itr;
    int eqSign = param.indexOf('=');
    if (eqSign > 0) {
      QByteArray key = param.mid(0, eqSign);
      QByteArray value = param.mid(eqSign + 1);
      params[key] = value;
    }
  }
  QByteArray extraHeaders;
  if (rtspChannel->GetMediaPlayer()->Play(params, extraHeaders)) {
    PrepareAnswer();
    Answer().append(extraHeaders);
    Answer().append("\r\n");
  } else {
    ErrorAnswer(457, "Invalid Range");
  }
#ifndef QT_NO_DEBUG
//  RhTester* t = new RhTester(this, channelId, rtspMedia->Id());
//  t->start();
#endif
  return true;
}

bool RtspHandler::Pause(const QString& path)
{
  LogCmd("PAUSE");

  AntiSpam(7);
  QByteArray channelId;
  RtspMediaS rtspMedia;
  RtspChannelS rtspChannel;
  if (!GetMediaChannel("PAUSE", path, channelId, rtspMedia, rtspChannel)) {
    return true;
  }

  if (rtspChannel->GetMediaPlayer()->Pause()) {
    PrepareAnswer();
    Answer().append("\r\n");
  } else {
    ErrorAnswer();
  }
  return true;
}

bool RtspHandler::GetParameter(const QString& path, const QList<File>& files)
{
  LogCmd("GET_PARAMETER");

  AntiSpam(8);
  QByteArray channelId;
  RtspMediaS rtspMedia;
  RtspChannelS rtspChannel;
  if (!GetMediaChannel("GET_PARAMETER", path, channelId, rtspMedia, rtspChannel)) {
    return true;
  }

  QByteArray dataIn;
  if (files.size() == 1) {
    dataIn = files.first().Data;
  }

  QByteArray dataOut;
  QByteArray extraHeaders;
  rtspChannel->GetParameters(dataIn, dataOut, extraHeaders);

  PrepareAnswer();
  Answer().append(extraHeaders);
  Answer().append("\r\n");
  Answer().append(dataOut);
  return true;
}

bool RtspHandler::Teardown(const QString& path)
{
  LogCmd("TEARDOWN");

  AntiSpam(9);
  QByteArray channelId;
  RtspMediaS rtspMedia;
  RtspChannelS rtspChannel;
  if (!GetMediaChannel("TEARDOWN", path, channelId, rtspMedia, rtspChannel)) {
    return true;
  }

  rtspChannel->GetMediaPlayer()->Stop();
  rtspMedia->RemoveChannel(rtspChannel.staticCast<Channel>());

  PrepareAnswer();
  Answer().append("\r\n");
  return true;
}

void RtspHandler::AntiSpam(int cmdId, int value)
{
  if (!mAntiSpam || !IsLogAny()) {
    return;
  }
  if (mRequestCommand == cmdId) {
    if (mRequestCount <= kSpamLimit) {
      mRequestCount += value;
      if (mRequestCount > kSpamLimit) {
        Log.Info(QString("Turn off spamming channel log"));
        SetLogRequest(false);
        SetLogRespond(false);
      }
    }
  } else {
    mRequestCommand = cmdId;
    if (mRequestCount > kSpamLimit) {
      Log.Info(QString("Turn on spamming channel log"));
      SetLogRequest(true);
      SetLogRespond(true);
    }
    mRequestCount = 0;
  }
}

QString RtspHandler::GetMediaPath(const QString& path)
{
  QUrl url = QUrl::fromUserInput(path);
  return (url.isValid())? url.path(): path;
}

bool RtspHandler::GetMediaChannel(const char* cmd, const QString& path, QByteArray& channelId, RtspMediaS& rtspMedia, RtspChannelS& rtspChannel)
{
  QString mediaName = GetMediaPath(path);

  auto itr2 = Headers().find("Session");
  if (itr2 == Headers().end()) {
    Log.Warning(QString("%1 without session id (stream: '%2')").arg(cmd).arg(mediaName));
    ErrorAnswer(400, "Bad Request");
    return false;
  }
  channelId = itr2.value();

  const MediaPath& mediaPath = mRtspServer->FindMediaPath(mediaName);
  if (!mediaPath.MediaLink) {
    Log.Warning(QString("%1 request not existed media (stream: '%2')").arg(cmd).arg(mediaName));
    ErrorAnswer(404, "Not Found");
    return false;
  } else if (mediaPath.MediaIndex >= 0) {
    ErrorAnswer(460, "Only aggregate operation allowed");
    return false;
  }
  rtspMedia = mediaPath.MediaLink.toStrongRef();
  if (!rtspMedia) {
    ErrorAnswer(404, "Not Found");
    return false;
  }

  rtspChannel = rtspMedia->FindChannel(channelId);
  if (!rtspChannel) {
    Log.Warning(QString("%1 session not found (stream: '%2', session: %3)").arg(cmd).arg(mediaName).arg(channelId.constData()));
    ErrorAnswer(404, "Not Found");
    return false;
  }

  return true;
}

bool RtspHandler::ReceiveRtspInterleaved()
{
  if (mInterleaveSize == 0) {
    if (Request().size() < 4) {
      return true;
    }
    mInterleaveChannel = Request()[1];
    mInterleaveSize = qFromBigEndian<quint16>((const uchar*)Request().constData() + 2) + 4;
//    Log.Trace(QString("%1 %2 %3 %4 (sz: %5)").arg((int)(uchar)Request().at(0), 2, 16).arg((int)(uchar)Request().at(1), 2, 16)
//      .arg((int)(uchar)Request().at(2), 2, 16).arg((int)(uchar)Request().at(3), 2, 16).arg(Request().size()));
  }

  ReadRequest(mInterleaveSize);
  if (Request().size() < mInterleaveSize) {
    return true;
  }

  if (mRtspChannelOut) {
    RtspChannelS channel = mRtspChannelOut.toStrongRef();
//    Log.Trace(QString("Play frame (ch: %1, sz: %2, channel: %3)").arg(mInterleaveChannel).arg(mInterleaveSize).arg(channel->Id().constData()));
    if (channel) {
      QByteArray data = Request().mid(0, mInterleaveSize);
      TrFrameS frame(new TrFrame(mInterleaveChannel, data));
      channel->InFrame(frame);
    } else {
      mRtspChannelOut.clear();
    }
  } else {
    LOG_WARNING_ONCE(QString("Receive unexpected interleaved frame (size: %1)").arg(mInterleaveSize));
  }
  EndRequest(mInterleaveSize);
  mInterleaveSize = 0;
  return true;
}

bool RtspHandler::ReceiveRtspReport()
{
  ReadRequest();

  for (; mRtspReportItr + 3 < Request().size(); mRtspReportItr++) {
    if (Request()[mRtspReportItr] == '\r' && Request()[mRtspReportItr + 1] == '\n' &&
        Request()[mRtspReportItr + 2] == '\r' && Request()[mRtspReportItr + 3] == '\n') {
      Log.Trace(QString("Receive report: '%1'").arg(Request().mid(0, mRtspReportItr + 4).constData()));
      EndRequest(mRtspReportItr + 4);
      mRtspReportItr = 0;
      return true;
    }
  }
  return true;
}

bool RtspHandler::FindChannel(const RtspMediaS& rtspMedia, QByteArray& channelId)
{
  auto itr = Headers().find("Session");
  if (itr != Headers().end()) {
    channelId = itr.value();
    return rtspMedia->FindChannel(channelId);
  }
  return false;
}

void RtspHandler::PrepareAnswer()
{
  Answer() = QByteArray(GetRtspHeader());
  if (Headers().contains("CSeq")) {
    Answer().append("CSeq: " + Headers()["CSeq"] + "\r\n");
  }
  QLocale locale = QLocale(QLocale::English, QLocale::AnyCountry);
  Answer().append("Date: " + locale.toString(QDateTime::currentDateTimeUtc(), "ddd, dd MMM yyyy hh:mm:ss") + " GMT\r\n");
  Answer().append("Server: " + mRtspServer->ServerName() + "\r\n");
  Answer().append("User-Agent: " + mRtspServer->UserAgent() + "\r\n");
  if (Headers().contains("Session")) {
    Answer().append("Session: " + Headers()["Session"] + "\r\n");
  }
}

void RtspHandler::ErrorAnswer(int code, const char* text)
{
  Answer() = QByteArray("RTSP/1.0 " + QByteArray::number(code) + " " + text + "\r\n");
  if (Headers().contains("CSeq")) {
    Answer().append("CSeq: " + Headers()["CSeq"] + "\r\n");
  }
  QLocale locale = QLocale(QLocale::English, QLocale::AnyCountry);
  Answer().append("Date: " + locale.toString(QDateTime::currentDateTimeUtc(), "ddd, dd MMM yyyy hh:mm:ss") + " GMT\r\n");
  Answer().append("Server: " + mRtspServer->ServerName() + "\r\n");
  Answer().append("User-Agent: " + mRtspServer->UserAgent() + "\r\n");
  Answer().append("\r\n");
}

void RtspHandler::SendFrame(const QByteArray& data)
{
  //static int gCount = 0;
  //if (gCount < 10) {
  //  char filename[] = "!ch0.bin";
  //  filename[3] = (char)('0' + gCount);
  //  FILE* file = fopen(filename, "wb");
  //  fwrite(data.constData(), data.size(), 1, file);
  //  fclose(file);
  //  gCount++;
  //}
  SendData(data);
}

void RtspHandler::ClearFrames()
{
  ClearData();
}

bool RtspHandler::PlayFake(QByteArray channelId, QString mediaName, const HeaderMap& headers)
{
  Log.Info("-------------=========== PLAY FAKE =========----------------");
  RtspMediaS rtspMedia;
  RtspChannelS rtspChannel;

  const MediaPath& mediaPath = mRtspServer->FindMediaPath(mediaName);
  if (!mediaPath.MediaLink) {
    Log.Warning(QString("%1 request not existed media (stream: '%2')").arg("Play fake").arg(mediaName));
    return false;
  } else if (mediaPath.MediaIndex >= 0) {
    Log.Info("Only aggregate operation allowed");
    return false;
  }
  rtspMedia = mediaPath.MediaLink.toStrongRef();
  if (!rtspMedia) {
    Log.Info("Not Found");
    return false;
  }

  rtspChannel = rtspMedia->FindChannel(channelId);
  if (!rtspChannel) {
    Log.Warning(QString("%1 session not found (stream: '%2', session: %3)").arg("Play fake").arg(mediaName).arg(channelId.constData()));
    return false;
  }

  if (headers.isEmpty()) {
    rtspChannel->GetMediaPlayer()->Pause();
    return true;
  }
  QByteArray extraHeaders;
  rtspChannel->GetMediaPlayer()->Play(headers, extraHeaders);
  Log.Info("-------------=========== PLAY FAKE DONE =========----------------");
  return true;
}


RtspHandler::RtspHandler(RtspServer* _RtspServer)
  : mRtspServer(_RtspServer)
  , mInterleaveChannel(-1), mInterleaveSize(0), mRtspReportItr(0)
  , mAntiSpam(true), mRequestCommand(-1), mRequestCount(0)
{
  SetKeepAlive(true);
  SetLogAll(true);
}

RtspHandler::~RtspHandler()
{
}


