#include <Lib/Log/Log.h>
#include <Lib/Common/Profiler.h>
#include <LibV/Include/Tools.h>

#include "CarMotion.h"
#include "CarDetection.h"
#include "CarTracker.h"


const int kBlockSizeMin = 8;
const int kBlockSizeMax = 96;
const int kBlockSizeDefault = 16;

void CarMotion::AnalizeInit()
{
  AnalizeFront();
}

void CarMotion::AnalizeFront()
{
  //mSceneProfiler->Start();
  mCarDetection->Analize(getImageData());
  //mSceneProfiler->Pause();
  mCarTracker->Analize(mCarDetection->getCarList());
}

void CarMotion::AnalizeScene()
{
}

void CarMotion::LoadSettings(const SettingsAS& settings)
{
  mCarDetection->LoadSettings(settings);
  mCarTracker->LoadSettings(settings);
}

void CarMotion::SaveSettings(const SettingsAS& settings)
{
  mCarDetection->SaveSettings(settings);
  mCarTracker->SaveSettings(settings);
}

int CarMotion::GetDiffCount()
{
  return 0;
}

int CarMotion::GetDebugFrameCount()
{
#ifdef QT_NO_DEBUG
  return 3;
#else
  return 7;
#endif
}

bool CarMotion::GetDebugFrame(const int index, QString& text, EImageType& imageType, uchar* data, bool &save)
{
  const static int kSwitchBase = __COUNTER__;
#define AUTO_CASE __COUNTER__ - kSwitchBase
#ifdef QT_NO_DEBUG
  Q_UNUSED(index);
  Q_UNUSED(text);
  Q_UNUSED(imageType);
  Q_UNUSED(data);
  save = false;
  return false;
#else
  save = true;
  save = false;
  int y = 1;
  switch (index) {
  case AUTO_CASE: text = "Src"; imageType = eValue; GetDbgSource(data); return true;
  // Settings
//  case AUTO_CASE: text = "Ignore"; imageType = eIndex; GetDbgIgnore(DebugData(data)); return true;
//  case AUTO_CASE: text = "Door"; imageType = eIndex; GetDbgDoor(DebugData(data)); return true;
//  case AUTO_CASE: text = "Zone"; imageType = eIndex; GetDbgZone(DebugData(data)); return true;

  // Layers line
//const int kTestLine = 10;
//  case AUTO_CASE: text = "Layer line B"; imageType = eIndex; mDiffLayers->GetDbgLayerLineB(kTestLine, DebugData(data)); return true;

  // Image
  case AUTO_CASE: text = "Uin pre"; imageType = eValue; mCarDetection->GetDbgUinPre(DebugData(data)); return true;
  case AUTO_CASE: text = "Uin pre img"; imageType = eValue; mCarDetection->GetDbgUinPreImg(DebugData(data)); return true;
  default: return false;
  }
#endif
  return true;
#undef AUTO_CASE
}

bool CarMotion::HaveNextObject()
{
  return mCarTracker->HaveObj();
  //return mBlockObj->HavePreObj();
}

bool CarMotion::RetrieveNextObject(Object& object)
{
  return mCarTracker->RetrieveObj(object);
  //return mBlockObj->RetrievePreObj(object);
}


CarMotion::CarMotion()
  : AnalyticsB(kBlockSizeDefault)
  , mCarDetection(new CarDetection(*this)), mCarTracker(new CarTracker(*this))
  //, mSceneProfiler(new Profiler("Scene profile"))
{
  Log.Info("Using Car motion analytics");
}

CarMotion::~CarMotion()
{
}

