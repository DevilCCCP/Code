#include <QtEndian>
#include <QDateTime>
#include <QByteArray>

#include <Lib/Log/Log.h>
#include <LibV/MediaServer/H264/H264NalUnit.h>
#include <LibV/MediaServer/TrFrame.h>

#include "RtspPackager.h"


#define LogLive(X)
//#define LogLive(X) Log.Trace(X)
//#define LogLive(X) Log.Info(X)
const int kPayloadSize = 1386;
const int kReportPeriodMs = 2000;

template<typename TypeT>
inline void WriteNetworkOrder(QByteArray& store, TypeT value)
{
  int pos = store.size();
  store.resize(pos + sizeof(TypeT));
  qToBigEndian(value, (uchar*)store.data() + pos);
}

void RtspPackager::InFrame(const qint64& timestamp, const char* data, int size)
{
  mFpsCalc.AddFrame();
  mH264Timestamp = (quint32)(timestamp * 90);

  //if (size > 20000) {
  //int yyy = 1;
  //static qint64 gBaseTs = 0;
  //static qint64 gLastTs = 0;
  //if (qAbs(gBaseTs - mH264Timestamp) > 3600*90000) {
  //  gBaseTs = mH264Timestamp;
  //  gLastTs = mH264Timestamp;
  //  Log.Trace(QString("Set base ts: %1").arg(gBaseTs));
  //}
  //QFile dump("C:/Temp/Rtsp/ts_dump.bin");
  //dump.open(QFile::Append);
  //dump.write(QString("%1;%2\n").arg((mH264Timestamp - gBaseTs) / 90).arg((mH264Timestamp - gLastTs) / 90).toLatin1());
  //gLastTs = mH264Timestamp;
  //}

//  static qint64 gLastTs = timestamp;
//  LogLive(QString("CreateRtpFrames (ts: %1(%2)+%3, sz: %4)").arg(timestamp).arg(mH264Timestamp).arg(timestamp - gLastTs).arg(size));
//  Log.Trace(QString("RtpFrame %1,%2,%3,%4").arg(QDateTime::fromMSecsSinceEpoch(timestamp).toString("hh:mm:ss.zzz")).arg(timestamp).arg(mH264Timestamp).arg(size));
  CreateRtpFrames(data, size);
//  gLastTs = timestamp;

  if (mReportTimer.elapsed() > kReportPeriodMs) {
    LogLive(QString("CreateRtcpFrames (ts: %1, sz: %2)").arg(timestamp).arg(size));
//    Log.Trace(QString("Package FPS: %1").arg(mFpsCalc.Format()));
    if (CreateRtcpFrames()) {
      mReportTimer.restart();
    }
  }
}

void RtspPackager::Init(const QByteArray& _SyncId)
{
  mReportTimer.start();
  mSequenceNumber = 0;
  mSyncId = _SyncId;
  LogLive(QString("Init"));
}

bool RtspPackager::CreateRtpFrames(const char* data, int size)
{
  H264NalUnit nalu(data, size);
  while (nalu.FindNext()) {
    AddRtpFrames(nalu.CurrentUnit(), nalu.CurrentUnitSize());
  }
  return true;
}

void RtspPackager::AddRtpFrames(const char* data, int size)
{
  uchar type = ((uchar)*data) & 0x1f;
  uchar idc = ((uchar)*data) & 0x60;
  data++; size--;

  if (size <= kPayloadSize) {
    QByteArray rtp;
    AddRtpHeader(rtp, true);
    rtp.append((char)(type | idc));
    rtp.append(data, size);

    AddFrame(TrFrameS(new TrFrame(0, rtp)));
    return;
  }

  bool first = true;
  while (size > 0) {
    bool last = (size <= kPayloadSize);
    QByteArray rtp;
    AddRtpHeader(rtp, last);

    rtp.append((char)(idc | (uchar)0x1c));
    if (first) {
      first = false;
      rtp.append((char)(type | 0x80));
    } else if (last) {
      rtp.append((char)(type | 0x40));
    } else {
      rtp.append((char)(type));
    }
    if (size >= kPayloadSize) {
      rtp.append(data, kPayloadSize);
    } else {
      rtp.append(data, size);
    }

    AddFrame(TrFrameS(new TrFrame(0, rtp)));

    data += kPayloadSize;
    size -= kPayloadSize;
  }
}

void RtspPackager::AddRtpHeader(QByteArray& rtpData, bool mark)
{
  rtpData.append((char)0x80);
  rtpData.append((char)(uint)(mark? 0x80 + 96: 96));
  WriteNetworkOrder(rtpData, mSequenceNumber);
  WriteNetworkOrder(rtpData, mH264Timestamp);
  rtpData.append(mSyncId);

  mSequenceNumber++;
}

bool RtspPackager::CreateRtcpFrames()
{
  QByteArray rtcp; // rfc3550
  rtcp.append((char)(uchar)0x80); // 6.4.1 SR: Sender Report RTCP Packet
  rtcp.append((char)(uchar)0xc8); // packet type (PT): 8 bits (Contains the constant 200 to identify this as an RTCP SR packet.)
  rtcp.append((char)(uchar)0x00); // length: 16 bits (The length of this RTCP packet in 32-bit words minus one,
  rtcp.append((char)(uchar)0x06); //                  including the header and any padding)
  rtcp.append(mSyncId);

  static QDateTime kBeforeTimeMs = QDateTime(QDate(1900, 1, 1));
  QDateTime curTime = QDateTime::currentDateTime();
  quint32 tsLsf = (quint32)((qint64)curTime.time().msec() * 0xffffffff / 1000);
  curTime.setTime(QTime(curTime.time().hour(), curTime.time().minute(), curTime.time().second()));
  quint32 tsMsf = (quint32)(kBeforeTimeMs.msecsTo(curTime) / 1000LL);
  WriteNetworkOrder(rtcp, tsMsf);
  WriteNetworkOrder(rtcp, tsLsf);
  WriteNetworkOrder(rtcp, mH264Timestamp);

  rtcp.append((char)(uchar)0x81); // 6.5 SDES: Source Description RTCP Packet
  rtcp.append((char)(uchar)0xca); // packet type (PT): 8 bits (Contains the constant 202 to identify this as an RTCP SDES packet.)
  rtcp.append((char)(uchar)0x00); // length: 16 bits (The length of this RTCP packet in 32-bit words minus one,
  rtcp.append((char)(uchar)0x12); //                  including the header and any padding)
  rtcp.append(mSyncId);

  rtcp.append((char)(uchar)0x01); // 6.5.1 CNAME: Canonical End-Point Identifier SDES Item
  rtcp.append((char)(uchar)0x16);
  rtcp.append("root@yava-accc12345678");
  rtcp.append((char)(uchar)0x02); // 6.5.2 NAME: User Name SDES Item
  rtcp.append((char)(uchar)0x04);
  rtcp.append("root");
  rtcp.append((char)(uchar)0x06); // 6.5.6 TOOL: Application or Tool Name SDES Item
  rtcp.append((char)(uchar)0x09);
  rtcp.append("YAVA RTSP");
  rtcp.append((char)0);
  rtcp.append((char)0);
  rtcp.append((char)0);

  AddFrame(TrFrameS(new TrFrame(1, rtcp)));
  return true;
}


RtspPackager::RtspPackager()
{
}

RtspPackager::~RtspPackager()
{
}
