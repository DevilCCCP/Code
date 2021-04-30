#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include <Lib/Log/Log.h>
#include <Lib/Ctrl/WorkerStat.h>
#include <Lib/Db/Db.h>

#include "Overseer.h"
#include "LogPublisher.h"


const int kStartRetryMs = 2000;

void Overseer::Stop()
{
  Restart();
}

bool Overseer::InitReport()
{
  if (mDetached) {
    return true;
  } else if (OpenShmem() && IsStatusFlag(eFlagStarting | eFlagLive)) {
    ChangeStatus(eInitialize);
    return true;
  } else {
    return false;
  }
}

bool Overseer::DoReport()
{
  if (mDetached) {
    return true;
  } else if (mProcessInfo->SayLive() && IsStatusFlag(eFlagStarting | eFlagLive)) {
    ChangeStatus(eLive);
    return true;
  } else {
    Log.Info("Dispatcher order module to stop");
    return false;
  }
}

void Overseer::FinalReport()
{
  if (!mDetached) {
    if (IsStatusFlag(eFlagLive)) {
      ChangeStatus(eStop);
    }
  }
}

void Overseer::CriticalFail()
{
  Log.Fatal("Critical worker fail");

  if (!mDetached) {
    Restart();
  }
}

bool Overseer::IsPublicStats()
{
  return Id() > 0;
}

void Overseer::PublishStats(const QDateTime& startTime, const QDateTime& endTime, const QList<WorkerStatS>& statList)
{
  if (mLogPublisher) {
    mLogPublisher->PushLog(startTime, endTime, statList);
  }
}

void Overseer::Restart()
{
  if (mDetached) {
    Log.Info("Stopping");
    CtrlManager::Stop();
  } else if (OpenShmem()) {
    Log.Info("Initialized restart");
    ChangeStatus(eRestart);
    CtrlManager::Stop();
  } else {
    throw FatalException();
  }
}

void Overseer::Done()
{
  Log.Info("Initialized exit done");
  if (mDetached) {
    CtrlManager::Stop();
  } else if (OpenShmem()) {
    ChangeStatus(eZombie);
    CtrlManager::Stop();
  } else {
    throw FatalException();
  }
}

bool Overseer::OpenShmem()
{
  if (mProcessInfo) {
    return true;
  }

  QElapsedTimer retry;
  retry.start();
  for (int page = 0; true; page++)
  {
    QString shmemName = MainInfo::GetShmemName(mDaemonName, page);
    mMainShmem.setKey(shmemName);
    if (!mMainShmem.attach(QSharedMemory::ReadWrite)) {
      Log.Fatal(QString("Main shmem not opened (name: '%1', error: '%2')").arg(shmemName).arg(mMainShmem.errorString()));
      return false;
    }
    MainInfo* info = reinterpret_cast<MainInfo*>(mMainShmem.data());
    if (!info->Validate(mMainShmem.size())) {
      Log.Fatal(QString("Main shmem has invalid format (name: '%1')").arg(shmemName));
      return false;
    }
    for (int index = 0; index < info->ProcessCount; index++) {
      ProcessInfo& procInfo = info->Process(index);
      if (procInfo.Id == mId) {
        mProcessInfo = &procInfo;
        return true;
      }
    }
    mMainShmem.detach();

    if (retry.elapsed() > kStartRetryMs) {
      Log.Fatal(QString("Main shmem open timeout (last page %1)").arg(page));
      break;
    }
  }
  return false;
}

void Overseer::ChangeStatus(EProcStatus newStatus, bool silent)
{
  if (mProcessInfo->CurrentStatus == newStatus) {
    return;
  }

  if (!silent) {
    Log.Info(QString("Process status %1 -> %2").arg(EProcStatus_ToString(mProcessInfo->CurrentStatus), EProcStatus_ToString(newStatus)));
  }
  mProcessInfo->CurrentStatus = newStatus;
}

int Overseer::ParseMainWithDb(const char *_DaemonName, int argc, char *argv[], OverseerS &overseer, DbS &db)
{
  if (int ret = OpenDb(db)) {
    return ret;
  }

  return ParseMain(_DaemonName, argc, argv, overseer);
}

const int kMandatory = 1;
void ParseGetInt(const QString& paramText, int flag, int& id, int& result)
{
  bool get;
  id = paramText.toInt(&get);
  if (get) {
    result |= flag;
  }
}

int Overseer::ParseMain(const char *_DaemonName, int argc, char *argv[], OverseerS &overseer)
{
  int     id;
  bool    debug;
  bool    detached;
  bool    quiet;
  QString params;
  QString uri;

  if (int ret = ParseCmdLine(argc, argv, id, debug, detached, quiet, params, uri)) {
    return ret;
  }

  overseer = OverseerS(new Overseer(_DaemonName, id, debug, detached, quiet, params, uri));
  return 0;
}

int Overseer::OpenDb(DbS& db)
{
  db = DbS(new Db());
  if (!db->OpenDefault()) {
    return -2002;
  }
  if (!db->Connect()) {
    Log.Warning(QString("Connect to db fail"));
    QThread::msleep(100);
    for (int i = 0; i < 30; i++) {
      if (!db->Connect() && i >= 29) {
        Log.Fatal(QString("Connect to db fail all tries"));
        QThread::msleep(5000);
        return -2003;
      }
      QThread::msleep(100);
    }
  }
  return 0;
}

int Overseer::ParseCmdLine(int argc, char* argv[], int& id, bool& debug, bool& detached, bool& quiet, QString& params, QString& uri)
{
  debug = false;
  detached = false;
  quiet = false;

  int result = 0;
  for (int i = 0; i < argc; i++)
  {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'd':
        debug = true;
        break;

      case 't':
        detached = true;
        break;

      case 'q':
        quiet = true;
        break;

      case 'i':
        ParseGetInt(QString::fromLocal8Bit(argv[i]+2), 1, id, result);
        break;

      case 'p':
        params = QString::fromLocal8Bit(argv[i]+2);
        break;

      case 'u':
        uri = QString::fromLocal8Bit(argv[i]+2);
        break;

      case '-':
        QString line = QString::fromLocal8Bit(argv[i]+2);
        {
          int eq = line.indexOf(QChar('='));
          if (eq > 0) {
            QString key = line.left(eq).trimmed();
            QString value = line.mid(eq + 1).trimmed();
            if (key == "id") {
              ParseGetInt(value, 1, id, result);
            } else if (key == "params") {
              params = value;
            } else if (key == "uri") {
              uri = value;
            }
          } else {
            if (line == "debug") {
              debug = true;
            } else if (line == "detached") {
              detached = true;
            } else if (line == "quiet") {
              quiet = true;
            }
          }
        }
        break;
      }
    }
  }

  if (!debug) {
    Log.SetFileLogging(QString("%1_").arg(id, 6, 10, QChar('0')));
  } else {
    Log.SetConsoleLogging();
  }

  if ((result & kMandatory) != kMandatory) {
    return -2001;
  }
  return 0;
}

Overseer::Overseer(const char* _DaemonName, int _Id, bool _Debug, bool _Detached, bool _Quiet, const QString& _Params, const QString& _Uri)
  : CtrlManager(_Debug)
  , mDaemonName(_DaemonName), mId(_Id), mDebug(_Debug), mParams(_Params), mUri(_Uri), mDetached(_Detached), mQuiet(_Quiet)
  , mProcessInfo(nullptr)
{
  if (!mQuiet) {
    DbS db;
    db.reset(new Db());
    if (db->OpenDefault()) {
      mLogPublisher.reset(new LogPublisher(db));
      RegisterWorker(mLogPublisher);
      SetLogWorker(mLogPublisher.data());
    } else {
      LOG_ERROR_ONCE("Open Db for log fail");
    }
  }
}

Overseer::~Overseer()
{
  if (!mDetached) {
    mMainShmem.detach();
  }
}
