#include <Lib/Log/Log.h>
#include <Lib/Common/Profiler.h>
#include <LibV/Include/Tools.h>

#include "MacroMotion.h"
#include "DiffLayers.h"
#include "BlockObj.h"


const int kBlockSizeMin = 8;
const int kBlockSizeMax = 96;
const int kBlockSizeDefault = 16;

void MacroMotion::AnalizeInit()
{
  mDiffLayers->Init(getImageData());
  mBlockObj->Init();
}

void MacroMotion::AnalizeFront()
{
  //mSceneProfiler->Start();
  mDiffLayers->Calc(getImageData());
  //mSceneProfiler->Pause();
}

void MacroMotion::AnalizeScene()
{
  //mSceneProfiler2->Start();
  mDiffLayers->Update();
  //mSceneProfiler2->Pause();

  //mBlockProfiler->Start();
  mBlockObj->SetThresholds(mDiffLayers->BlockDiffThreshold());
  mBlockObj->Analize();
  //mBlockProfiler->Pause();

  //const int kDumpMs = 1000;
  //mSceneProfiler->AutoDump(kDumpMs);
  //mSceneProfiler2->AutoDump(kDumpMs);
  //mBlockProfiler->AutoDump(kDumpMs);
}

bool MacroMotion::NeedStable()
{
  return true;
}

qreal MacroMotion::CalcStable()
{
  return mDiffLayers->CalcStable() + mBlockObj->CalcStable();
}

void MacroMotion::LoadSettings(const SettingsAS& settings)
{
  mDiffLayers->LoadSettings(settings);
  mBlockObj->LoadSettings(settings);
}

void MacroMotion::SaveSettings(const SettingsAS& settings)
{
  mDiffLayers->SaveSettings(settings);
  mBlockObj->SaveSettings(settings);
}

int MacroMotion::GetDiffCount()
{
  return mDiffLayers->GetMediumDiff();
}

int MacroMotion::GetDebugFrameCount()
{
#ifdef QT_NO_DEBUG
  return 3;
#else
  return 7;
#endif
}

bool MacroMotion::GetDebugFrame(const int index, QString& text, EImageType& imageType, uchar* data, bool &save)
{
  const static int kSwitchBase = __COUNTER__;
#define AUTO_CASE __COUNTER__ - kSwitchBase
#ifdef QT_NO_DEBUG
  save = false;
  switch (index) {
//  case AUTO_CASE: text = "Background"; imageType = eValue2; mDiffLayers->GetDbgLayerB(DebugData(data)); return true;
//  case AUTO_CASE: text = "Front"; imageType = eValue2; mDiffLayers->GetDbgLayerF(DebugData(data)); return true;
////  case AUTO_CASE: text = "Preobjs"; imageType = eIndex; mBlockObj->GetDbgPreObj(DebugData(data)); return true;
//  case AUTO_CASE: text = "Objects"; imageType = eIndex; mBlockObj->GetDbgObj(DebugData(data)); return true;
  case AUTO_CASE: text = "Layer B"; imageType = eValue2; mDiffLayers->GetDbgLayerB(DebugData(data)); return true;
  case AUTO_CASE: text = "Calc"; imageType = eIndex; mBlockObj->GetDbgPreObjMark(DebugData(data)); mDiffLayers->GetDbgLayerDCalc(DebugData(data)); return true;
  case AUTO_CASE: text = "Layer B diff threshold"; imageType = eIndex; mDiffLayers->GetDbgLayerBThreshold(DebugData(data)); return true;
  default: return false;
  }
#else
  save = false;
  switch (index) {
//  case AUTO_CASE: text = "Src"; imageType = eValue; GetDbgSource(data); return true;
  // Settings
//  case AUTO_CASE: text = "Ignore"; imageType = eIndex; GetDbgIgnore(DebugData(data)); return true;
//  case AUTO_CASE: text = "Door"; imageType = eIndex; GetDbgDoor(DebugData(data)); return true;
//  case AUTO_CASE: text = "Zone"; imageType = eIndex; GetDbgZone(DebugData(data)); return true;

  // Layer B
  case AUTO_CASE: text = "Layer B"; imageType = eValue2; mDiffLayers->GetDbgLayerB(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B diff"; imageType = eValue2; mDiffLayers->GetDbgLayerBDiff(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B diff shad"; imageType = eValue2; mDiffLayers->GetDbgLayerBDiffShad(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B diff count"; imageType = eValue; mDiffLayers->GetDbgLayerBDiffCount(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B diff mass"; imageType = eValue2; mDiffLayers->GetDbgLayerBDiffMass(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B formula"; imageType = eIndex; mDiffLayers->GetDbgLayerBFormula(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B formula2"; imageType = eIndex; mDiffLayers->GetDbgLayerBFormula2(DebugData(data)); return true;

  // Layers
//  case AUTO_CASE: text = "Layer T1"; imageType = eValue; mDiffLayers->GetDbgLayerT1(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer T2"; imageType = eValue; mDiffLayers->GetDbgLayerT2(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer T3"; imageType = eValue2; mDiffLayers->GetDbgLayerT3(DebugData(data)); return true;
  //case AUTO_CASE: text = "Layer B"; imageType = eValue2; mDiffLayers->GetDbgLayerB(DebugData(data)); return true;
  //case AUTO_CASE: text = "Layer T"; imageType = eValue2; mDiffLayers->GetDbgLayerT(DebugData(data)); return true;
  //case AUTO_CASE: text = "Layer F"; imageType = eValue2; mDiffLayers->GetDbgLayerF(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B diff"; imageType = eValue2; mDiffLayers->GetDbgLayerBDiff(DebugData(data)); return true;
  //case AUTO_CASE: text = "Layer T diff"; imageType = eValue2; mDiffLayers->GetDbgLayerTDiff(DebugData(data)); return true;
  //case AUTO_CASE: text = "Current layer"; imageType = eIndex; mDiffLayers->GetDbgCurrentLayer(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer D"; imageType = eValue; mDiffLayers->GetDbgLayerD(DebugData(data)); return true;
//  case AUTO_CASE: text = "Stable image"; imageType = eValue; GetDbgBackImage(data); return true;

  // Layers line
//const int kTestLine = 10;
//  case AUTO_CASE: text = "Layer line B"; imageType = eIndex; mDiffLayers->GetDbgLayerLineB(kTestLine, DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer line T"; imageType = eIndex; mDiffLayers->GetDbgLayerLineT(kTestLine, DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer line F"; imageType = eIndex; mDiffLayers->GetDbgLayerLineF(kTestLine, DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B diff"; imageType = eValue2; mDiffLayers->GetDbgLayerBDiff(DebugData(data)); return true;

  // Calc pre objects
//  case AUTO_CASE: text = "Calc"; imageType = eIndex; mBlockObj->GetDbgPreObjMark(DebugData(data)); mDiffLayers->GetDbgLayerBFormula(DebugData(data)); return true;
//  case AUTO_CASE: text = "Calc2"; imageType = eIndex; mBlockObj->GetDbgPreObjMark(DebugData(data)); mDiffLayers->GetDbgLayerBFormula2(DebugData(data)); return true;
//  case AUTO_CASE: text = "DiffB"; imageType = eValue; mDiffLayers->GetDbgDiffB(DebugData(data)); return true;
//  case AUTO_CASE: text = "DiffB Block"; imageType = eValue; mDiffLayers->GetDbgDiffBlockB(DebugData(data)); return true;
//  case AUTO_CASE: text = "Calc"; imageType = eIndex; mBlockObj->GetDbgPreObjMark(DebugData(data)); mDiffLayers->GetDbgLayerDCalc(DebugData(data)); return true;
//  case AUTO_CASE: text = "Calc"; imageType = eIndex; mBlockObj->GetDbgPreObjMark(DebugData(data)); return true;
//  case AUTO_CASE: text = "DiffB Minus"; imageType = eValue; mDiffLayers->GetDbgDiffMinusBlockB(DebugData(data)); return true;
//  case AUTO_CASE: text = "Block mark"; imageType = eIndex; mDiffLayers->GetDbgObjBlock(DebugData(data)); return true;
//  case AUTO_CASE: text = "Diff moment"; imageType = eHyst; mBlockObj->GetDbgMomentHyst(data); return true;
//  case AUTO_CASE: text = "Pre obj"; imageType = eIndex; mBlockObj->GetDbgPreObj(DebugData(data)); return true;

  // Obj from pre
  case AUTO_CASE: text = "Calc"; imageType = eIndex; mBlockObj->GetDbgPreObjMark(DebugData(data)); mDiffLayers->GetDbgLayerDCalc(DebugData(data)); return true;
  case AUTO_CASE: text = "Layer B diff threshold"; imageType = eIndex; mDiffLayers->GetDbgLayerBThreshold(DebugData(data)); return true;
//  case AUTO_CASE: text = "Pre obj"; imageType = eIndex; mBlockObj->GetDbgPreObj(DebugData(data)); return true;
//  case AUTO_CASE: text = "Obj"; imageType = eIndex; mBlockObj->GetDbgObj(DebugData(data)); return true;
//  case AUTO_CASE: text = "Block diff hyst"; imageType = eHyst; mDiffLayers->GetDbgBlockDiffHyst(22, 8, data); return true;
//  case AUTO_CASE: text = "Grad"; imageType = eValue; mDiffLayers->GetDbgGrad(DebugData(data)); return true;
//  case AUTO_CASE: text = "Grad hyst"; imageType = eHyst; mDiffLayers->GetDbgGradHyst(data); return true;
//  case AUTO_CASE: text = "Diff calc"; imageType = eIndex; mDiffLayers->GetDbgDiffFormula(DebugData(data)); return true;
//  case AUTO_CASE: text = "Obj image"; imageType = eIndex; mDiffLayers->GetDbgObjImage(DebugData(data)); return true;
//  case AUTO_CASE: text = "Obj block"; imageType = eIndex; mDiffLayers->GetDbgObjBlock(DebugData(data)); return true;
//  case AUTO_CASE: text = "Pre obj mark"; imageType = eIndex; mBlockObj->GetDbgPreObjMark(DebugData(data)); return true;
//  case AUTO_CASE: text = "Pre obj"; imageType = eIndex; mBlockObj->GetDbgPreObj(DebugData(data)); return true;
//  case AUTO_CASE: text = "Obj"; imageType = eIndex; mBlockObj->GetDbgObj(DebugData(data)); return true;

  // UIN
//  case AUTO_CASE: text = "Uin regions"; imageType = eValue; mBlockObj->GetDbgUinRegions(DebugData(data)); return true;
//  case AUTO_CASE: text = "Uin detect"; imageType = eIndex; mBlockObj->GetDbgUinDetect(DebugData(data)); return true;

  // Energy
  //case AUTO_CASE: text = "Scene energy";  imageType = eValue; mDiffLayers->GetDbgEnergy(DebugData(data)); return true;
  //case AUTO_CASE: text = "Obj energy"; imageType = eValue; mBlockObj->GetDbgObjEnergy(DebugData(data)); return true;
  default: return false;
  }
#endif
  return true;
#undef AUTO_CASE
}

bool MacroMotion::GetStatAbbr(int type, QString& abbr)
{
  switch (type) {
  case 0: abbr = "mov"; return true;
//  case 1: return mBlockObj->getEnergyMs();
  }
  return false;
}

int MacroMotion::GetStatTimeMs(int type)
{
  switch (type) {
  case 0: return mDiffLayers->getEnergyMs();
//  case 1: return mBlockObj->getEnergyMs();
  }
  return 0;
}

bool MacroMotion::GetStatImage(int type, QByteArray& image)
{
  switch (type) {
  case 0: MakeStatImage(mDiffLayers->SceneEnergy(), image); return true;
//  case 1: MakeStatImage(mBlockObj->ObjEnergy(), image); break;
  }
  return false;
}

void MacroMotion::ResetStat(int type)
{
  switch (type) {
  case 0: mDiffLayers->ResetEnergy(); break;
//  case 1: mBlockObj->ResetEnergy(); break;
  }
}

bool MacroMotion::HaveNextObject()
{
  return mBlockObj->HaveObj();
  //return mBlockObj->HavePreObj();
}

bool MacroMotion::RetrieveNextObject(Object& object)
{
  return mBlockObj->RetrieveObj(object);
  //return mBlockObj->RetrievePreObj(object);
}

void MacroMotion::ExtraSettings(const SettingsAS& _Settings)
{
  int manSize = _Settings->GetValue("ManSize", 5 * kBlockSizeDefault).toInt();
  int blockSize = (manSize + 20) / 40 * 8;
  blockSize = qBound(kBlockSizeMin, blockSize, kBlockSizeMax);
  SetBlockSize(blockSize);
  int thresholdSens = _Settings->GetValue("ThresholdSens", 50).toInt();

  mDiffLayers->SetThresholdSens(thresholdSens);
  bool smooth = false;
  mBlockObj->SetSmoothBlocks(smooth);
  mBlockObj->SetThresholds(smooth? 8: 4);
  int threadsCount = 1;//_Settings->GetValue("Threads", 1).toInt();
  mDiffLayers->SetThreadsCount(threadsCount);
  int inZoneTime = 15000;//_Settings->GetValue("InZoneTime", 15000).toInt();
  mBlockObj->SetInZoneTime(inZoneTime);

  bool useScreenshots = _Settings->GetValue("UseScreenshots", 0).toInt();
  mBlockObj->SetUseScreenshots(useScreenshots);
}


MacroMotion::MacroMotion()
  : AnalyticsB(kBlockSizeDefault)
  //, mSceneProfiler(new Profiler("Scene profile"))//, mSceneProfiler2(new Profiler("Scene profile2"))
  //, mBlockProfiler(new Profiler("Block profile"))
{
  mDiffLayers = DiffLayersS(new DiffLayers(*this, false));
  mBlockObj = BlockObjS(new BlockObj(*this, mDiffLayers->BlockDiffCount(), mDiffLayers->LayerDiff()));
  mBlockObj->SetLayerBackground(&mDiffLayers->LayerBackground());
  mBlockObj->SetLayerFront(&mDiffLayers->LayerFront());

  Log.Info("Using Macro motion analytics");
}

MacroMotion::~MacroMotion()
{
}

