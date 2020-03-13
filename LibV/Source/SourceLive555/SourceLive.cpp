#include <QByteArray>
#include <QMutexLocker>
#include <QSize>

#include <Lib/Ctrl/ManagerThread.h>
#include <Lib/Log/Log.h>

#include <H264VideoRTPSource.hh>
#include <MPEG4GenericRTPSource.hh>

#include "MediaSinkImpl.h"
#include "SourceLive.h"


const int kPlayCyrcleMs = 200;
const int kMaxWaitRespondMs = 10 * 1000;
const int kMaxFrameSize = 2 * 1024 * 1024 - 80;
const int kMaxFrameASize = 500 * 1024 - 20;
const QString SourceLive::kVideoStream("video");
const QString SourceLive::kAudioStream("audio");

void EventLoop::run()
{
  return mSourceLive->EventLoopEx();
}

// A function that is called in response to a RTSP command.  The parameters are as follows:
//     "rtspClient": The "RTSPClient" object on which the original command was issued.
//     "resultCode": If zero, then the command completed successfully.  If non-zero, then the command did not complete
//         successfully, and "resultCode" indicates the error, as follows:
//             A positive "resultCode" is a RTSP error code (for example, 404 means "not found")
//             A negative "resultCode" indicates a socket/network error; 0-"resultCode" is the standard "errno" code.
//     "resultString": A ('\0'-terminated) string returned along with the response, or else NULL.
//         In particular:
//             "resultString" for a successful "DESCRIBE" command will be the media session's SDP description.
//             "resultString" for a successful "OPTIONS" command will be a list of allowed commands.
//         Note that this string can be present (i.e., not NULL) even if "resultCode" is non-zero - i.e., an error message.
//         Also, "resultString" can be NULL, even if "resultCode" is zero (e.g., if the RTSP command succeeded, but without
//             including an appropriate result header).
//         Note also that this string is dynamically allocated, and must be freed by the handler (or the caller)
//             - using "delete[]".

// template for minimize copy-paste errors
void CallbackX(RtspCmd& cmd, int resultCode, char* resultString)
{
  if (cmd.Pending) {
    cmd.ResultCode = resultCode;
    if (resultString) {
      cmd.ResultString = resultString;
    } else {
      cmd.ResultString.clear();
    }
    cmd.Pending = false;
  }
}

void FinalizeResults(char* resultString)
{
  delete[] resultString;
}

void CallbackDESCRIBE(RTSPClient* client, int resultCode, char* resultString)
{
  RtspClientEx* rtspClient = static_cast<RtspClientEx*>(client);
  SourceLive* _SourceLive = rtspClient->GetSourceLive();
  CallbackX(_SourceLive->mCmdDESCRIBE, resultCode, resultString);
  return FinalizeResults(resultString);
}

void CallbackSETUP(RTSPClient* client, int resultCode, char* resultString)
{
  RtspClientEx* rtspClient = static_cast<RtspClientEx*>(client);
  SourceLive* _SourceLive = rtspClient->GetSourceLive();
  CallbackX(_SourceLive->mCmdSETUP, resultCode, resultString);
  return FinalizeResults(resultString);
}

void CallbackDefault(RTSPClient* client, int resultCode, char* resultString)
{
  RtspClientEx* rtspClient = static_cast<RtspClientEx*>(client);
  SourceLive* _SourceLive = rtspClient->GetSourceLive();
  CallbackX(_SourceLive->mCmdDefault, resultCode, resultString);
  return FinalizeResults(resultString);
}

bool SourceLive::DoInit()
{
  mEventLoop = EventLoopS(new EventLoop(this));
  mEventLoop->ConnectModule(this);

  return Source::DoInit();
}

bool SourceLive::DoCircle()
{
  if (ConnectSource()) {
    PlaySource();
    StopSource();
  }
  Cleanup();
  return true;
}

void SourceLive::DoRelease()
{
  //StopSource();
  //Cleanup();
}

void SourceLive::Stop()
{
  Source::Stop();
  Break();
}

bool SourceLive::ProcessFrame()
{
  OnFrame(CurrentVFrame());
  return true;
}

void SourceLive::Reconnect()
{
  Break();
}

bool SourceLive::ConnectSource()
{
  OnStatus(Connection::eConnecting);
  if (!mLogin.isEmpty()) {
    mAuthenticator = AuthenticatorS(new Authenticator(mLogin.toLatin1().constData(), mPassword.toLatin1().constData()));
  }

  mRtspClient = RtspClientExS(new RtspClientEx(this, *mUsageEnvironment, mUri.toLatin1().constData(), 1/*verbosityLevel*/, nullptr/*applicationName*/));

  // start ower loop for taking back answers
  Continue();

  // real init will stay behind that loopback (live555 spaghetti send - callback single threaded code)
  mEventLoop->start();

  InitCommand(mCmdDESCRIBE);
  mRtspClient->sendDescribeCommand(CallbackDESCRIBE, mAuthenticator.data());
  if (!WaitCommandResult(mCmdDESCRIBE)) {
    OnStatus(Connection::eDisconnected);
    return false;
  }

  if (mCmdDESCRIBE.ResultCode == 0) {
    Log.Info("DESCRIBE ok");
  } else {
    Log.Info(QString("DESCRIBE fail (code %1)").arg(mCmdDESCRIBE.ResultCode));
    OnStatus(Connection::eDisconnected);
    return false;
  }

  if (!InitMediaSession(mCmdDESCRIBE.ResultString)) {
    OnStatus(Connection::eDisconnected);
    return false;
  }

  OnStatus(Connection::eConnected);
  return true;
}

bool SourceLive::PlaySource()
{
  Log.Info("Play event loop");
  while (mEventLoop->isRunning()) {
    Rest(kPlayCyrcleMs);
    if (!ConveyorV::DoCircle()) {
      break;
    }
  }
  return true;
}

bool SourceLive::StopSource()
{
  InitCommand(mCmdDefault);
  mRtspClient->sendTeardownCommand(*mMediaSession, CallbackDefault);
  return WaitCommandResult(mCmdDefault);
}

bool SourceLive::InitMediaSession(const QString& sdpDescription)
{
  mMediaSession = MediaSessionExS(new MediaSessionEx(*mUsageEnvironment));
  if (!mMediaSession->InitializeWithSDP(sdpDescription.toLatin1().constData())) {
    Break();
  }

  bool init = false;
  MediaSubsessionIterator iter(*mMediaSession);
  for (auto subsession = iter.next(); subsession; subsession = iter.next()) {
    if (subsession->initiate(0)) {
      Log.Info(QString("codecName: %1, sprops: %2").arg(subsession->codecName()).arg(subsession->fmtp_spropparametersets()));

      InitCommand(mCmdSETUP);
      mRtspClient->sendSetupCommand(*subsession, CallbackSETUP, False, mTcp);
      if (!WaitCommandResult(mCmdSETUP)) {
        return false;
      }

      if (mCmdSETUP.ResultCode == 0) {
        Log.Info("SETUP ok");
      } else {
        Log.Info(QString("SETUP fail (code %1)").arg(mCmdSETUP.ResultCode));
        continue;
      }
      if (SetupSubsession(subsession)) {
        init = true;
      }
    }
  }

  if (init) {
    InitCommand(mCmdDefault);
    mRtspClient->sendPlayCommand(*mMediaSession, CallbackDefault);
    if (!WaitCommandResult(mCmdDefault)) {
      return false;
    }

    if (mCmdDefault.ResultCode == 0) {
      Log.Info("Play ok");
    } else {
      Log.Info(QString("Play fail (code %1)").arg(mCmdDefault.ResultCode));
      return false;
    }
  }
  return init;
}

bool SourceLive::SetupSubsession(MediaSubsession* subsession)
{
  auto rtpSource = subsession->rtpSource();
  QString codec = subsession->codecName();

  if (!rtpSource) {
    return false;
  }

  MediaSinkImpl* sink;
  if (subsession->mediumName() == kVideoStream) {
    if (codec == "H264") {
      const char* spropText = subsession->fmtp_spropparametersets();
      sink = new MediaSinkImpl(mEventLoop.data(), *mUsageEnvironment, kMaxFrameSize, eH264, spropText);
    } else if(codec == "JPEG") {
      sink = new MediaSinkImpl(mEventLoop.data(), *mUsageEnvironment, kMaxFrameSize, eJpeg);
    } else {
      return false;
    }
  } else if (subsession->mediumName() == kAudioStream) {
    if (codec == "MPEG4-GENERIC") {
      sink = new MediaSinkImpl(mEventLoop.data(), *mUsageEnvironment, kMaxFrameASize, eAac16b);
    } else {
      return false;
    }
  } else {
    sink = nullptr;
  }
  subsession->sink = sink;

  increaseReceiveBufferTo(*mUsageEnvironment, rtpSource->RTPgs()->socketNum(), kMaxFrameSize);
  subsession->sink = sink;

  auto source = subsession->readSource();
  return sink->startPlaying(*source, 0, 0);
}

void SourceLive::Cleanup()
{
  Log.Info("Clean source");
  Break();
  if (mMediaSession)
  {
    MediaSubsessionIterator iter(*mMediaSession);
    while (MediaSubsession* subsession = iter.next()) {
      if (subsession->sink) {
        subsession->sink->stopPlaying();
        MediaSink::close(subsession->sink);
        subsession->sink = nullptr;
      }
      subsession->deInitiate();
    }
    mMediaSession.clear();
  }

  mRtspClient.clear();
  for (int i = 0; i < 3 && !mEventLoop->wait(kPlayCyrcleMs); i++) {
    Log.Warning(QString("Can't join loopback (try: %1)").arg(i));
  }
}

void SourceLive::EventLoopEx()
{
  while (!CheckEventLoop()) {
    Log.Trace("do live555 event loop");
    mScheduler->doEventLoop(&mSchedulerWatch);
    Log.Trace("live555 event loop exited");
  }
  Log.Trace("Exit event loop");
}

bool SourceLive::CheckEventLoop()
{
  QMutexLocker lock(&mEventLoopMutex);
  if (mEventLoopStop) {
    return true;
  } else {
    mSchedulerWatch = 0;
    return false;
  }
}

void SourceLive::Break()
{
  Log.Trace("Event loop break");
  QMutexLocker lock(&mEventLoopMutex);
  mEventLoopStop = true;
  mSchedulerWatch = 1;
}

bool SourceLive::Continue()
{
  Log.Trace("Event loop continue");
  QMutexLocker lock(&mEventLoopMutex);
  if (IsAlive()) {
    mEventLoopStop = false;
    mSchedulerWatch = 0;
    return true;
  } else {
    return false;
  }
}

bool SourceLive::InitCommand(RtspCmd &cmd)
{
  cmd.Pending = true;
  return true;
}

bool SourceLive::WaitCommandResult(RtspCmd &cmd)
{
  mWaitRespondTimer.start();
  while (cmd.Pending && mWaitRespondTimer.elapsed() < kMaxWaitRespondMs) {
    if (IsStop() || mEventLoopStop) {
      return false;
    }
    msleep(1);
  }
  return true;
}

SourceLive::SourceLive(SettingsA &settings)
  : Source()
  , mUsageEnvironment(nullptr), mScheduler(new TaskSchedulerEx())
{
  mUri = settings.GetMandatoryValue("Uri").toString();
  mLogin = settings.GetValue("Login", "").toString();
  mPassword = settings.GetValue("Password", "").toString();
  mTcp = settings.GetValue("Transport", "0").toBool() == 0;

  mUsageEnvironment = BasicUsageEnvironment::createNew(*mScheduler);
}

SourceLive::~SourceLive()
{
  if (mUsageEnvironment) {
    mUsageEnvironment->reclaim();
    mUsageEnvironment = nullptr;
  }
}

