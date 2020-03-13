#pragma once

#include <QByteArray>

#include <Lib/NetServer/HttpHandler.h>
#include <Lib/NetServer/NetServer.h>
#include <Lib/NetServer/HandlerManager.h>


DefineClassS(RtspHandler);
DefineClassS(RtspServer);
DefineClassS(RtspMedia);
DefineClassS(RtspChannel);

class RtspHandler: public HttpHandler
{
  RtspServer*       mRtspServer;
  RtspChannelW      mRtspChannelOut;
  RtspChannelW      mRtspChannelIn;

  int               mInterleaveChannel;
  int               mInterleaveSize;
  int               mRtspReportItr;

  bool              mAntiSpam;
  int               mRequestCommand;
  int               mRequestCount;
  QList<QByteArray> mPlayOption;

public:
  void SetAntiSpam(bool _AntiSpam) { mAntiSpam = _AntiSpam; }
  void SetOutChannel(RtspChannelS _RtspChannelOut) { mRtspChannelOut = _RtspChannelOut; }
  void SetInChannel(RtspChannelS _RtspChannelIn) { mRtspChannelIn = _RtspChannelIn; }

protected: /*Handler*/
  /*override */virtual bool Receive(SyncSocket* socket, bool& sendDone) Q_DECL_OVERRIDE; // false: disconnect
  /*override */virtual void OnDisconnected() Q_DECL_OVERRIDE;

protected: /*HttpHandler*/
  /*override */virtual const char* Protocol() Q_DECL_OVERRIDE { return "RTSP"; }

  /*override */virtual bool Options(const QString& path) Q_DECL_OVERRIDE;

//  /*override */virtual bool Announce(const QString& path, const QList<File>& files) Q_DECL_OVERRIDE;

  /*override */virtual bool Describe(const QString& path) Q_DECL_OVERRIDE;

  /*override */virtual bool Setup(const QString& path) Q_DECL_OVERRIDE;

//  /*override */virtual bool Record(const QString& path) Q_DECL_OVERRIDE;

  /*override */virtual bool Play(const QString& path) Q_DECL_OVERRIDE;

  /*override */virtual bool Pause(const QString& path) Q_DECL_OVERRIDE;

  /*override */virtual bool GetParameter(const QString& path, const QList<File>& files) Q_DECL_OVERRIDE;

  /*override */virtual bool Teardown(const QString& path) Q_DECL_OVERRIDE;

private:
  void AntiSpam(int cmdId, int value = 1);

  QString GetMediaPath(const QString& path);
  bool GetMediaChannel(const char* cmd, const QString& path, QByteArray& channelId, RtspMediaS& rtspMedia, RtspChannelS& rtspChannel);

  bool ReceiveRtspInterleaved();
  bool ReceiveRtspReport();

  bool FindChannel(const RtspMediaS& rtspMedia, QByteArray& channelId);

  void PrepareAnswer();
  void ErrorAnswer(int code = 404, const char* text = "Client error");

public:
  void SendFrame(const QByteArray& data);
  void ClearFrames();
  bool PlayFake(QByteArray channelId, QString mediaName, const HeaderMap& headers);

public:
  explicit RtspHandler(RtspServer* _RtspServer);
  /*override */virtual ~RtspHandler();
};
