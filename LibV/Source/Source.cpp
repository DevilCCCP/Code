#include <QMutexLocker>

#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Common/Format.h>
#include <Lib/Log/Log.h>
#include <LibV/Decoder/Thumbnail.h>

#include "Source.h"
#include "SourceState.h"
#include "SourceChild.h"
#ifdef USE_LIVE555
#include "SourceLive555/SourceLive.h"
#endif
#include "SourceFfmpeg/SourceFfmpeg.h"
#include "SourceScript/SourceScript.h"
#ifdef USE_V4L
#include "SourceV4l/SourceV4l.h"
#endif
#include "SourceProc/SourceProc.h"

const int kFixerMaxDiffMs = 5000;
const int kFixerMaxLogDiffMs = 5000;

bool Source::DoInit()
{
  ResetFixer();
  mLastStatus.start();
  mSourceState = SourceStateS(new SourceState());
  if (!mQuiet) {
    mSourceState->Init(GetOverseer()->Id());
  }
  return true;
}

void Source::OnStatus(Connection::EStatus status)
{
  if (status == Connection::eConnected) {
    CheckTimeout(status, kFramesLostTimeoutMs);
  } else if (status == Connection::eConnecting) {
    CheckTimeout(status, kStreamOpenTimeoutMs);
  }

  SwitchStatus(status);

  if (status != Connection::eConnected && mLastStatus.elapsed() > kStatusPeriodMs) {
    FrameS frame(new Frame());
    frame->ReserveData(sizeof(Frame::StatusHeader));
    Frame::StatusHeader* header = frame->GetStatusHeader();
    header->Size = sizeof(Frame::StatusHeader);
    header->HeaderSize = sizeof(Frame::StatusHeader);
    header->Status = status;

    EmergeVFrame(frame);
    mLastStatus.restart();
  }
}

void Source::OnFrame(const FrameS &frame)
{
  QMutexLocker lock(&mMutex);
  if (frame->GetHeader()->VideoDataSize == 0) {
    if (mUseAudio) {
      mExtraFrames.append(frame);
    }
    return;
  }

  while (mExtraFrames.size() > 0) {
    FrameS frameEx = mExtraFrames.takeFirst();
    if (frame->GetHeader()->CompressionAudio == eCmprNone) {
      frame->GetHeader()->CompressionAudio = frameEx->GetHeader()->CompressionAudio;
    } else if (frame->GetHeader()->CompressionAudio != frameEx->GetHeader()->CompressionAudio) {
      Log.Warning(QString("Audio compression differ frame skipped (first: %1, next: %2)")
                  .arg(frame->GetHeader()->CompressionAudio).arg(frameEx->GetHeader()->CompressionAudio));
      continue;
    }
    frame->ReserveData(frame->Size() + frameEx->AudioDataSize() + frameEx->ObjectDataSize());
    memcpy(frame->AudioData() + frame->AudioDataSize(), frameEx->AudioData(), frameEx->AudioDataSize());
    frame->GetHeader()->AudioDataSize += frameEx->AudioDataSize();
    frame->GetHeader()->Size += frameEx->AudioDataSize();
  }
  lock.unlock();

  FixFrameTimestamp(frame->GetHeader()->Timestamp);
  mLastFrame.restart();

  //static qint64 gTsBase = frame->GetHeader()->Timestamp;
  //static qint64 gTsLast = frame->GetHeader()->Timestamp;
  //Log.Trace(QString("EmergeVFrame (dt: %1, ts: %2, key: %3)")
  //          .arg(frame->GetHeader()->Timestamp - gTsLast).arg(frame->GetHeader()->Timestamp - gTsBase).arg((frame->GetHeader()->Key)? "y": "n"));
  //gTsLast = frame->GetHeader()->Timestamp;

  EmergeVFrame(frame);
  SwitchStatus(Connection::eConnected);
}

void Source::ResetFixer()
{
  mTimestampFixer = 0;
  mLastTimestamp = QDateTime::currentMSecsSinceEpoch();
  mLastFrame.start();
}

void Source::FixFrameTimestamp(qint64& timestamp)
{
  timestamp += mTimestampFixer;
  qint64 now = QDateTime::currentMSecsSinceEpoch();
  //Log.Trace(QString("%1 (%2) now: %3 (%4)").arg(timestamp).arg(QDateTime::fromMSecsSinceEpoch(timestamp).toString("hh:mm:ss.zzz"))
  //                   .arg(now).arg(QDateTime::fromMSecsSinceEpoch(now).toString("hh:mm:ss.zzz")));
  bool ok1 = timestamp > mLastTimestamp;
  bool ok2 = timestamp < mLastTimestamp + kFixerMaxDiffMs;
  if (!(ok1 && ok2)) {
    qint64 newFixer = now - (timestamp - mTimestampFixer);
    if (qAbs(newFixer - mLastFixer) > kFixerMaxLogDiffMs) {
      Log.Info(QString("Set timestamp fixer (abs: '%1', delta: %2, rel: %3, q: %4%5)").arg(QDateTime::fromMSecsSinceEpoch(newFixer).toString())
               .arg(FormatTimeDelta(newFixer - mTimestampFixer)).arg(FormatTimeDelta(mTimestampFixer - now)).arg(ok1? "Y": "N").arg(ok2? "Y": "N"));
      mLastFixer = newFixer;
    }
    timestamp = now;
    mTimestampFixer = newFixer;
  }
  mLastTimestamp = timestamp;
}

void Source::CheckTimeout(Connection::EStatus &status, int timeoutMs)
{
  if (mStatus != status) {
    mLastFrame.restart();
  } else if (mLastFrame.elapsed() > timeoutMs) {
    status = Connection::eNoFrames;
    ResetFixer();
    Reconnect();
  }
}

void Source::SwitchStatus(Connection::EStatus status)
{
  QMutexLocker lock(&mMutex);
  if (!mQuiet) {
    mSourceState->UpdateObjectState(status == Connection::eConnected);
  }

  if (mStatus != status) {
    mStatus = status;
    lock.unlock();
    Log.Info(QString("Source %1").arg(Connection::StatusToString(mStatus)));
  }
}

SourceS Source::CreateSource(SettingsA &settings, bool quiet, bool thumbnail)
{
  QString uri = settings.GetMandatoryValue("Uri", true).toString();
  bool useAudio = settings.GetValue("Audio", false).toBool();
  SourceS source;
  if (uri.startsWith("rtsp")) {
#ifdef USE_LIVE555
    int module = settings.GetValue("Module", 0).toInt();
    switch (module) {
    case 0:
      source = SourceS(new SourceLive(settings));
      break;
    case 1:
      source = SourceS(new SourceFfmpeg(settings));
      break;
    default: Log.Warning("Unknown source module, use live555");
      source = SourceS(new SourceLive(settings));
      break;
    }
#else
    source = SourceS(new SourceFfmpeg(settings));
#endif
  } else if (uri.startsWith("file")) {
    source = SourceS(new SourceFfmpeg(settings));
  } else if (uri.startsWith("proc")) {
    source = SourceS(new SourceProc(settings));
  } else if (uri.startsWith("usb")) {
    source = SourceS(new SourceFfmpeg(settings));
#ifdef USE_V4L
  } else if (uri.startsWith("v4l")) {
    source = SourceS(new SourceV4l(settings));
#endif
  } else if (uri.startsWith("script")) {
    source = SourceS(new SourceScript(settings));
  } else {
    Log.Fatal(QString("Can't create source (uri: %1)").arg(uri), true);
  }
  source->mUseAudio = useAudio;
  source->mQuiet = quiet;
  if (thumbnail) {
    source->mThumbnail.reset(new Thumbnail());
  }
  return source;
}

SourceS Source::CreateChildSource()
{
  return SourceS(new SourceChild());
}

Source::Source(int _WorkPeriodMs)
  : ConveyorV(_WorkPeriodMs)
  , mQuiet(false), mUseAudio(true)
  , mStatus(Connection::eNone), mLastFixer(0)
{
}

Source::~Source()
{
}
