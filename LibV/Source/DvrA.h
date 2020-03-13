#pragma once

#include <QElapsedTimer>

#include <Lib/Dispatcher/Imp.h>
#include <Lib/Settings/SettingsA.h>
#include <LibV/Include/Frame.h>


DefineClassS(DvrA);
DefineClassS(SettingsA);
DefineClassS(SourceState);
DefineClassS(Db);
DefineClassS(ObjectTypeTable);
DefineClassS(ObjectTable);
DefineClassS(FrameChannel);

class DvrA: public Imp
{
  PROPERTY_GET_SET(bool, Quiet)

  struct Channel {
    int                   CameraId;
    QElapsedTimer         LastFrame;
    FrameChannelS         FrameChan;

    Channel(int _CameraId): CameraId(_CameraId) { LastFrame.start(); }
    Channel(): CameraId(0) { LastFrame.start(); }
  };

  const Db&           mDb;
  int                 mAnalogTypeId;
  QMap<int, Channel>  mAnalogCameras;
  QList<int>          mChannels;

  Connection::EStatus mStatus;
  bool                mConnected;
  QMutex              mMutex;

  SourceStateS        mSourceState;

protected:
  const QList<int>& Channels() const { return mChannels; }

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "Dvr"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "D"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;
public:
  /*override */virtual void Stop() Q_DECL_OVERRIDE;

protected:
  /*new */virtual bool ReadSettings(SettingsA* settings);
  /*new */virtual bool Init();
  /*new */virtual void Release();
  /*new */virtual bool Connect() = 0;
  /*new */virtual void Disconnect() = 0;

public:
  /*external thread*/ void OnFrame(int channel, const Frame::Header& frameHeader, const char* data);

private:
  void ConnectDvr();
  void DisonnectDvr();
  void CheckDvr();
  void OnStatus(Connection::EStatus status);
  void ClearTimeout();
  bool CheckTimeout(int timeoutMs);
  void SwitchStatus(Connection::EStatus status);

public:
  DvrA(const Db& _Db, int _WorkPeriodMs = 100);
  /*override */virtual ~DvrA();
};

