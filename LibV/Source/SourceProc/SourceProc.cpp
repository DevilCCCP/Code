#include <QDir>
#include <QDirIterator>
#include <QProcess>

#include <Lib/Log/Log.h>
#include <LibV/Decoder/Thumbnail.h>

#include "SourceProc.h"


const int kFileScanPeriod = 20;

bool SourceProc::DoCircle()
{
  CheckProcess();
  DoPath();
  return true;
}

void SourceProc::DoRelease()
{
  StopProcess();
}

void SourceProc::Reconnect()
{
  mRestartProcess = true;
}

bool SourceProc::NeedDecoder()
{
  return true;
}

bool SourceProc::CheckProcess()
{
  if (mProcess && mRestartProcess) {
    if (StopProcess()) {
      mRestartProcess = false;
    }
  }

  if (!mProcess) {
    if (!StartProcess()) {
      return false;
    }
  }
  return true;
}

bool SourceProc::StartProcess()
{
  QDir procDir(mProcessFolder);
  if (!procDir.exists()) {
    procDir.mkpath(mProcessFolder);
  }

  mProcess.reset(new QProcess());
  mProcess->start(mProcessExe, mProcessParams.split('|'));
  if (!mProcess->waitForStarted()) {
    return false;
  }
  return true;
}

bool SourceProc::StopProcess()
{
  mProcess->terminate();
  if (!mProcess->waitForFinished()) {
    mProcess->kill();
  }
  mProcess.clear();
  return true;
}

bool SourceProc::DoPath()
{
  QDirIterator itr(mProcessFolder);
  while (itr.hasNext()) {
    DoFile(itr.next());
  }
  return true;
}

bool SourceProc::DoFile(const QString& path)
{
  QFile file(path);
  if (file.open(QFile::ReadOnly)) {
    int size = file.size();

    FrameS frame = FrameS(new Frame());
    frame->ReserveData(size);
    Frame::Header* header = frame->InitHeader();
    header->Compression   = eJpeg;
    header->Key           = true;
    header->Size          = sizeof(Frame::Header) + size;
    header->VideoDataSize = size;
    file.read(frame->VideoData(), size);
    file.close();

    OnFrame(frame);
    ThumbnailS thumbnail = GetThumbnail();
    if (thumbnail && thumbnail->IsTimeToCreate()) {
      thumbnail->Create(frame);
    }
    file.remove();
    return true;
  }
  return false;
}


SourceProc::SourceProc(SettingsA &settings)
  : Source()
  , mRestartProcess(false)
{
  const static QString kProcPrefix("proc://");
  QString uri = settings.GetMandatoryValue("Uri", true).toString();
  if (uri.startsWith(kProcPrefix)) {
    mProcessExe    = uri.mid(kProcPrefix.size());
    mProcessParams = settings.GetValue("ProcParams", "").toString();
    mProcessFolder = settings.GetValue("ProcFolder", "").toString();
  } else {
    Log.Fatal(QString("Proc with bad Uri"));
  }
}

SourceProc::~SourceProc()
{
}
