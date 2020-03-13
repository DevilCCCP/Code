#pragma once

#include <QElapsedTimer>
#include <QSharedPointer>
#include <QThread>
#include <QMutex>

#include <Lib/Settings/SettingsA.h>
#include <LibV/Include/Frame.h>
#include "../Source.h"
#include "Def.h"


DefineClassS(SourceLive);

DefineClassS(EventLoop);
class EventLoop: public ConveyorV
{
  SourceLive* mSourceLive;

protected:
  /*override */virtual void run() Q_DECL_OVERRIDE;

public:
  void OnFrame(const FrameS& frame) { EmergeVFrame(frame); }

public:
  EventLoop(SourceLive* _SourceLive): mSourceLive(_SourceLive) { }
};

class SourceLive: public Source
{
  const static QString kVideoStream;
  const static QString kAudioStream;

  QString       mUri;
  QString       mLogin;
  QString       mPassword;
  bool          mTcp;

  // live555 internal classes
  RtspClientExS           mRtspClient;
  BasicUsageEnvironment*  mUsageEnvironment;
  TaskSchedulerExS        mScheduler;
  AuthenticatorS          mAuthenticator;
  MediaSessionExS         mMediaSession;
  char                    mSchedulerWatch;

  // thread to straighten live555 spaghetti code
  EventLoopS              mEventLoop;      // поток отвечающий за обработку RTSP ответов от источника
  bool                    mEventLoopStop;  // флаг остановки потока mEventLoop (внутри live555 есть свой цикл обработки, вылетающий при ошибке, нам же интересна только остановка по требованию, за которую отвечает данный флаг)
  QMutex                  mEventLoopMutex; // синхр. чтения/записи флага
  RtspCmd                 mCmdDESCRIBE;    // результат выполнения команды (ответ)
  RtspCmd                 mCmdSETUP;       // результат выполнения команды (ответ)
  RtspCmd                 mCmdDefault;     // результат выполнения команды (ответ)

  QElapsedTimer           mWaitRespondTimer;

  // callback to straighten live555 spaghetti code
  friend void CallbackX(RtspCmd& cmd, int resultCode, char* resultString);
  friend void FinalizeResults(SourceLive* _SourceLive, int resultCode, char* resultString);
  friend void CallbackDESCRIBE(RTSPClient* client, int resultCode, char* resultString);
  friend void CallbackSETUP(RTSPClient* client, int resultCode, char* resultString);
  friend void CallbackDefault(RTSPClient* client, int resultCode, char* resultString);

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "SourceRTSP"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "S"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;

  /*override */virtual void Stop() Q_DECL_OVERRIDE;

protected:
  /*override */virtual bool ProcessFrame() Q_DECL_OVERRIDE;

protected:
  /*override */virtual void Reconnect() Q_DECL_OVERRIDE;

private:
  bool ConnectSource();
  bool PlaySource(); // цикл ожидания прерываний от пользователя
  bool StopSource();
  bool InitMediaSession(const QString& sdpDescription);
  bool SetupSubsession(MediaSubsession* subsession);
  void Cleanup();

  void EventLoopEx();    // цикл приёма ответов от RTSP источника
  bool CheckEventLoop(); // проверка есть ли прерывание от пользователя
  void Break();          // прерывание от пользователя
  bool Continue();       // восстановление возможности повторного запуска

  bool InitCommand(RtspCmd& cmd);
  bool WaitCommandResult(RtspCmd& cmd);

public:
  SourceLive(SettingsA& settings);
  /*override */virtual ~SourceLive();

  friend class EventLoop;
};
