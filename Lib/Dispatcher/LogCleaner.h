#pragma once

#include <QElapsedTimer>

#include <Lib/Dispatcher/ImpD.h>
#include <Lib/Db/Db.h>


DefineClassS(LogCleaner);

class LogCleaner: public ImpD
{
  int                     mTruncHours;
  int                     mCleanHours;
  int                     mTruncPeriodMs;
  int                     mCleanPeriodMs;
  QString                 mObjectTypeList;

  int                     mEstimateTruncMs;
  int                     mEstimateCleanMs;
  QElapsedTimer           mWorkTimer;
  qint64                  mNextInitTime;
  qint64                  mNextTruncTime;
  qint64                  mNextCleanTime;
  QVector<ObjectLogInfoS> mTruncInfoList;
  QVector<ObjectLogInfoS> mCleanInfoList;

public:
  /*override */virtual const char* Name() override { return "LogCleaner"; }
  /*override */virtual const char* ShortName() override { return "L"; }
protected:
  /*override */virtual bool LoadSettings(SettingsA* settings) override;
  /*override */virtual bool DoCircle() override;
//  /*override */virtual void DoRelease() override;

private:
  bool Init();
  bool TruncNext();
  bool TruncObject(const qint64& id);
  bool CleanNext();
  bool CleanObject(const qint64& id);

public:
  LogCleaner(const Db& _Db);
};
