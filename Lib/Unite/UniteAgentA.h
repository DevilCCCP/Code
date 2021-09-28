#pragma once

#include <QElapsedTimer>

#include <Lib/Dispatcher/Imp.h>
#include <Lib/Db/Db.h>


DefineClassS(UniteAgentA);
DefineClassS(UniteInfo);
DefineClassS(DbStateNotifier);
DefineClassS(Rsa);
DefineClassS(UniteObject);
DefineClassS(QNetworkAccessManager);
DefineClassS(QNetworkReply);
DefineClassS(QEventLoop);
DefineClassS(QTimer);

class UniteAgentA: public Imp
{
  DbS                     mDb;
  UniteInfoS              mUniteInfo;
  QString                 mUri;
  int                     mPeriodMs;
  PROPERTY_GET_SET(bool,  UniteBackward)
  PROPERTY_GET_SET(bool,  UniteEvents)
  PROPERTY_GET_SET(bool,  UniteLogs)
  PROPERTY_GET_SET(bool,  RemoveLocal)

  DbStateNotifierS        mNotifier;
  RsaS                    mRsa;
  QString                 mRsaPubText;
  UniteObjectS            mUniteObject;

  bool                    mValidated;
  VariablesS              mSegment;
  QElapsedTimer           mCheckTimer;
  qint64                  mNextCheck;
  QMap<int, ObjectItemS>  mLocalServers;
  QList<ObjectItemS>      mUpdateServers;
  bool                    mHasError;

  QByteArray              mFileType;
  QByteArray              mFileName;
  QByteArray              mFileData;
  QByteArray              mRequestUuid;
  QByteArray              mRequestSign;

  struct EventInfo {
    int     Id;
    QString ObjectUuid;
    QString EventName;

    EventInfo() { }
    EventInfo(int _Id, const QString& _ObjectUuid, const QString& _EventName)
      : Id(_Id), ObjectUuid(_ObjectUuid), EventName(_EventName) { }
  };

  struct EventLog {
    int        EventId;
    qint64     TriggeredTime;
    int        Value;
    QString    Info;
    QByteArray ScreenShot;

    EventLog() { }
    EventLog(int _EventId, const qint64& _TriggeredTime, int _Value, const QString& _Info, const QByteArray& _ScreenShot)
      : EventId(_EventId), TriggeredTime(_TriggeredTime), Value(_Value), Info(_Info), ScreenShot(_ScreenShot) { }
  };

  ObjectItemS             mCurrentObject;
  qint64                  mEventsBaseTimestamp;
  QList<EventInfo>        mEventInfo;
  QList<EventLog>         mEventLog;
  qint64                  mTopHostEventId;
  qint64                  mTopNextEventId;
  qint64                  mTopLocalEventId;

  qint64                  mLogsBaseTimestamp;
  QList<ObjectItemS>      mLogObjectList;
  QList<ObjectLogS>       mLogList;
  qint64                  mTopHostLogId;
  qint64                  mTopNextLogId;
  qint64                  mBottomLocalLogId;
  qint64                  mTopLocalLogId;

  QNetworkAccessManager*  mNetManager;
  QEventLoop*             mEventLoop;
  bool                    mNetworkHasError;
  int                     mNetworkError;
  int                     mNetworkErrorCount;
  int                     mNetworkErrorMessage;
  QTimer*                 mNetAliveTimer;
  QNetworkReply*          mNetReply;
  QTimer*                 mNetTimeoutTimer;

protected:
  UniteInfo* GetUniteInfo() { return mUniteInfo.data(); }

public:
  void SetNotifier(const DbStateNotifierS& _Notifier) { mNotifier = _Notifier; }

public:
  /*override */virtual const char* Name() { return "Unite agent"; }
  /*override */virtual const char* ShortName() { return "Ua"; }

protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
  /*override */virtual void DoRelease() override;

protected:
  /*new */virtual void UniteStateChanged(bool ok);

private:
  bool CheckUpdateTime();
  bool CheckUpdateObjects(bool& needUpdate);
  bool CheckUpdateObjectsOne(int id, int rev);
  bool CheckUpdateEvents(bool& needUpdate);
  bool CheckUpdateLogs(bool& needUpdate);
  bool DoValidateSegment();

  bool DoUpdateAll();
  bool DoUpdateObjects();
  bool DoUpdateObjectsOne();
  bool DoUpdateEvents();
  bool DoUpdateEvents1st();
  bool DoUpdateEventsNext();
  bool DoUpdateLogs();
  bool DoUpdateLogs1st();
  bool DoUpdateLogsNext();

  bool LoadEvents();
  bool LoadEventLog();
  bool LoadLog();

  bool PackValidate();
  bool PackQueryObject();
  bool PackEventLogQuery();
  bool PackEvents();
  bool PackLogQuery();
  bool PackLogs();

  bool SendValidate(bool& valid);
  bool SendQueryObject(bool& needUpdate, bool& needBackUpdate, QByteArray& backFile);
  bool SendEventLogQuery(bool& needUpdate);
  bool SendLogQuery(bool& needUpdate);
  bool SendObjects();
  bool SendEvents();
  bool SendLogs();

  bool SignRequest();
  bool SendRequest(const QByteArray& function, int& code, QByteArray* file = nullptr);

  bool UpdateTopEventLog();
  bool UpdateTopLog();

private:
  void OnAliveTimeout();

public:
  UniteAgentA(const DbS& _Db, const UniteInfoS& _UniteInfo, const QString& _Uri, int _PeriodMs);
};
