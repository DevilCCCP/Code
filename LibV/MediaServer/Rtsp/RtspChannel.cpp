#include <QMutexLocker>

#include <Lib/Log/Log.h>
#include <Lib/Common/Format.h>

#include "RtspChannel.h"
#include "RtspHandler.h"
#include "../TrFrame.h"
#include "../MediaPlayer.h"


void RtspChannel::OnStop()
{
  mOutSocket.clear();
  mUdpMap.clear();
}

void RtspChannel::OnFrame(const TrFrameS& frame)
{
  QMutexLocker lock(&mHandlerMutex);
  if (mHandler) {
    int ch = frame->mType >> 1;
    int type = frame->mType & 1;
    if (ch >= 0 && ch < mPortMap.size()) {
      int index = (type)? mPortMap[ch].ControlPort: mPortMap[ch].DataPort;
      QByteArray data;
      data.append('$');
      data.append(index);
      quint16 sz = frame->mData.size();
      data.append((char)(uchar)(sz >> 8));
      data.append((char)(uchar)(sz & 0xff));
      data.append(frame->mData);
      mHandler->SendFrame(data);
    }
  } else if (mRtspChannelOut) {
    mRtspChannelOut->OnFrame(frame);
  } else if (frame->mType >= 0 && frame->mType < 2*mUdpMap.size()) {
    int ch = frame->mType >> 1;
    int type = frame->mType & 1;
    int port = (type)? mUdpMap[ch].ControlPort: mUdpMap[ch].DataPort;
    mOutSocket->writeDatagram(frame->mData, mHostAddress, port);
  }
}

void RtspChannel::OnClearFrames()
{
  QMutexLocker lock(&mHandlerMutex);
  if (mHandler) {
    mHandler->ClearFrames();
  }
}

void RtspChannel::GetParameters(const QByteArray& dataIn, QByteArray& dataOut, QByteArray& extraHeader)
{
  if (const MediaPlayerS& mp = GetMediaPlayer()) {
    return mp->GetParameters(dataIn, dataOut, extraHeader);
  }
}

bool RtspChannel::SetIvPorts(int mediaIndex, int ch1, int ch2)
{
  if (mediaIndex < 0 || mediaIndex >= 8) {
    Log.Warning(QString("Rtsp channel: bad media setup (index: %1, port1: %2, port2: %3)").arg(mediaIndex).arg(ch1).arg(ch2));
    return false;
  }

  mPortMap.resize(mediaIndex + 1);
  mPortMap[mediaIndex].DataPort = ch1;
  mPortMap[mediaIndex].ControlPort = ch2;
  return true;
}

bool RtspChannel::GetIvPorts(int mediaIndex, int& ch1, int& ch2)
{
  if (mediaIndex < mPortMap.size()) {
    ch1 = mPortMap[mediaIndex].DataPort;
    ch2 = mPortMap[mediaIndex].ControlPort;
    return true;
  }
  return false;
}

bool RtspChannel::SetUdp(const QHostAddress& _HostAddress, int mediaIndex, int ch1, int ch2, int& bch1, int& bch2)
{
  mHostAddress = _HostAddress;
  if (mediaIndex < 0 || mediaIndex >= 8) {
    Log.Warning(QString("Rtsp channel: bad media setup (index: %1, port1: %2, port2: %3)").arg(mediaIndex).arg(ch1).arg(ch2));
    return false;
  }

  if (!mOutSocket) {
    mOutSocket = QUdpSocketS(new QUdpSocket());
  }
  mUdpMap.resize(mediaIndex + 1);
  UdpConnection* udpConnection = &mUdpMap[mediaIndex];
  udpConnection->DataPort = ch1;
  udpConnection->ControlPort = ch2;

  udpConnection->DataSocket = QUdpSocketS(new QUdpSocket());
  udpConnection->ControlSocket = QUdpSocketS(new QUdpSocket());
  qsrand(QDateTime::currentMSecsSinceEpoch());
  forever {
    bch1 = 5000 + 2 * (qrand() % 25000);
    bch2 = bch1 + 1;
    bool ok1 = udpConnection->DataSocket->bind(QHostAddress::AnyIPv4, bch1);
    bool ok2 = udpConnection->ControlSocket->bind(QHostAddress::AnyIPv4, bch2);
    if (ok1 && ok2) {
      break;
    }
  }
  return true;
}

void RtspChannel::SetHandler(RtspHandler* _Handler)
{
  QMutexLocker lock(&mHandlerMutex);
  mHandler = _Handler;
}

void RtspChannel::SetOutChannel(const RtspChannelS& _RtspChannelOut)
{
  QMutexLocker lock(&mHandlerMutex);
  mRtspChannelOut = _RtspChannelOut;
}


RtspChannel::RtspChannel(Media* _Media, QByteArray _Id, const MediaPlayerS& _MediaPlayer)
  : Channel(_Media, _MediaPlayer)
  , mId(_Id), mHandler(nullptr)
{
}

RtspChannel::~RtspChannel()
{
}

