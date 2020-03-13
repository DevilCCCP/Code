#pragma once

#include <QElapsedTimer>

#include <Lib/Common/FpsCalc.h>

#include "../MediaPackager.h"


class RtspPackager: public MediaPackager
{
  // Rtp
  QElapsedTimer           mReportTimer;
  quint16                 mSequenceNumber;
  QByteArray              mSyncId;
  quint32                 mH264Timestamp;
  FpsCalc                 mFpsCalc;

public:
  /*override */virtual void InFrame(const qint64& timestamp, const char* data, int size) Q_DECL_OVERRIDE;

public:
  void Init(const QByteArray& _SyncId);

private:
  bool CreateRtpFrames(const char* data, int size);
  void AddRtpFrames(const char* data, int size);
  void AddRtpHeader(QByteArray& rtpData, bool mark);

  bool CreateRtcpFrames();

public:
  explicit RtspPackager();
  /*override */virtual ~RtspPackager();
};
