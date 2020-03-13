#pragma once

#include <QMutex>
#include <QMutexLocker>

#include <Lib/Db/Db.h>


DefineClassS(UniteInfo);
DefineClassS(Overseer);
DefineClassS(InnerCrypt);

class UniteInfo
{
  const Overseer* mOverseer;
  const Db&       mDb;
  ObjectTableS    mObjectTable;
  ObjectTypeS     mObjectType;
  EventTableS     mEventTable;
  FilesTableS     mFilesTable;
  VariablesTableS mVariablesTable;
  QMutex          mMutexDb;
  int             mServerTypeId;
  int             mUniteServerTypeId;
  int             mCameraTypeId;
  int             mCamera2TypeId;
  int             mUniteCameraTypeId;

  InnerCryptS     mInnerCrypt;
  QMutex          mMutexCrypt;

public:
  const Overseer* GetOverseer() { return mOverseer; }
//  int GetUniteServerId() const { return mUniteServerId; }

public:
  bool Init();
  bool TranslateObject(const ObjectItemS& object);
  bool IsTranslatedObject(const ObjectItemS& object);
  bool ValidateId(const QString& objUuid, bool& valid, int* id = nullptr);

public:
  UniteInfo(const Overseer* _Overseer, const Db& _Db);

  friend class UniteObjectTable;
  friend class UniteObjectType;
  friend class UniteEventTable;
  friend class UniteFilesTable;
  friend class UniteVariablesTable;
  friend class UniteInnerCrypt;
};

#define UniteShareObject(ObjectT) \
  class Unite##ObjectT { \
    ObjectT* m##ObjectT; \
    QMutexLocker mLocker; \
   \
  public: \
    ObjectT* operator->() { return m##ObjectT; } \
   \
  public: \
    Unite##ObjectT(UniteInfo* _UniteInfo) \
      : m##ObjectT(_UniteInfo->m##ObjectT.data()), mLocker(&_UniteInfo->mMutexDb) \
    { } \
  }

UniteShareObject(ObjectTable);
UniteShareObject(ObjectType);
UniteShareObject(EventTable);
UniteShareObject(FilesTable);
UniteShareObject(VariablesTable);
UniteShareObject(InnerCrypt);
