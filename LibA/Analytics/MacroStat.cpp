#include <Lib/Log/Log.h>
#include <Lib/Common/Profiler.h>
#include <LibV/Include/Tools.h>

#include "MacroStat.h"
#include "DiffLayers.h"
#include "BlockStat.h"


void MacroStat::AnalizeInit()
{
  mDiffLayers->Init(getImageData());
  mBlockStat->Init(getSectionCount());
}

void MacroStat::AnalizeFront()
{
  mDiffLayers->Calc(getImageData());
}

void MacroStat::AnalizeScene()
{
  mDiffLayers->Update();
  mBlockStat->Calc();
  mBlockStat->UpdateStat();
}

qreal MacroStat::CalcStable()
{
  return mDiffLayers->CalcStable();
}

void MacroStat::LoadSettings(const SettingsAS& settings)
{
  mDiffLayers->LoadSettings(settings);
  mBlockStat->LoadSettings(settings);
}

void MacroStat::SaveSettings(const SettingsAS& settings)
{
  mDiffLayers->SaveSettings(settings);
  mBlockStat->SaveSettings(settings);
}

int MacroStat::GetDiffCount()
{
  return mDiffLayers->GetMediumDiff();
}

int MacroStat::GetDebugFrameCount()
{
  return 7;
}

bool MacroStat::GetDebugFrame(const int index, QString& text, EImageType& imageType, byte* data, bool &save)
{
  Q_UNUSED(save);

  const static int kSwitchBase = __COUNTER__;
#define AUTO_CASE __COUNTER__ - kSwitchBase
#ifndef QT_NO_DEBUG
save = false;
#endif
  switch (index) {/*save = true; */
  // Layer B
//  case AUTO_CASE: text = "Layer B"; imageType = eValue2; mDiffLayers->GetDbgLayerB(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B diff"; imageType = eValue2; mDiffLayers->GetDbgLayerBDiff(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B formula"; imageType = eIndex; mDiffLayers->GetDbgLayerBFormula(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B formula2"; imageType = eIndex; mDiffLayers->GetDbgLayerBFormula2(DebugData(data)); return true;

  // Layers
//  case AUTO_CASE: text = "Layer B"; imageType = eValue2; mDiffLayers->GetDbgLayerB(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer T"; imageType = eValue2; mDiffLayers->GetDbgLayerT(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer F"; imageType = eValue2; mDiffLayers->GetDbgLayerF(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B diff"; imageType = eValue2; mDiffLayers->GetDbgLayerBDiff(DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer T diff"; imageType = eValue2; mDiffLayers->GetDbgLayerTDiff(DebugData(data)); return true;
//  case AUTO_CASE: text = "Current layer"; imageType = eIndex; mDiffLayers->GetDbgCurrentLayer(DebugData(data)); return true;

  // Layers line
//  case AUTO_CASE: text = "Layer line B"; imageType = eIndex; mDiffLayers->GetDbgLayerLineB(100, DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer line T"; imageType = eIndex; mDiffLayers->GetDbgLayerLineT(100, DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer line F"; imageType = eIndex; mDiffLayers->GetDbgLayerLineF(100, DebugData(data)); return true;
//  case AUTO_CASE: text = "Layer B diff"; imageType = eValue2; mDiffLayers->GetDbgLayerBDiff(DebugData(data)); return true;

  // Stat
  case AUTO_CASE: text = "Layer B diff"; imageType = eValue2; mDiffLayers->GetDbgLayerBDiff(DebugData(data)); return true;
  case AUTO_CASE: text = "Current layer"; imageType = eIndex; mDiffLayers->GetDbgCurrentLayer(DebugData(data)); return true;
  case AUTO_CASE: text = "Section mark"; imageType = eIndex; GetDbgStatSection(DebugData(data)); return true;
  case AUTO_CASE: text = "Section count"; imageType = eValue; mBlockStat->GetDbgSectionCount(DebugData(data)); return true;
  case AUTO_CASE: text = "Section value"; imageType = eIndex; mBlockStat->GetDbgSectionValue(DebugData(data)); return true;
  case AUTO_CASE: text = "Section in"; imageType = eIndex; mBlockStat->GetDbgSectionIn(DebugData(data)); return true;
  //case AUTO_CASE: text = "Section Hyst"; imageType = eHyst; mBlockStat->GetDbgSectionHyst(data); return true;
  default: return false;
  }
  return true;
#undef AUTO_CASE
}

bool MacroStat::HaveNextObject()
{
  return mBlockStat->HaveSection();
}

bool MacroStat::RetrieveNextObject(Object& object)
{
  return mBlockStat->RetrieveSection(object);
}

void MacroStat::ExtraSettings(const SettingsAS& _Settings)
{
  bool macro = _Settings->GetValue("Macro", true).toBool();
  mBlockStat->SetSmoothBlocks(macro);
  int sectionTime = _Settings->GetValue("SectionTime", 5000).toInt();
  int queueTime = _Settings->GetValue("QueueTime", 5000).toInt();
  mBlockStat->SetTiming(sectionTime, queueTime);
  int queueThreshold = _Settings->GetValue("QueueThreshold", 1).toInt();
  mBlockStat->SetQueueThreshold(queueThreshold);
  int threadsCount = _Settings->GetValue("Threads", 1).toInt();
  mDiffLayers->SetThreadsCount(threadsCount);
}


MacroStat::MacroStat()
  : AnalyticsB(kBlockSize)
{
  mDiffLayers = DiffLayersS(new DiffLayers(*this, true));
  mBlockStat = BlockStatS(new BlockStat(*this, mDiffLayers->BlockDiffCount()));

  Log.Info("Using Macro motion analytics");
}

MacroStat::~MacroStat()
{
}

