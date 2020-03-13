#pragma once

#include <QList>

#include <LibV/Include/Frame.h>
#include <LibV/Include/ConveyorV.h>
#include <Lib/Settings/SettingsA.h>
#include <Lib/Include/License_h.h>
#include <Lib/Db/Db.h>


DefineClassS(Analizer);
DefineClassS(AnalyticsA);
DefineClassS(Profiler);
DefineStructS(Detector);

class Analizer: public ConveyorV
{
  DbS                    mDb;
  int                    mVaId;
  QString                mVaType;
  bool                   mHasCodec;
  bool                   mDebug;

  EventTypeTableS        mEventTypeTable;
  EventTableS            mEventTable;
  FilesTableS            mFilesTable;

  AnalyticsAS            mAnalytics;
  SettingsAS             mSettings;
  SettingsAS             mLocalSettings;
  QList<DetectorS>       mDetectors;
  ProfilerS              mProfiler;
  qint64                 mNextOptimizeTime;

  QList<FrameS>          mSourceFrames;
  QList<FrameS>          mDecodedFrames;

  LICENSE_HEADER

public:
  /*override */virtual const char* Name() { return "Analizer"; }
  /*override */virtual const char* ShortName() { return "A"; }
protected:
  /*override */virtual bool DoInit();
  /*override */virtual void DoRelease();

protected:
  /*override */virtual bool ProcessFrame();

private:
  bool ProcessRawFrame(const FrameS& currentFrame);
  bool ProcessCodedFrame(const FrameS& currentFrame);

public:
  int LoadSettings(DbS& db, int vaId);
private: /*internal*/
  void TriggerEvent(int detectorId, const char* eventType, const QDateTime& timestamp, qreal value);
  void TriggerEvent(int detectorId, const char* eventType, const QDateTime& timestamp, qreal value, const QByteArray& img);
  void StatEvent(int detectorId, const char* eventType, const QDateTime& timestamp, qreal value, int periodMs);

private:
  void FixFrames();
  FrameS MakeAnalized(const FrameS& sourceFrame, const FrameS& decodedFrame);
  bool Optimize();
  bool OptimizeRemoveTime(const DetectorS& detector, qint64 periodMs);
  bool OptimizeRemoveCount(const DetectorS& detector, int count);
  bool OptimizeRemoveSize(const DetectorS& detector, qint64 size);

public:
  Analizer(DbS& _Db, int _VaId, const QString& _VaType, bool _HasCodec, bool _Debug);
  /*override */virtual ~Analizer();

  friend struct DbTriggeredDetector;
};

