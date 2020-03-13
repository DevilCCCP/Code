#pragma once

#include <QVector>

#include <LibV/Va/Region.h>

#include "AnalyticsB.h"


DefineClassS(LineLayers);
DefineClassS(DiffLayers);
DefineClassS(LineObj);
DefineClassS(Profiler);

class MacroLine: public AnalyticsB
{
  LineLayersS        mLineLayers;
  DiffLayersS        mDiffLayers;
  LineObjS           mLineObj;

  Region<uchar>      mSource;
  Region<uchar>      mDebug;
  //ProfilerS          mSceneProfiler;
  //ProfilerS          mLinesProfiler;

protected:
  /*override */virtual void AnalizePrepare(const uchar* imageData) Q_DECL_OVERRIDE;
  /*override */virtual void AnalizeInit() Q_DECL_OVERRIDE;
  /*override */virtual void AnalizeFront() Q_DECL_OVERRIDE;
  /*override */virtual void AnalizeScene() Q_DECL_OVERRIDE;
  /*override */virtual qreal CalcStable() Q_DECL_OVERRIDE;

  /*override */virtual int GetDebugFrameCount() Q_DECL_OVERRIDE;
  /*override */virtual bool GetDebugFrame(const int index, QString& text, EImageType& imageType, uchar* data, bool& save) Q_DECL_OVERRIDE;

  /*override */virtual bool GetStatAbbr(int type, QString& abbr);
  /*override */virtual int  GetStatTimeMs(int type) Q_DECL_OVERRIDE;
  /*override */virtual bool GetStatImage(int type, QByteArray& image) Q_DECL_OVERRIDE;
  /*override */virtual void ResetStat(int type) Q_DECL_OVERRIDE;

public:
  /*override */virtual bool HaveNextObject() Q_DECL_OVERRIDE;
  /*override */virtual bool RetrieveNextObject(Object& object) Q_DECL_OVERRIDE;

protected:
  /*override */virtual void ExtraSettings(const SettingsAS& _Settings) Q_DECL_OVERRIDE;

  /*override */virtual void LoadSettings(const SettingsAS& settings) Q_DECL_OVERRIDE;
  /*override */virtual void SaveSettings(const SettingsAS& settings) Q_DECL_OVERRIDE;
  /*override */virtual int GetDiffCount() Q_DECL_OVERRIDE;

private:
  void InitStat();
  Region<uchar>& DebugData(uchar* data);

public:
  MacroLine();
  /*override */virtual ~MacroLine();
};

