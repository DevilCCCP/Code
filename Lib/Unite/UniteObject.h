#pragma once

#include <Lib/Db/Db.h>


DefineClassS(UniteObject);
DefineClassS(UniteInfo);

class UniteObject
{
  const Db&               mDb;
  UniteInfo*              mUniteInfo;
  struct UpdateObject {
    ObjectItemS Object;
    int         Master;

    UpdateObject(ObjectItemS _Object, int _Master)
      : Object(_Object), Master(_Master) { }
  };
  QList<UpdateObject>     mUpdateObjects;
  ObjectItemS             mCurrentObject;
  QList<ObjectSettingsS>  mCurrentSettings;
  QList<VariablesS>       mCurrentVariables;

  QJsonDocument           mCurrentJsonDoc;

public:
  bool ToJson(const ObjectItemS& obj, QByteArray& jsonData);
  bool FromJson(const QString& serverUuid, const QByteArray& pkey, const QByteArray& jsonData);

private:
  bool LoadObjectSlaves();
  bool PackObjects();

  bool GetObjectTypeId(const QString& typeName, int& id);
  bool UpdateObjectOne();
  bool UpdateObjectSettings();
  bool UpdateObjectVariables();
  bool UpdateParentConnection();
  bool RemoveUnused(int serverId, const QSet<int>& usedObjs, int& objRemoved);

public:
  UniteObject(const Db& _Db, UniteInfo* _UniteInfo);
};
