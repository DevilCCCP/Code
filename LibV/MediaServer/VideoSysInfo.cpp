#include <QMutexLocker>

#include <Lib/Db/ObjectType.h>

#include "VideoSysInfo.h"



MediaPackagerS VideoSysInfo::CreateMediaPackager()
{
  return CreateDefaultMediaPackager();
}

QString VideoSysInfo::GetVideoUri(int cameraId)
{
  QMutexLocker lock(&mMutexDb);
  mDb->MoveToThread();
  if (TableItemS item = mObjectTable->GetItem(cameraId)) {
    const ObjectItem* obj = static_cast<const ObjectItem*>(item.data());
    return obj->Uri;
  }
  return QString();
}

bool VideoSysInfo::LoadCameras(int id, QMap<int, ObjectItemS>& queryList)
{
  QMutexLocker lock(&mMutexDb);
  mDb->MoveToThread();
  return mObjectTable->LoadSlaves(id, queryList);
}

VideoSysInfo::VideoSysInfo(const DbS& _Db, CtrlManager* _CtrlManager)
  : mDb(_Db), mObjectTable(new ObjectTable(*_Db)), mCtrlManager(_CtrlManager)
{
}

VideoSysInfo::~VideoSysInfo()
{
}

