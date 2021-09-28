#pragma once

#include <QMutex>

#include <Lib/Db/Db.h>

#include "Rtsp/RtspPackager.h"


DefineClassS(VideoSysInfo);
DefineClassS(CtrlManager);
DefineClassS(MediaPackager);

class VideoSysInfo
{
  DbS            mDb;
  ObjectTableS   mObjectTable;
  CtrlManager*   mCtrlManager;
  QMutex         mMutexDb;

protected:
  /*new */virtual MediaPackagerS CreateDefaultMediaPackager() = 0;

public:
  CtrlManager* GetCtrlManager() { return mCtrlManager; }
  MediaPackagerS CreateMediaPackager();
  QString GetVideoUri(int cameraId);
  bool LoadCameras(int id, QMap<int, ObjectItemS>& queryList);

public:
  VideoSysInfo(const DbS& _Db, CtrlManager* _CtrlManager);
  /*new */~VideoSysInfo();
};

class RtspInfo: public VideoSysInfo
{
protected:
  /*override */virtual MediaPackagerS CreateDefaultMediaPackager() override { return MediaPackagerS(new RtspPackager()); }

public:
  RtspInfo(const DbS& _Db, CtrlManager* _CtrlManager): VideoSysInfo(_Db, _CtrlManager) { }
};
