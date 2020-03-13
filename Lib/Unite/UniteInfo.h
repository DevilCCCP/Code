#pragma once

#include <QMutex>

#include <Lib/Db/Db.h>


DefineClassS(UniteInfo);
DefineClassS(Overseer);
DefineClassS(InnerCrypt);

enum UniteStage: int {
  eUniteInit = 0,
  eHandShake,
  eValidateSegment,
  eQueryObjects,
  eUniteObjects,
  eQueryEvents,
  eUniteEvents,
  eQueryLogs,
  eUniteLogs,
  eUniteTotalStages,
};

struct UniteStat {
  struct UniteStageStat {
    int  OkCount;
    int  FailCount;
    bool LastOk;
    int  MessageCount;
    int  NextMessage;

    UniteStageStat(): OkCount(0), FailCount(0), LastOk(false), MessageCount(0), NextMessage(1)
    { }
  };

  QMap<int, UniteStageStat> UniteStageStatMap;
};

class UniteInfo
{
  PROPERTY_GET_SET(int, Debug)
  PROPERTY_GET    (int, ServiceId)

  QMap<int, int>        mUnitTypeIdMap;
  QSet<int>             mUnitedTypeIdList;
  QStringList           mSyncTypeIdList;
  QString               mSyncTypeIds;

  QMutex                mMutex;
  QMap<int, UniteStat>  mUniteStageMap;
  QSet<QString>         mWarningMessages;

public:
  bool AddSyncObject(const Db& db, const QString& typeSync);
  bool AddSyncObject(const Db& db, const QString& typeFrom, const QString& typeTo);
  const QStringList& GetSyncTypeIdList();
  const QString& GetSyncTypeIds();
  bool IsTranslatedObject(const ObjectItemS& object);
  bool TranslateObject(const ObjectItemS& object);
  bool ValidateId(const Db& db, const QString& objUuid, bool& valid, int* id = nullptr);

public:
  void SetUniteStage(int objectId, int stage);
  void SetUniteStageFail(int objectId, int stage);
  void WarnOnce(const QString& text);

private:
  void ConstructSyncTypeIds();
  UniteStat* FindUniteStat(int objectId);

public:
  UniteInfo(int _ServiceId);
};
