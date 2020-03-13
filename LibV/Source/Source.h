#pragma once

#include <QMutex>
#include <QElapsedTimer>
#include <QList>

#include <LibV/Include/Frame.h>
#include <LibV/Include/ConveyorV.h>
#include <Lib/Settings/SettingsA.h>

#include "SourceTypes.h"


DefineClassS(Source);
DefineClassS(SourceState);
DefineClassS(Db);
DefineClassS(Thumbnail);

class Source: public ConveyorV
{
  PROPERTY_GET_SET(bool, Quiet)
  PROPERTY_GET_SET(bool, UseAudio)

  Connection::EStatus mStatus;
  QMutex              mMutex;
  QList<FrameS>       mExtraFrames;
  QElapsedTimer       mLastFrame;
  QElapsedTimer       mLastStatus;
  qint64              mLastFixer;
  qint64              mTimestampFixer;
  qint64              mLastTimestamp;

  SourceStateS        mSourceState;
  ThumbnailS          mThumbnail;

public:
  const ThumbnailS& GetThumbnail() { return mThumbnail; }

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "Source"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "S"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
//  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
//  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;

  /*new */virtual void Reconnect() = 0;

public:
  /*new */virtual bool NeedDecoder() { return true; }
//  /*override */virtual bool ProcessFrame() Q_DECL_OVERRIDE { return false; }

public:
  void OnStatus(Connection::EStatus status);
  void OnFrame(const FrameS& frame);

private:
  void ResetFixer();
  void FixFrameTimestamp(qint64& timestamp);
  void CheckTimeout(Connection::EStatus& status, int timeoutMs);
  void SwitchStatus(Connection::EStatus status);

public:
  static SourceS CreateSource(SettingsA& settings, bool quiet);
  static SourceS CreateChildSource();

protected:
  Source(int _WorkPeriodMs = -1);
public:
  /*override */virtual ~Source();
};

