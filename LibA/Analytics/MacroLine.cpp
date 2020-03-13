#include <Lib/Log/Log.h>
#include <Lib/Common/Profiler.h>
#include <LibV/Include/Tools.h>

#include "MacroLine.h"
#include "LineLayers.h"
#include "DiffLayers.h"
#include "LineObj.h"


void MacroLine::AnalizePrepare(const uchar* imageData)
{
  mSource.SetSource(const_cast<uchar*>(imageData), getWidth(), getHeight(), getStride());
}

void MacroLine::AnalizeInit()
{
  mLineLayers->Init(mSource);
//  mDiffLayers->Init(getImageData(), true);
//  mLineObj->Init();
}

void MacroLine::AnalizeFront()
{
  //mSceneProfiler->Start();
  mLineLayers->Calc(mSource);
//  mDiffLayers->Calc(getImageData());
  //mSceneProfiler->Pause();
}

void MacroLine::AnalizeScene()
{
//  mDiffLayers->Update();
//  mDiffLayers->MakeDiff();

//  mLineObj->Analize();
}

qreal MacroLine::CalcStable()
{
  return 0;//mDiffLayers->CalcStable() + mLineObj->CalcStable();
}

int MacroLine::GetDebugFrameCount()
{
  return 7;
}

bool MacroLine::GetDebugFrame(const int index, QString& text, EImageType& imageType, uchar* data, bool &save)
{
  Q_UNUSED(save);

  const static int kSwitchBase = __COUNTER__;
#define AUTO_CASE __COUNTER__ - kSwitchBase
#ifndef QT_NO_DEBUG
//save = true;
#endif
//const int kTestLine = 200;
  switch (index) {
  // Settings
//  case AUTO_CASE: text = "Layer B Line"; imageType = eIndex; mLineLayers->GetDbgLayerBLine(kTestLine, DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B Line diff"; imageType = eIndex; mLineLayers->GetDbgLayerBLineDiff(kTestLine, DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer T Line"; imageType = eIndex; mLineLayers->GetDbgLayerTLine(kTestLine, DebugData(data)); return true;
  case AUTO_CASE: text = "Layer B"; imageType = eValue; mLineLayers->GetDbgLayerB(DebugData(data)); return true;
  case AUTO_CASE: text = "Layer B seg fail"; imageType = eValue2; mLineLayers->GetDbgLayerBSegFail(DebugData(data)); return true;
  case AUTO_CASE: text = "Layer T seg fail"; imageType = eValue2; mLineLayers->GetDbgLayerTSegFail(DebugData(data)); return true;
  case AUTO_CASE: text = "Layer T"; imageType = eValue; mLineLayers->GetDbgLayerT(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B seg"; imageType = eIndex; mLineLayers->GetDbgLayerBSeg(DebugData(data)); return true;
  case AUTO_CASE: text = "Layer D"; imageType = eIndex; mLineLayers->GetDbgLayerD(DebugData(data)); return true;
  case AUTO_CASE: text = "Layer B diff"; imageType = eIndex; mLineLayers->GetDbgLayerBDiff(DebugData(data)); return true;
//  case AUTO_CASE: text = "Src Line"; imageType = eIndex; GetDbgSourceLine(kTestLine, data); return true;
//  case AUTO_CASE: text = "Src Layer Line"; imageType = eIndex; mLineLayers->GetDbgSourceLineWithLayer(kTestLine, DebugData(data)); return true;

//  case AUTO_CASE: text = "Src"; imageType = eValue; GetDbgSourceWithLine(kTestLine, data); return true;
//  case AUTO_CASE: text = "Ignore"; imageType = eIndex; GetDbgIgnore(DebugData(data)); return true;
//  case AUTO_CASE: text = "Door"; imageType = eIndex; GetDbgDoor(DebugData(data)); return true;
//  case AUTO_CASE: text = "Zone"; imageType = eIndex; GetDbgZone(DebugData(data)); return true;

  // Layer B
//  case AUTO_CASE: text = "Layer B"; imageType = eValue2; mLineLayers->GetDbgLayerB(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B"; imageType = eValue2; mDiffLayers->GetDbgLayerB(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B diff"; imageType = eValue2; mDiffLayers->GetDbgLayerBDiff(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B formula"; imageType = eIndex; mDiffLayers->GetDbgLayerBFormula(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B formula2"; imageType = eIndex; mDiffLayers->GetDbgLayerBFormula2(DebugData(data)); return true;

  // Layers
//  case AUTO_CASE: text = "Layer B"; imageType = eValue2; mDiffLayers->GetDbgLayerB(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer T"; imageType = eValue2; mDiffLayers->GetDbgLayerT(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer F"; imageType = eValue2; mDiffLayers->GetDbgLayerF(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B diff"; imageType = eValue2; mDiffLayers->GetDbgLayerBDiff(DebugData(data)); return true;
  //case AUTO_CASE: text = "Layer T diff"; imageType = eValue2; mDiffLayers->GetDbgLayerTDiff(DebugData(data)); return true;
  //case AUTO_CASE: text = "Current layer"; imageType = eIndex; mDiffLayers->GetDbgCurrentLayer(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer D"; imageType = eValue; mDiffLayers->GetDbgLayerD(DebugData(data)); return true;
//  case AUTO_CASE: text = "Stable image"; imageType = eValue; GetDbgBackImage(data); return true;

  // Layers line
//  case AUTO_CASE: text = "Line Layer B"; imageType = eIndex; mDiffLayers->GetDbgLayerLineB(kTestLine, DebugData(data)); return true;
//  case AUTO_CASE: text = "Line Layer T"; imageType = eIndex; mDiffLayers->GetDbgLayerLineT(kTestLine, DebugData(data)); return true;
//  case AUTO_CASE: text = "Line Layer F"; imageType = eIndex; mDiffLayers->GetDbgLayerLineF(kTestLine, DebugData(data)); return true;
//  case AUTO_CASE: text = "Line Layer F vs B"; imageType = eIndex; mDiffLayers->GetDbgLayerLineBfDiff(kTestLine, DebugData(data)); return true;
//  case AUTO_CASE: text = "Src Line"; imageType = eIndex; GetDbgSourceLine(kTestLine, DebugData(data)); return true;
//  case AUTO_CASE: text = "Line place"; imageType = eValue; mDiffLayers->GetDbgLinePlace(kTestLine, DebugData(data)); return true;

  // Calc pre objects
//  case AUTO_CASE: text = "Layer Diff"; imageType = eIndex; mDiffLayers->GetDbgLayerDiff(DebugData(data)); return true;
//  case AUTO_CASE: text = "Pre Obj mark"; imageType = eValue2; mLineObj->GetDbgPreObjMark(DebugData(data)); return true;
//  case AUTO_CASE: text = "Pre Obj mark border"; imageType = eValue2; mLineObj->GetDbgPreObjMarkSolid(DebugData(data)); return true;
//  case AUTO_CASE: text = "All Obj type1"; imageType = eValue2; mLineObj->GetDbgAllObjType1(DebugData(data)); return true;
//  case AUTO_CASE: text = "Pre Hyst"; imageType = eHyst; mLineObj->GetDbgPreHyst(data); return true;
//  case AUTO_CASE: text = "Pre Hyst2"; imageType = eHyst; mLineObj->GetDbgPreHyst2(data); return true;
//  case AUTO_CASE: text = "Pre Obj"; imageType = eIndex; mLineObj->GetDbgPreObj(DebugData(data)); return true;
//  case AUTO_CASE: text = "Obj"; imageType = eIndex; mLineObj->GetDbgObj(DebugData(data)); return true;

  // Energy
//  case AUTO_CASE: text = "Scene energy";  imageType = eValue; mDiffLayers->GetDbgEnergy(DebugData(data)); return true;
  default: return false;
  }
  return true;
#undef AUTO_CASE
}

bool MacroLine::GetStatAbbr(int type, QString& abbr)
{
  switch (type) {
  case 0: abbr = "mov"; return true;
  }
  return false;
}

int MacroLine::GetStatTimeMs(int type)
{
  switch (type) {
  case 0: return mDiffLayers->getEnergyMs();
  }
  return 0;
}

bool MacroLine::GetStatImage(int type, QByteArray& image)
{
  switch (type) {
  case 0: MakeStatImage(mDiffLayers->SceneEnergy(), image); return true;
  }
  return false;
}

void MacroLine::ResetStat(int type)
{
  switch (type) {
  case 0: mDiffLayers->ResetEnergy(); break;
  }
}

bool MacroLine::HaveNextObject()
{
  return false;
//  return mLineObj->HaveObj();
  //return mBlockObj->HavePreObj();
}

bool MacroLine::RetrieveNextObject(Object& object)
{
  Q_UNUSED(object);

  return false;
//  return mLineObj->RetrieveObj(object);
  //return mBlockObj->RetrievePreObj(object);
}

void MacroLine::ExtraSettings(const SettingsAS& _Settings)
{
  Q_UNUSED(_Settings);
}

void MacroLine::LoadSettings(const SettingsAS& settings)
{
  mLineLayers->LoadSettings(settings);
//  mDiffLayers->LoadSettings(settings);
//  mLineObj->LoadSettings(settings);
}

void MacroLine::SaveSettings(const SettingsAS& settings)
{
  mLineLayers->SaveSettings(settings);
//  mDiffLayers->SaveSettings(settings);
//  mLineObj->SaveSettings(settings);
}

int MacroLine::GetDiffCount()
{
  return 0;
  //  return mDiffLayers->GetMediumDiff();
}

Region<uchar>& MacroLine::DebugData(uchar* data)
{
  mDebug.SetSource(data, getWidth(), getHeight(), getStride());
  return mDebug;
}


MacroLine::MacroLine()
  : AnalyticsB(kBlockSize)
  //, mSceneProfiler(new Profiler("Scene profile")), mLinesProfiler(new Profiler("Line profile"))
{
  mLineLayers = LineLayersS(new LineLayers(*this));
//  mDiffLayers = DiffLayersS(new DiffLayers(*this, true));
//  mLineObj = LineObjS(new LineObj(*this, mDiffLayers->LayerDiff()));

  Log.Info("Using line object analytics");
}

MacroLine::~MacroLine()
{
}

