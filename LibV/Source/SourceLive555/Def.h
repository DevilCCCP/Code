#pragma once

#include <QString>

#include <Lib/Include/Common.h>

#include <liveMedia_version.hh>
#include <BasicUsageEnvironment.hh>
#include <RTSPClient.hh>
#include <MediaSession.hh>
#include <DigestAuthentication.hh>
#include <GroupsockHelper.hh>

DefineClassS(SourceLive);
DefineClassS(RtspClientEx);
DefineClassS(MediaSessionEx);
DefineClassS(TaskSchedulerEx);
DefineClassS(Authenticator);

class RtspClientEx: public RTSPClient
{
  SourceLive* mSourceLive;

public:
  SourceLive* GetSourceLive() { return mSourceLive; }

  RtspClientEx(SourceLive* _SourceLive, UsageEnvironment& env, char const* rtspURL
               , int verbosityLevel, char const* applicationName = nullptr, portNumBits tunnelOverHTTPPortNum = 0)
#if LIVEMEDIA_LIBRARY_VERSION_INT >= 1389571200
    : RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1)
#else
    : RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum)
#endif
    , mSourceLive(_SourceLive)
  { }
  ~RtspClientEx() { }

  friend class SourceLive;
};

class MediaSessionEx: public MediaSession
{
public:
  bool InitializeWithSDP(const char* sdpDescription) { return initializeWithSDP(sdpDescription); }

  MediaSessionEx(UsageEnvironment& env): MediaSession(env) { }
  ~MediaSessionEx() { }
};

class TaskSchedulerEx: public BasicTaskScheduler
{
public:
#if LIVEMEDIA_LIBRARY_VERSION_INT >= 1389571200
  TaskSchedulerEx(unsigned maxSchedulerGranularity = 10000): BasicTaskScheduler(maxSchedulerGranularity) { }
#else
  TaskSchedulerEx(): BasicTaskScheduler() { }
#endif
  ~TaskSchedulerEx() { }
};

struct RtspCmd
{
  volatile bool Pending;
  int           ResultCode;
  QString       ResultString;

  void SetResult(int resultCode, char* resultString) { ResultCode = resultCode; ResultString = resultString; }
};

