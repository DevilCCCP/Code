#include <QElapsedTimer>
#include <QPoint>

#include <Lib/Log/Log.h>

#include "SourceScript.h"
#include "ScriptIn.h"


bool SourceScript::DoInit()
{
  mScript = ScriptInS(new ScriptIn());
  if (!mScript) {
    return false;
  }

  return Source::DoInit();
}

bool SourceScript::DoCircle()
{
  if (OpenFile()) {
    PlayFile();
  } else {
    LOG_ERROR_ONCE("Open file fail");
  }
  return true;
}

void SourceScript::Stop()
{
  Source::Stop();
}

void SourceScript::Reconnect()
{
}

bool SourceScript::OpenFile()
{
  Log.Info("Opening file...");
  if (mScript->Open(mFilename)) {
    Log.Info("File opened");
    return true;
  }
  return false;
}

void SourceScript::PlayFile()
{
  QElapsedTimer getFrameTimer;
  FrameS frame;
  getFrameTimer.start();
  while (IsAlive() && mScript->GetNext(frame)) {
    qint64 ts = frame->GetHeader()->Timestamp;
    qint64 ms = getFrameTimer.elapsed();
    if (ts > ms) {
      qint64 ds = ts - ms;
      msleep(ds);
    }
    OnFrame(frame);
  }
}


SourceScript::SourceScript(SettingsA &settings)
  : Source()
{
  QString uri = settings.GetMandatoryValue("Uri", true).toString();
  if (uri.startsWith("script://")) {
    mFilename = uri.mid(QString("script://").size());
    Log.Info(QString("Created source from script (filename: '%1')").arg(mFilename));
  } else {
    Log.Fatal(QString("Invalid uri (uri: %1)").arg(uri), true);
  }
}

SourceScript::~SourceScript()
{
}
