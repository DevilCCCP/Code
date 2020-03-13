#include <QMutexLocker>

#include <Lib/Common/Profiler.h>
#include <Lib/Log/Log.h>
#include <Lib/Common/FpsDown.h>
#include <LibV/Include/Frame.h>

#include "CodecA.h"


#ifdef QT_DEBUG
const int kProfileDumpPeriodMs = 10 * 1000;
#else
const int kProfileDumpPeriodMs = 30 * 60 * 1000;
#endif

void CodecA::OnDecodedFrame(const FrameS& frame)
{
  if (!frame) {
    FrameS emptyFrame(new Frame());
    emptyFrame->ReserveData(0);

    Frame::Header* header = emptyFrame->InitHeader();
    header->Timestamp = 0;
    header->Key = true;
    header->Compression = mDestCompression;
    header->VideoDataSize = 0;

    QMutexLocker lock(&mFramesMutex);
    mFramesOut.append(emptyFrame);
  } else {
    QMutexLocker lock(&mFramesMutex);
    mFramesOut.append(frame);
  }
}

void CodecA::DecodeFrame(const FrameS& frame, bool audioVideo)
{
  bool canProfile = CanProfile();
  if (canProfile) {
    mProfiler->Start();
  }

  qint64 ts = frame->GetHeader()->Timestamp;

  if (!mLastTimestamp) {
    mLastTimestamp = ts;
  }

  bool todo = mFpsDown->TakeFrame(ts - mLastTimestamp);
  bool ok = (audioVideo)? DecodeVideoFrame(frame, !todo): DecodeAudioFrame(frame, !todo);

  if (canProfile) {
    mProfiler->Pause();
    mProfiler->AutoDump(kProfileDumpPeriodMs);
  }

  if (ok) {
    mLastTimestamp = ts;
    mFramesIn.append(frame);
  }
}

bool CodecA::GetDecodedFrame(FrameS& destFrame)
{
  QMutexLocker lock(&mFramesMutex);
  if (!mFramesOut.isEmpty()) {
    destFrame = mFramesOut.takeFirst();
    if (!mFramesIn.isEmpty()) {
      FrameS srcFrame = mFramesIn.takeFirst();
      destFrame->GetHeader()->Timestamp = srcFrame->Timestamp();
      destFrame->ReserveData(destFrame->Size() + srcFrame->ObjectDataSize());
      destFrame->GetHeader()->ObjectDataSize = srcFrame->ObjectDataSize();
      destFrame->GetHeader()->Size = destFrame->Size() + srcFrame->ObjectDataSize();
      memcpy(destFrame->ObjectData(), srcFrame->ObjectData(), srcFrame->ObjectDataSize());
      return true;
    } else {
      Log.Warning(QString("No timestamp for new frame"));
    }
  }
  return false;
}


CodecA::CodecA(ECompression _DestCompression, int _Fps, bool _UseHardware)
  : mDestCompression(_DestCompression), mUseHardware(_UseHardware)
  , mProfiler(new Profiler((mDestCompression & eAnyAudio)? "Audio decoder": "Video decoder")), mFpsDown(new FpsDown(_Fps))
  , mLastTimestamp(0)
{
}
