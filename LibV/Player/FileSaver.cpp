#include <Lib/Log/Log.h>
#include <Lib/Dispatcher/Overseer.h>

#include "FileSaver.h"
#include "Ffmpeg/FfmpegOut.h"


bool FileSaver::DoInit()
{
  mFfmpegOut = FfmpegOutS(new FfmpegOut);
  if (!mFfmpegOut->Open(mFilename)) {
    Done();
    return false;
  }
  mOpened = true;
  return true;
}

void FileSaver::DoRelease()
{
  if (mOpened) {
    mFfmpegOut->Close();
  }
}

//void FileSaver::Stop()
//{
//  ConveyorV::Stop();
//}

bool FileSaver::ProcessFrame()
{
  if (FrameS currentFrame = CurrentVFrame()) {
    if (currentFrame->GetHeader()->HeaderSize == sizeof(Frame::Header)) {
      if (!mFfmpegOut->WriteNext(currentFrame)) {
        Done();
      }
      return true;
    }
  }

  mFfmpegOut->Close();
  Done();
  return true;
}

void FileSaver::OnOverflow(QList<FrameAS>& conveyorFrames)
{
  Q_UNUSED(conveyorFrames);

  LOG_WARNING_ONCE("Saving overflow");
}

void FileSaver::Done()
{
  if (mParent) {
    mParent->Stop();
    Stop();
  } else {
    GetOverseer()->Done();
  }
}


FileSaver::FileSaver(const QString& _Filename, CtrlWorker* _Parent)
  : mFilename(_Filename), mParent(_Parent)
  , mOpened(false)
{
}

FileSaver::~FileSaver()
{
}
