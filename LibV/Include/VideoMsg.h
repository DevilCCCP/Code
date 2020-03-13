#pragma once

#include "Frame.h"


const int kConfirmFramesMs = 5000;

enum VideoMsgType {
  // from client
  eMsgLiveRequest = 1000,
  eMsgArchRequest,
  eMsgContinue,
  eMsgStop,
  eMsgThumbnail,
  eMsgMediaInfo,
  eMsgPtzRequest,
  // to client
  eMsgPlayGranted,
  eMsgPlayDenied,
  eMsgPlayDropped,
  eMsgPlayAborted,
  eMsgPlayNoStorage,
  eMsgThumbnailOk,
  eMsgThumbnailNo,
  eMsgSpsPps,
  eMsgNoMediaInfo,
  eMsgOneFrame,
  eMsgPtzRespond,
  // frame
  eMsgVideoInfo = 1100,
  eMsgVideoFrame
};

#pragma pack(push, 2)

struct LiveRequest
{
  int    CameraId;
  int    Priority;
};

struct ArchRequest
{
  int    CameraId;
  int    Priority;
  qint64 Timestamp;
  int    SpeedNum;
  int    SpeedDenum;
};

struct VideoInfo
{
  qint64 StartTimestamp;
};

#pragma pack(pop)
