#pragma once

#include <QUdpSocket>
#include <QMutex>
#include <QVector>

#include "../Channel.h"


DefineClassS(RtspChannel);
DefineClassS(RtspHandler);
DefineClassS(MediaPlayer);
DefineClassS(QUdpSocket);

class RtspChannel: public Channel
{
  struct StreamPortMap {
    int DataPort;
    int ControlPort;
  };
  typedef QVector<StreamPortMap> PortMap;

  struct UdpConnection: public StreamPortMap {
    QUdpSocketS DataSocket;
    QUdpSocketS ControlSocket;
  };
  typedef QVector<UdpConnection> UdpMap;

  QByteArray     mId;
  PortMap        mPortMap;
  UdpMap         mUdpMap;
  QUdpSocketS    mOutSocket;
  QHostAddress   mHostAddress;

  RtspHandler*   mHandler;
  RtspChannelS   mRtspChannelOut;
  QMutex         mHandlerMutex;

public:
  const QByteArray& Id() { return mId; }

protected:
//  /*override */virtual bool OnStart() override;
//  /*override */virtual bool OnTest() override;
  /*override */virtual void OnStop() override;

  /*override */virtual void OnFrame(const TrFrameS& frame) override;
  /*override */virtual void OnClearFrames() override;

public:
  void GetParameters(const QByteArray& dataIn, QByteArray& dataOut, QByteArray& extraHeader);

  bool SetIvPorts(int mediaIndex, int ch1, int ch2);
  bool GetIvPorts(int mediaIndex, int& ch1, int& ch2);

  bool SetUdp(const QHostAddress& _HostAddress, int mediaIndex, int ch1, int ch2, int& bch1, int& bch2);

  void SetHandler(RtspHandler* _Handler);
  void SetOutChannel(const RtspChannelS& _RtspChannelOut);

public:
  explicit RtspChannel(Media* _Media, QByteArray _Id, const MediaPlayerS& _MediaPlayer);
  /*override */virtual ~RtspChannel();
};

