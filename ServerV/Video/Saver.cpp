#include <Lib/Log/Log.h>
#include <Lib/Dispatcher/Overseer.h>
#include <LibV/Storage/Storage.h>
#include <Lib/Include/License.h>

#include "Saver.h"


bool Saver::DoInit()
{
  mStorage = StorageS(new Storage(*mStorageSettings));
  if (!mStorage->Open(GetOverseer()->Id())) {
    Log.Error("Open storage fail");
    return false;
  }
  return true;
}

void Saver::DoRelease()
{
  if (!mStorage->CloseWrite()) {
    Log.Error("Close storage fail");
  }
  mStorage.clear();
}

bool Saver::ProcessFrame()
{
  LICENSE_CIRCLE(0xA72D095);
  FrameS currentFrame = CurrentVFrame();
  if (currentFrame->GetHeader()->HeaderSize != sizeof(Frame::Header)) {
    return false;
  }

  if (!mStorage->WriteFrame(currentFrame)) {
    if (!mWriteError) {
      Log.Error("Write frame error");
      mWriteError = true;
    }
  } else if (mWriteError) {
    Log.Info("Write frame resumed");
    mWriteError = false;
  }
  return true;
}

Saver::Saver(SettingsAS& _StorageSettings)
  : mStorageSettings(_StorageSettings)
  , mWriteError(false)
{
}
