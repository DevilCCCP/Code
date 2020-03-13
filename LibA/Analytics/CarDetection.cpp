#include <LibV/Include/Region.h>
#include <LibA/Analyser/Analyser.h>

#include "CarDetection.h"


void CarDetection::LoadSettings(const SettingsAS& settings)
{
  mPlateWidth = settings->GetValue("PlateWidth", 140).toInt();
}

void CarDetection::SaveSettings(const SettingsAS& settings)
{
  Q_UNUSED(settings);
}

void CarDetection::Init(const ImageSrc<uchar>& source)
{

}

void CarDetection::Analize(const ImageSrc<uchar>& source)
{
  mRegion.SetSource(const_cast<uchar*>(source.Data(0, 0)), source.Width(), source.Height(), source.Stride());
  mAnalyser->FindUinRu(mRegion, mPlateWidth);
}

void CarDetection::GetDbgUinPre(ImageSrc<uchar>& debug)
{
  Region<uchar> debugRegion;
  mAnalyser->DumpSignalPack(&debugRegion);

  for (int j = 0; j < debugRegion.Height(); j++) {
    const uchar* src = debugRegion.Line(j);
    uchar* dst = debug.Line(j);
    memcpy(dst, src, debug.Width());
  }
}

void CarDetection::GetDbgUinPreImg(ImageSrc<uchar>& debug)
{
  Region<uchar> debugRegion;
  mAnalyser->DumpPlate(&debugRegion);

  for (int j = 0; j < debugRegion.Height(); j++) {
    const uchar* src = debugRegion.Line(j);
    uchar* dst = debug.Line(j);
    memcpy(dst, src, debug.Width());
  }
}


CarDetection::CarDetection(const AnalyticsB& _Analytics)
  : BlockSceneAnalizer(_Analytics)
  , mAnalyser(new Analyser()), mPlateWidth(140)
{
}

