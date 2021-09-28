#pragma once

#include <QElapsedTimer>
#include <QMutex>
#include <QList>

#include <LibV/Include/Frame.h>
#include <Lib/Settings/SettingsA.h>
#include <Lib/Net/Chater.h>
#include <LibV/Include/ConveyorV.h>
#include <Lib/Include/License_h.h>


DefineClassS(Transmit);
DefineClassS(ListenSvc);
DefineClassS(Storage);
DefineClassS(Saver);
DefineClassS(TrReceiver);
DefineClassS(Thumbnail);
DefineClassS(Ptz);

struct ClentInfo
{
  Chater*       Chat;
  bool          Live;
  int           Priority;
  int           Speed;
  int           Denum;
  bool          PreFrames;
  QElapsedTimer ConfirmTimer;

  ClentInfo(Chater* _Chater, bool _Live, int _Priority, int _Speed, int _Denum)
    : Chat(_Chater), Live(_Live), Priority(_Priority), Speed(_Speed), Denum(_Denum)
    , PreFrames(true)
  { ConfirmTimer.start(); }
};

class Transmit: public ConveyorV
{
  SettingsAS          mStorageSettings;
  ListenSvcS          mListenSvc;
  const int           mPoolLimit;
  const int           mLiveWeight;
  const int           mArchWeight;

  PROPERTY_GET_SET(PtzS,   Ptz)
  QMutex              mPtzMutex;

  QMutex              mPoolMutex;
  QList<ClentInfo>    mClientPool; // ordered by priority asc
  int                 mPoolWeight;

  QList<FrameS>       mFramePool;
  FrameS              mKeyFrame;
  bool                mIsStatus;
  QMutex              mStoreMutex;
  QList<FrameS>       mStorePool;
  int                 mStoreSize;
  int                 mStoreFirstPartSize;
  int                 mStoreSizeLimit;

  ThumbnailS          mThumbnail;
  QMutex              mMediaMutex;
  QByteArray          mSpsPpsInfo;
  bool                mMediaInfo;

  LICENSE_HEADER

public:
  ThumbnailS& Thumbnail() { return mThumbnail; }
  FrameS&     KeyFrame()  { return mKeyFrame; }
  bool        IsStatus()  { return mIsStatus; }
  void SetThumbnail(const ThumbnailS& _Thumbnail) { mThumbnail = _Thumbnail; }
  int GetStoreSize() { return mStoreSize; }
  QMutex* PtzMutex() { return &mPtzMutex; }

public:
  /*override */virtual const char* Name() override { return "Transmit"; }
  /*override */virtual const char* ShortName() override { return "T"; }
protected:
  /*override */virtual bool DoInit() override;
//  /*override */virtual void DoRelease() override;

protected:
  /*override */virtual bool ProcessFrame() override;

public:
  bool GetMediaInfo(int& rspMsgId, QByteArray& rspMsgData);

public:
  bool HaveStorage();
  StorageS CreateStorage();
  bool ClientPlayRequest(Chater* chater, int priority, bool live, const qint64 timestamp = 0, int speed = 0, int denum = 1);
  void ClientContinue(Chater* chater);
  void ClientDisconnected(Chater* chater);

private:
  void InitMediaInfo();

  void UpdateStoreFrames();
  void RemoveOldStoreFrames(const qint64& timestamp);
  bool CalcStoreFirstPartSize();
public:/*internal*/
  bool TakeStoreFrames(const qint64& timestamp, QList<FrameS>& frames, bool needKey);
  bool TakeStoreBackFrames(const qint64& timestamp, QList<FrameS>& frames);
  QDateTime GetStoreFirstFrameTs();

private:
  QList<ClentInfo>::iterator ClientRemove(QList<ClentInfo>::iterator& where, const char* reason, bool stop = false);

private:
  bool SendFrames(ClentInfo& info);
  bool SendInfo(Chater* chater);
  bool SendFrame(Chater* chater, FrameS& frame);

  int CalcWeight(bool live, int speed);

public:
  Transmit(SettingsAS& _StorageSettings, int _Port, int _PoolLimit, int _LiveWeight = 1, int _ArchWeight = 3);
  /*override */virtual ~Transmit();
};

