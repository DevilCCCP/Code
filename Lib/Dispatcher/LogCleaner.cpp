#include <Lib/Common/Format.h>
#include <Lib/Settings/SettingsA.h>
#include <Lib/Db/ObjectLog.h>
#include <Lib/Db/ObjectLogInfo.h>

#include "LogCleaner.h"


const int kWorkPeriodMs = 100;
const int kTooLongFunctionMs = 30*1000;
const int kFailFunctionMs = 15*60*1000;
const int kTruncHours = 7*24;
const int kCleanHours = 3*30*24;
const int kTruncPeriodMs = 12*60*60*1000;
const int kCleanPeriodMs = 24*60*60*1000;
const int kReInitPeriodMs = 24*60*60*1000;

bool LogCleaner::LoadSettings(SettingsA* settings)
{
  mTruncHours = settings->GetValue("TruncHours", kTruncHours).toInt();
  mCleanHours = settings->GetValue("CleanHours", kCleanHours).toInt();
  mTruncPeriodMs = settings->GetValue("TruncPeriodMs", kTruncPeriodMs).toInt();
  mCleanPeriodMs = settings->GetValue("CleanPeriodMs", kCleanPeriodMs).toInt();
  mObjectTypeList = settings->GetMandatoryValue("ObjectTypeList").toString();
  mWorkTimer.start();
  return true;
}

bool LogCleaner::DoCircle()
{
  if (mWorkTimer.elapsed() >= mNextInitTime) {
    if (!Init()) {
      return true;
    }
  }

  if (mWorkTimer.elapsed() >= mNextTruncTime) {
    if (!TruncNext()) {
      return true;
    }
    mNextTruncTime += mEstimateTruncMs;
  }
  if (mWorkTimer.elapsed() >= mNextCleanTime) {
    if (!CleanNext()) {
      return true;
    }
    mCleanInfoList.removeFirst();
    mNextCleanTime += mEstimateCleanMs;
  }
  return true;
}

bool LogCleaner::Init()
{
  mTruncInfoList.clear();
  if (!GetDb().getObjectLogInfoTable()->LoadTruncListByType(mObjectTypeList, mTruncInfoList)) {
    return false;
  }
  mCleanInfoList.clear();
  if (!GetDb().getObjectLogInfoTable()->LoadCleanListByType(mObjectTypeList, mCleanInfoList)) {
    return false;
  }
  mEstimateTruncMs = mTruncPeriodMs / (mTruncInfoList.size() + 1);
  mEstimateCleanMs = mCleanPeriodMs / (mCleanInfoList.size() + 1);

  Log.Info(QString("Trunc init (count: %1, interval: %2, period: %3)")
           .arg(mTruncInfoList.size()).arg(FormatTime(mEstimateTruncMs)).arg(FormatTime(mTruncPeriodMs)));
  Log.Info(QString("Clean init (count: %1, interval: %2, period: %3)")
           .arg(mCleanInfoList.size()).arg(FormatTime(mEstimateCleanMs)).arg(FormatTime(mCleanPeriodMs)));
  mNextInitTime = mWorkTimer.elapsed() + kReInitPeriodMs;
  mNextTruncTime = 0;
  mNextCleanTime = 0;
  return true;
}

bool LogCleaner::TruncNext()
{
  if (mTruncInfoList.isEmpty()) {
    return true;
  }

  if (!TruncObject(mTruncInfoList.first()->Id)) {
    return false;
  }
  mTruncInfoList.removeFirst();
  return true;
}

bool LogCleaner::TruncObject(const qint64& id)
{
  ObjectLogInfoS objectLogInfo;
  if (!GetDb().getObjectLogInfoTable()->SelectById(id, objectLogInfo)) {
    return false;
  }
  if (!objectLogInfo) {
    return true;
  }
  if (objectLogInfo->LastTrunc.isValid() && objectLogInfo->LastTrunc > QDateTime::currentDateTime().addMSecs(-mTruncPeriodMs/2)) {
    return true;
  }
  if (!GetDb().getObjectLogTable()->TruncHours(objectLogInfo->ObjectId, mTruncHours)) {
    return false;
  }
  return true;
}

bool LogCleaner::CleanNext()
{
  if (mCleanInfoList.isEmpty()) {
    return true;
  }

  if (!CleanObject(mCleanInfoList.first()->Id)) {
    return false;
  }
  mCleanInfoList.removeFirst();
  return true;
}

bool LogCleaner::CleanObject(const qint64& id)
{
  ObjectLogInfoS objectLogInfo;
  if (!GetDb().getObjectLogInfoTable()->SelectById(id, objectLogInfo)) {
    return false;
  }
  if (!objectLogInfo) {
    return true;
  }
  if (objectLogInfo->LastClean.isValid() && objectLogInfo->LastClean > QDateTime::currentDateTime().addMSecs(-mCleanPeriodMs/2)) {
    return true;
  }
  if (!GetDb().getObjectLogTable()->CleanHours(objectLogInfo->ObjectId, mCleanHours)) {
    return false;
  }
  return true;
}


LogCleaner::LogCleaner(const Db& _Db)
  : ImpD(_Db, kWorkPeriodMs)
  , mNextInitTime(0), mNextTruncTime(0), mNextCleanTime(0)
{
  SetCriticalWarnMs(kTooLongFunctionMs);
  SetCriticalFailMs(kFailFunctionMs);
}
