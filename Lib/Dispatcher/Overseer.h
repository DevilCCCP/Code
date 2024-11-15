#pragma once

#include <QSharedPointer>
#include <QSharedMemory>

#include <Lib/Ctrl/CtrlManager.h>
#include "MainInfo.h"


DefineClassS(Overseer);
DefineClassS(Db);
DefineClassS(LogPublisher);

// Модуль отвечает за взаимодействие с диспетчером на стороне рабочего модуля
class Overseer: public CtrlManager
{
  const char*   mDaemonName;
  const int     mId;
  const QString mParams;
  const QString mUri;
  const bool    mDetached;
  const bool    mQuiet;

  QSharedMemory mMainShmem;
  ProcessInfo*  mProcessInfo;
  LogPublisherS mLogPublisher;

public:
  int            Id()       const { return mId; }
  const QString& Params()   const { return mParams; }
  const QString& Uri()      const { return mUri; }
  bool           Detached() const { return mDetached; }
  bool           Quiet()    const { return mQuiet; }

public:
  /*override */virtual void Stop() override;

protected:
  /// return continue or end work
  /*override */virtual bool InitReport() override;
  /*override */virtual bool DoReport() override;
  /*override */virtual void FinalReport() override;
  /*override */virtual void CriticalFail() override;

protected:
  /*override */virtual bool IsPublicStats() override;
  /*override */virtual void PublishStats(const QDateTime& startTime, const QDateTime& endTime, const QList<WorkerStatS>& statList) override;

public:
  void Restart();
  void Done();

private:
  bool OpenShmem();
  bool IsStatusFlag(int flag) { return ((mProcessInfo->CurrentStatus & flag) != 0); }
  void ChangeStatus(EProcStatus newStatus, bool silent = false);

public:
  static int ParseMainWithDb(const char* _DaemonName, int argc, char *argv[], OverseerS& overseer, DbS& db);
  static int ParseMain(const char* _DaemonName, int argc, char *argv[], OverseerS& overseer);
  static int OpenDb(DbS& db);
protected:
  static int ParseCmdLine(int argc, char *argv[], int& id, bool& debug, bool& detached, bool& quiet, QString& params, QString& uri);

public:
  template<typename ImpT>
  static int RunWithOneImp(const char* _DaemonName, int argc, char *argv[])
  {
    OverseerS overseer;
    if (!ParseMain(_DaemonName, argc, argv, overseer)) {
      return -2001;
    }
    QSharedPointer<ImpT> _Imp(new ImpT);
    overseer->RegisterWorker(_Imp);
    return overseer->Run();
  }

  Overseer(const char* _DaemonName, int _Id, bool _Debug, bool _Detached, bool _Quiet, const QString& _Params, const QString& _Uri);
  /*override */virtual ~Overseer();
};

