#pragma once

#include <QElapsedTimer>

#include <Lib/Dispatcher/Imp.h>
#include <Lib/Db/Db.h>


DefineClassS(UniteAgent);
DefineClassS(DbStateNotifier);
DefineClassS(InnerCrypt);
DefineClassS(QNetworkAccessManager);
DefineClassS(QNetworkReply);
DefineClassS(QEventLoop);

class UniteAgent: public Imp
{
  DbS                     mDb;
  ObjectTypeS             mObjectType;
  ObjectTableS            mObjectTable;
  VariablesTableS         mVariablesTable;
  DbStateNotifierS        mNotifier;
  InnerCryptS             mInnerCrypt;
  QString                 mUri;
  int                     mPeriodMs;

  bool                    mValidated;
  VariablesS              mSegment;
  QElapsedTimer           mCheckTimer;
  int                     mServerTypeId;
  int                     mCameraTypeId;
  int                     mCamera2TypeId;
  QString                 mCameraTypes;
  QMap<int, ObjectItemS>  mLocalServers;
  QList<ObjectItemS>      mUpdateServers;

  struct UpdateObject {
    ObjectItemS Object;
    int         Master;

    UpdateObject(ObjectItemS _Object, int _Master)
      : Object(_Object), Master(_Master) { }
  };
  QList<UpdateObject>     mUpdateObjects;
  ObjectItemS             mCurrentObject;
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

  qint64                  mEventsBaseTimestamp;
  QList<EventInfo>        mEventInfo;
  QList<EventLog>         mEventLog;
  qint64                  mTopHostEventId;
  qint64                  mTopNextEventId;
  qint64                  mTopLocalEventId;

  QNetworkAccessManager*  mNetManager;
  QEventLoop*             mEventLoop;
  int                     mNetworkError;
  QNetworkReply*          mNetReply;

public:
  void SetPrimary(const DbS& _Db, const DbStateNotifierS& _Notifier) { mDb = _Db; mNotifier = _Notifier; }

public:
  /*override */virtual const char* Name() { return "Unite agent"; }
  /*override */virtual const char* ShortName() { return "Ua"; }

protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
  /*override */virtual void DoRelease() override;

private:
  bool CheckUpdateTime();
  bool CheckUpdateObjects();
  bool CheckUpdateObjectsOne(int id, int rev);
  bool CheckUpdateEvents();
  bool DoValidateSegment();
  bool DoUpdateObjects();
  bool DoUpdateObjectsOne();
  bool DoUpdateEvents1st();
  bool DoUpdateEventsNext();
  bool LoadObject();
  bool LoadEvents();
  bool LoadLog();
  bool PackValidate();
  bool PackQuery();
  bool PackLogQuery();
  bool PackObjects();
  bool PackEvents();
  bool SendValidate(bool& valid);
  bool SendQuery(bool& needUpdate);
  bool SendLogQuery(bool& needUpdate);
  bool SendObjects();
  bool SendEvents();
  bool UpdateTopLog();

  bool SignRequest();
  bool SendRequest(const QByteArray& function, int& code);

public:
  UniteAgent(const QString& _Uri, int _PeriodMs);
};
